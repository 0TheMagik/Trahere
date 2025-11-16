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
    }

    const qreal dpr = (m_canvas && m_canvas->window()) ? m_canvas->window()->effectiveDevicePixelRatio() : 1.0;

    // Paint a filled circle into the CPU buffer at pixel coordinates, skipping pixels already equal to color
    auto paintCirclePix = [&](float cxPix, float cyPix, const QColor &color, float radiusPix){
        float r = std::max(0.5f, radiusPix);
        int rPix = static_cast<int>(std::ceil(r));
        int x0 = std::max(0, static_cast<int>(std::floor(cxPix - rPix)));
        int x1 = std::min(m_buffer.width() - 1, static_cast<int>(std::ceil(cxPix + rPix)));
        int y0 = std::max(0, static_cast<int>(std::floor(cyPix - rPix)));
        int y1 = std::min(m_buffer.height() - 1, static_cast<int>(std::ceil(cyPix + rPix)));
        int rSq = static_cast<int>(r * r);
        for (int y = y0; y <= y1; ++y) {
            int dy = y - static_cast<int>(cyPix);
            int dySq = dy * dy;
            for (int x = x0; x <= x1; ++x) {
                int dx = x - static_cast<int>(cxPix);
                if (dx*dx + dySq <= rSq) {
                    QRgb *scan = reinterpret_cast<QRgb*>(m_buffer.scanLine(y));
                    QRgb existing = scan[x];
                    if (existing != color.rgba()) {
                        scan[x] = color.rgba();
                    }
                }
            }
        }
    };

    // Helper to draw a stroke by interpolating between points in pixel space
    auto drawStrokeInterpolated = [&](const QList<QVector2D>& ptsLogical, const QColor& col, float sizeLogical){
        if (ptsLogical.size() == 0) return;
        float radiusPix = std::max(0.5f, sizeLogical * 0.5f * (float)dpr);
        // Always stamp first point
        {
            QVector2D p0 = ptsLogical.first() * (float)dpr;
            paintCirclePix(p0.x(), p0.y(), col, radiusPix);
        }
        for (int i = 1; i < ptsLogical.size(); ++i) {
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
    int strokeCount = m_canvas->m_brush.strokeCount();
    if (m_rebuildVersion != strokeCount) {
        m_buffer.fill(Qt::white);
        for (const auto &stroke : m_canvas->m_brush.strokes()) {
            drawStrokeInterpolated(stroke.points, stroke.color, stroke.size);
        }
        m_rebuildVersion = strokeCount;
    }
    // Add in-progress stroke on top (not yet committed)
    if (m_canvas->m_brush.isDrawing()) {
        drawStrokeInterpolated(m_canvas->m_brush.currentPoints(), m_canvas->m_brush.currentColor(), m_canvas->m_brush.currentSize());
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
    } else {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        if (m_textureSize != m_buffer.size()) {
            // Reallocate texture on size change (e.g., fullscreen)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_buffer.width(), m_buffer.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer.constBits());
            m_textureSize = m_buffer.size();
        } else {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_buffer.width(), m_buffer.height(), GL_RGBA, GL_UNSIGNED_BYTE, m_buffer.constBits());
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
    QVector2D cursorLogical = m_canvas->cursorPos();
    if (m_canvas->m_brush.isDrawing() && cursorLogical.x() >= 0 && cursorLogical.y() >= 0) {
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
        QColor outline = m_canvas->brushColor();
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
