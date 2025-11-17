#include "GLRenderer.h"
#include "Canvas.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickWindow>
#include <cmath>
#include <algorithm>

GLRenderer::GLRenderer(Canvas *canvas)
    : m_canvas(canvas)
{
}

QOpenGLFramebufferObject *GLRenderer::createFramebufferObject(const QSize &size) {
    m_viewportSize = size;
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

void GLRenderer::synchronize(QQuickFramebufferObject *item) {
    // Called on render thread while GUI thread is blocked; safe to read item state
    auto *canvas = static_cast<Canvas*>(item);
    // Snapshot brush engine state
    m_isDrawingSnap = canvas->m_brush.isDrawing();
    m_currentPointsSnap = canvas->m_brush.currentPoints();
    m_currentColorSnap = canvas->m_brush.currentColor();
    m_currentSizeSnap = canvas->m_brush.currentSize();
    m_strokesSnap = canvas->m_brush.strokes();

    // Snapshot UI-related values
    m_cursorPosSnap = canvas->cursorPos();
    m_brushColorSnap = canvas->brushColor();
    m_brushSizeSnap = canvas->brushSize();
    m_dpr = (canvas->window() ? canvas->window()->effectiveDevicePixelRatio() : 1.0);
}

void GLRenderer::render() {
    // Ensure we have a current context and initialize once per renderer
    if (!m_initialized) {
        initializeOpenGLFunctions();

        // Shader for textured quad
        m_program.addShaderFromSourceCode(QOpenGLShader::Vertex,
            R"(
            attribute vec2 a_pos;
            attribute vec2 a_uv;
            varying vec2 v_uv;
            void main(){
                v_uv = a_uv;
                gl_Position = vec4(a_pos, 0.0, 1.0);
            })");
        m_program.addShaderFromSourceCode(QOpenGLShader::Fragment,
            R"(
            varying vec2 v_uv;
            uniform sampler2D u_tex;
            void main(){
                gl_FragColor = texture2D(u_tex, v_uv);
            })");
        m_program.bindAttributeLocation("a_pos", 0);
        m_program.bindAttributeLocation("a_uv", 1);
        m_program.link();

        // Overlay shader for preview outline (solid color)
        m_overlayProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
            R"(
            attribute vec2 a_pos;
            void main(){
                gl_Position = vec4(a_pos, 0.0, 1.0);
            })");
        m_overlayProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
            R"(
            uniform vec4 u_color;
            void main(){
                gl_FragColor = u_color;
            })");
        m_overlayProgram.bindAttributeLocation("a_pos", 0);
        m_overlayProgram.link();
        m_initialized = true;
    }

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Prepare CPU buffer if needed (pixel size)
    if (m_buffer.size() != m_viewportSize) {
        m_buffer = QImage(m_viewportSize, QImage::Format_RGBA8888);
        m_buffer.fill(Qt::white);
        m_rebuildVersion = -1; // force rebuild
        m_bufferDirty = true;
    }

    const qreal dpr = m_dpr;

    // Paint a filled circle into the CPU buffer at pixel coordinates, skipping pixels already equal to color
    auto paintCirclePix = [&](float cxPix, float cyPix, const QColor &color, float radiusPix){
        // Antialiased stamp: per-pixel coverage with fast accept/reject, supersample on edges.
        const float r = std::max(0.5f, radiusPix);
        const int rPix = static_cast<int>(std::ceil(r));
        const int w = m_buffer.width();
        const int h = m_buffer.height();
        const int x0 = std::max(0, static_cast<int>(std::floor(cxPix - rPix)));
        const int x1 = std::min(w - 1, static_cast<int>(std::ceil(cxPix + rPix)));
        const int y0 = std::max(0, static_cast<int>(std::floor(cyPix - rPix)));
        const int y1 = std::min(h - 1, static_cast<int>(std::ceil(cyPix + rPix)));

        const float rSq = r * r;
        constexpr float root2over2 = 0.70710678f;
        const float inner = std::max(0.0f, r - root2over2);
        const float innerSq = inner * inner;
        const float outer = r + root2over2;
        const float outerSq = outer * outer;

        // Brush color as floats
        const float sr = color.redF();
        const float sg = color.greenF();
        const float sb = color.blueF();
        const float saBrush = color.alphaF();

        // Supersample grid (4x4) offsets relative to pixel center
        static const float offs[4] = { -0.375f, -0.125f, 0.125f, 0.375f };

        for (int y = y0; y <= y1; ++y) {
            QRgb *scan = reinterpret_cast<QRgb*>(m_buffer.scanLine(y));
            const float pyCenter = (float)y + 0.5f;
            const float dyc = pyCenter - cyPix;
            const float dycSq = dyc * dyc;
            for (int x = x0; x <= x1; ++x) {
                const float pxCenter = (float)x + 0.5f;
                const float dxc = pxCenter - cxPix;
                const float centerDistSq = dxc * dxc + dycSq;

                float coverage = 0.0f;
                if (centerDistSq <= innerSq) {
                    coverage = 1.0f; // fully inside
                } else if (centerDistSq >= outerSq) {
                    continue; // fully outside
                } else {
                    // Edge pixel: 4x4 supersampling
                    int inside = 0;
                    for (int jy = 0; jy < 4; ++jy) {
                        const float oy = offs[jy];
                        const float sy = pyCenter + oy;
                        const float dy = sy - cyPix;
                        for (int ix = 0; ix < 4; ++ix) {
                            const float ox = offs[ix];
                            const float sx = pxCenter + ox;
                            const float dx = sx - cxPix;
                            const float dsq = dx * dx + dy * dy;
                            if (dsq <= rSq) ++inside;
                        }
                    }
                    coverage = inside / 16.0f;
                    if (coverage <= 0.0f) continue;
                }

                // Blend new color over existing pixel using straight alpha compositing.
                const float srcA = std::clamp(coverage * saBrush, 0.0f, 1.0f);
                if (srcA <= 0.0f) continue;

                const QRgb dst = scan[x];
                const float dr = qRed(dst) / 255.0f;
                const float dg = qGreen(dst) / 255.0f;
                const float db = qBlue(dst) / 255.0f;
                // Destination is opaque (background is opaque white), keep alpha at 1.0
                const float outR = sr * srcA + dr * (1.0f - srcA);
                const float outG = sg * srcA + dg * (1.0f - srcA);
                const float outB = sb * srcA + db * (1.0f - srcA);

                const int ir = (int)std::lround(std::clamp(outR * 255.0f, 0.0f, 255.0f));
                const int ig = (int)std::lround(std::clamp(outG * 255.0f, 0.0f, 255.0f));
                const int ib = (int)std::lround(std::clamp(outB * 255.0f, 0.0f, 255.0f));
                const QRgb out = qRgba(ir, ig, ib, 255);
                if (out != dst) scan[x] = out;
            }
        }
        m_bufferDirty = true;
    };

    // Helper to draw a stroke by interpolating between points in pixel space
    auto drawStrokeInterpolated = [&](const QList<QVector2D>& ptsLogical, const QColor& col, float sizeLogical){
        const int n = ptsLogical.size();
        if (n == 0) return;
        float radiusPix = std::max(0.5f, sizeLogical * 0.5f * (float)dpr);
        // Always stamp first point
        {
            QVector2D p0 = ptsLogical.first() * (float)dpr;
            paintCirclePix(p0.x(), p0.y(), col, radiusPix);
        }
        for (int i = 1; i < n; ++i) {
            QVector2D a = ptsLogical[i-1] * (float)dpr;
            QVector2D b = ptsLogical[i] * (float)dpr;
            QVector2D d = b - a;
            float len = std::sqrt(d.lengthSquared());
            if (len < 1e-3f) {
                paintCirclePix(b.x(), b.y(), col, radiusPix);
                continue;
            }
            QVector2D dir = d / len;
            float step = std::max(1.0f, radiusPix * 0.5f); // dense enough to avoid gaps
            float t = 0.0f;
            while (t <= len) {
                QVector2D p = a + dir * t;
                paintCirclePix(p.x(), p.y(), col, radiusPix);
                t += step;
            }
            // Ensure we hit the exact end point
            paintCirclePix(b.x(), b.y(), col, radiusPix);
        }
    };

    // Rebuild buffer from all committed strokes only if stroke count changed or forced
    int strokeCount = m_strokesSnap.size();
    if (m_rebuildVersion != strokeCount) {
        m_buffer.fill(Qt::white);
        for (const auto &stroke : m_strokesSnap) {
            drawStrokeInterpolated(stroke.points, stroke.color, stroke.size);
        }
        m_rebuildVersion = strokeCount;
        m_bufferDirty = true;
    }
    // Add in-progress stroke on top (not yet committed)
    if (m_isDrawingSnap) {
        drawStrokeInterpolated(m_currentPointsSnap, m_currentColorSnap, m_currentSizeSnap);
    }

    // Upload to texture
    if (m_texture == 0) {
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_buffer.width(), m_buffer.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer.constBits());
        m_textureSize = m_buffer.size();
        m_bufferDirty = false;
    } else {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        if (m_textureSize != m_buffer.size()) {
            // Reallocate texture on size change (e.g., fullscreen)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_buffer.width(), m_buffer.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer.constBits());
            m_textureSize = m_buffer.size();
            m_bufferDirty = false;
        } else if (m_bufferDirty) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_buffer.width(), m_buffer.height(), GL_RGBA, GL_UNSIGNED_BYTE, m_buffer.constBits());
            m_bufferDirty = false;
        }
    }

    // Draw textured quad covering viewport
    m_program.bind();
    glBindTexture(GL_TEXTURE_2D, m_texture);
    // Quad with UVs matching QImage's top-left origin (no vertical flip)
    GLfloat verts[] = {
        //  x     y      u    v
        -1.f, -1.f,   0.f, 0.f, // bottom-left maps to (0,0)
         1.f, -1.f,   1.f, 0.f, // bottom-right maps to (1,0)
        -1.f,  1.f,   0.f, 1.f, // top-left maps to (0,1)
         1.f,  1.f,   1.f, 1.f  // top-right maps to (1,1)
    };
    // position (vec2) + uv (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), verts);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), verts + 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Brush preview circle: outline-only (1px), center transparent
    // Show only while drawing (mouse pressed)
    QVector2D cursorLogical = m_cursorPosSnap;
    if (m_isDrawingSnap && cursorLogical.x() >= 0 && cursorLogical.y() >= 0) {
        // Convert to pixel space for mapping, then to NDC; flip Y for GL
        QVector2D cursorPix = cursorLogical * (float)dpr;
        float radiusPix = std::max(0.5f, m_canvas->brushSize() * 0.5f * (float)dpr);
        const int SEG = 64;
        QVector<GLfloat> ringNdc;
        ringNdc.reserve(SEG * 2);
        for (int i=0; i<SEG; ++i) {
            float ang = (float)i / SEG * 2.0f * (float)M_PI;
            float px = cursorPix.x() + std::cos(ang) * radiusPix;
            float py = cursorPix.y() + std::sin(ang) * radiusPix;
            float x = (px / m_viewportSize.width())*2.f - 1.f;
            float y = (py / m_viewportSize.height())*2.f - 1.f; 
            ringNdc.push_back(x);
            ringNdc.push_back(y);
        }
        // Setup overlay shader and draw
        m_overlayProgram.bind();
        QColor outline = m_brushColorSnap;
        outline.setAlphaF(1.0f); // solid outline; center remains transparent
        m_overlayProgram.setUniformValue("u_color", outline);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(1.f);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, ringNdc.constData());
        glDrawArrays(GL_LINE_LOOP, 0, SEG);
        glDisableVertexAttribArray(0);
        glDisable(GL_BLEND);
        m_overlayProgram.release();
    }

    m_program.release();
    update(); // continuous repaint while drawing
}
