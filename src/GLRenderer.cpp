#include "GLRenderer.h"
#include "Canvas.h"
#include "Layer.h" // ensure complete type for method calls
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickWindow>
#include <QPainter>
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

    // Snapshot per-layer content in stacking order (bottom -> top)
    m_layersSnap.clear();
    m_activeLayerIndexSnap = -1;
    const QList<Layer*> &raw = canvas->rawLayers();
    for (int li = 0; li < raw.size(); ++li) {
        Layer* layer = raw.at(li);
        if (!layer) continue;
        LayerSnap snap;
        snap.visible = layer->isVisible();
        if (snap.visible && layer->hasRaster()) snap.raster = layer->raster();
        snap.strokes = layer->engine().strokes();
        m_layersSnap.append(std::move(snap));
        if (layer == canvas->activeLayer()) m_activeLayerIndexSnap = li;
    }

    // Snapshot active layer in-progress stroke if any
    Layer* active = canvas->activeLayer();
    if (active) {
        m_isDrawingSnap = active->engine().isDrawing();
        m_currentPointsSnap = active->engine().currentPoints();
        m_currentColorSnap = active->engine().currentColor();
        m_currentSizeSnap = active->engine().currentSize();
        m_currentModeSnap = active->engine().currentMode();
    } else {
        m_isDrawingSnap = false;
        m_currentPointsSnap.clear();
        m_currentModeSnap = BrushStroke::Draw;
    }

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

    // --- Layer versioning & caching ---
    // Compute version signatures for each layer to know if cached image needs rebuild.
    QVector<qint64> currentLayerVersions; currentLayerVersions.reserve(m_layersSnap.size());
    const qint64 P1 = 1000003LL, P2 = 2000003LL, P3 = 3000017LL, P4 = 7000003LL;
    for (int idx=0; idx<m_layersSnap.size(); ++idx) {
        const auto &ls = m_layersSnap[idx];
        qint64 strokeCount = ls.strokes.size();
        qint64 eraseCount = 0;
        qint64 pointsAccum = 0;
        for (const auto &s : ls.strokes) {
            if (s.mode == BrushStroke::Erase) ++eraseCount;
            pointsAccum += s.points.size();
        }
        qint64 rasterFlag = (!ls.raster.isNull() && ls.visible) ? 1 : 0;
        // Include raster revision via attached property stored in alpha channel of first pixel if needed; fallback 0.
        // For robustness, derive a lightweight hash of raster top-left pixel + size even if stroke counts unchanged.
        qint64 rasterHash = 0;
        if (rasterFlag) {
            rasterHash = ls.raster.width()*1469598103934665603ULL + ls.raster.height()*1099511628211ULL;
            if (!ls.raster.isNull()) {
                QRgb px = ls.raster.pixel(0,0);
                rasterHash ^= (qRed(px) + 1); rasterHash *= 1099511628211ULL;
                rasterHash ^= (qGreen(px) + 1); rasterHash *= 1099511628211ULL;
                rasterHash ^= (qBlue(px) + 1); rasterHash *= 1099511628211ULL;
                rasterHash ^= (qAlpha(px) + 1); rasterHash *= 1099511628211ULL;
            }
        }
        qint64 version = strokeCount * P1 + eraseCount * P2 + pointsAccum * P3 + rasterFlag * P4 + rasterHash;
        currentLayerVersions.push_back(version);
    }

    // Initialize caches if first time or layer count changed
    if (m_cachedLayerImages.size() != m_layersSnap.size()) {
        m_cachedLayerImages.resize(m_layersSnap.size());
        m_cachedLayerVersions.resize(m_layersSnap.size());
        for (int i=0;i<m_cachedLayerVersions.size();++i) m_cachedLayerVersions[i] = -1; // force rebuild
    }

    // Function to stamp stroke into a layer image (supports erase)
    auto stampStroke = [&](QImage &layerImg, const QList<QVector2D>& pts, const QColor &col, float size, BrushStroke::StrokeMode mode){
            if (pts.isEmpty()) return;
            // Reuse previous interpolated logic: treat layerImg as buffer target
            QImage *targetPtr = &layerImg;
            auto paintCirclePixLayer = [&](float cxPix, float cyPix, const QColor &color, float radiusPix){
                const float r = std::max(0.5f, radiusPix);
                const int rPix = static_cast<int>(std::ceil(r));
                const int w = targetPtr->width();
                const int h = targetPtr->height();
                const int x0 = std::max(0, static_cast<int>(std::floor(cxPix - rPix)));
                const int x1 = std::min(w - 1, static_cast<int>(std::ceil(cxPix + rPix)));
                const int y0 = std::max(0, static_cast<int>(static_cast<int>(std::floor(cyPix - rPix))));
                const int y1 = std::min(h - 1, static_cast<int>(std::ceil(cyPix + rPix)));
                const float rSq = r * r;
                constexpr float root2over2 = 0.70710678f;
                const float inner = std::max(0.0f, r - root2over2);
                const float innerSq = inner * inner;
                const float outer = r + root2over2;
                const float outerSq = outer * outer;
                const float sr = color.redF();
                const float sg = color.greenF();
                const float sb = color.blueF();
                const float saBrush = color.alphaF();
                static const float offs[4] = { -0.375f, -0.125f, 0.125f, 0.375f };
                for (int y = y0; y <= y1; ++y) {
                    QRgb *scan = reinterpret_cast<QRgb*>(targetPtr->scanLine(y));
                    const float pyCenter = (float)y + 0.5f;
                    const float dyc = pyCenter - cyPix;
                    const float dycSq = dyc * dyc;
                    for (int x = x0; x <= x1; ++x) {
                        const float pxCenter = (float)x + 0.5f;
                        const float dxc = pxCenter - cxPix;
                        const float centerDistSq = dxc * dxc + dycSq;
                        float coverage = 0.0f;
                        if (centerDistSq <= innerSq) coverage = 1.0f; else if (centerDistSq >= outerSq) continue; else {
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
                        if (mode == BrushStroke::Erase) {
                            // Clear pixel: set transparent
                            scan[x] = qRgba(0,0,0,0);
                            continue;
                        }
                        // Draw mode compositing over existing pixel
                        const float srcA = std::clamp(coverage * saBrush, 0.0f, 1.0f);
                        if (srcA <= 0.0f) continue;
                        const QRgb dst = scan[x];
                        float dr = qRed(dst) / 255.0f;
                        float dg = qGreen(dst) / 255.0f;
                        float db = qBlue(dst) / 255.0f;
                        float da = qAlpha(dst) / 255.0f;
                        float outA = srcA + da * (1.0f - srcA);
                        float outR = (sr * srcA + dr * da * (1.0f - srcA));
                        float outG = (sg * srcA + dg * da * (1.0f - srcA));
                        float outB = (sb * srcA + db * da * (1.0f - srcA));
                        if (outA > 0.0001f) {
                            outR /= outA; outG /= outA; outB /= outA;
                        }
                        int ir = (int)std::lround(std::clamp(outR * 255.0f, 0.0f, 255.0f));
                        int ig = (int)std::lround(std::clamp(outG * 255.0f, 0.0f, 255.0f));
                        int ib = (int)std::lround(std::clamp(outB * 255.0f, 0.0f, 255.0f));
                        int ia = (int)std::lround(std::clamp(outA * 255.0f, 0.0f, 255.0f));
                        scan[x] = qRgba(ir, ig, ib, ia);
                    }
                }
            };
            const int n = pts.size();
            if (n == 0) return;
            float radiusPix = std::max(0.5f, size * 0.5f * (float)dpr);
            QVector2D p0 = pts.first() * (float)dpr;
            paintCirclePixLayer(p0.x(), p0.y(), col, radiusPix);
            for (int i=1;i<n;++i){
                QVector2D a = pts[i-1] * (float)dpr;
                QVector2D b = pts[i] * (float)dpr;
                QVector2D d = b - a;
                float len = std::sqrt(d.lengthSquared());
                if (len < 1e-3f) { paintCirclePixLayer(b.x(), b.y(), col, radiusPix); continue; }
                QVector2D dir = d / len;
                float step = std::max(1.0f, radiusPix * 0.5f);
                float t = 0.0f;
                while (t <= len) { QVector2D p = a + dir * t; paintCirclePixLayer(p.x(), p.y(), col, radiusPix); t += step; }
                paintCirclePixLayer(b.x(), b.y(), col, radiusPix);
            }
        };

    // Rebuild any invalidated cached layer images (excluding in-progress stroke)
    for (int li=0; li<m_layersSnap.size(); ++li) {
        const auto &ls = m_layersSnap[li];
        if (!ls.visible) {
            m_cachedLayerImages[li] = QImage();
            m_cachedLayerVersions[li] = currentLayerVersions[li];
            continue;
        }
        if (m_cachedLayerVersions[li] != currentLayerVersions[li]) {
            QImage layerImg(m_viewportSize, QImage::Format_RGBA8888);
            layerImg.fill(Qt::transparent);
            // Raster base
            if (!ls.raster.isNull()) {
                QImage r = ls.raster;
                if (r.size() != m_viewportSize) r = r.scaled(m_viewportSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                QPainter rp(&layerImg); rp.drawImage(0,0,r); rp.end();
            }
            // Committed strokes
            for (const auto &stroke : ls.strokes) {
                stampStroke(layerImg, stroke.points, stroke.color, stroke.size, stroke.mode);
            }
            m_cachedLayerImages[li] = layerImg;
            m_cachedLayerVersions[li] = currentLayerVersions[li];
        }
    }

    // Incremental compositing path
    qint64 layersCombinedVersion = 0;
    for (qint64 v : currentLayerVersions) layersCombinedVersion ^= (v + 0x9e3779b97f4a7c15ULL);
    qint64 contentVersion = layersCombinedVersion; // exclude in-progress stroke changes
    bool layersChanged = (m_rebuildVersion != contentVersion);

    if (layersChanged) {
        // Rebuild base composite excluding active layer
        if (m_canvas && m_canvas->hasBaseImage()) {
            QImage base = m_canvas->baseImage();
            if (base.size() != m_viewportSize) base = base.scaled(m_viewportSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            if (base.format() != QImage::Format_RGBA8888) base = base.convertToFormat(QImage::Format_RGBA8888);
            m_baseCompositeExcludingActive = base;
        } else {
            m_baseCompositeExcludingActive = QImage(m_viewportSize, QImage::Format_RGBA8888);
            m_baseCompositeExcludingActive.fill(Qt::white);
        }
        QPainter basep(&m_baseCompositeExcludingActive);
        for (int li=0; li<m_cachedLayerImages.size(); ++li) {
            if (li == m_activeLayerIndexSnap) continue; // exclude active layer content
            const QImage &img = m_cachedLayerImages[li];
            if (!img.isNull()) basep.drawImage(0,0,img);
        }
        basep.end();
        m_rebuildVersion = contentVersion;
        // Base changed invalidates incremental composite
        m_incrementalCompositeValid = false;
    }

    // Handle in-progress stroke incrementally
    if (m_isDrawingSnap) {
        if (!m_prevWasDrawing) {
            // New stroke start: clone active cached image or initialize transparent
            if (m_activeLayerIndexSnap >=0 && m_activeLayerIndexSnap < m_cachedLayerImages.size()) {
                m_activeInProgressImage = m_cachedLayerImages[m_activeLayerIndexSnap];
            }
            if (m_activeInProgressImage.isNull()) {
                m_activeInProgressImage = QImage(m_viewportSize, QImage::Format_RGBA8888);
                m_activeInProgressImage.fill(Qt::transparent);
            }
            m_lastInProgressStampedCount = 0;
            // Initialize final buffer composite now
            if (m_buffer.size() != m_viewportSize) m_buffer = QImage(m_viewportSize, QImage::Format_RGBA8888);
            m_buffer.fill(Qt::transparent);
            QPainter initp(&m_buffer);
            if (!m_baseCompositeExcludingActive.isNull()) initp.drawImage(0,0,m_baseCompositeExcludingActive);
            if (!m_activeInProgressImage.isNull()) initp.drawImage(0,0,m_activeInProgressImage);
            initp.end();
            m_incrementalCompositeValid = true;
            m_bufferDirty = true;
        }
        int currentCount = m_currentPointsSnap.size();
        if (currentCount > 0) {
            QList<QVector2D> segment;
            if (m_lastInProgressStampedCount == 0) {
                segment = m_currentPointsSnap; // first frame stamp all
            } else if (currentCount > m_lastInProgressStampedCount) {
                int start = m_lastInProgressStampedCount - 1;
                if (start < 0) start = 0;
                for (int i=start; i<currentCount; ++i) segment.append(m_currentPointsSnap[i]);
            }
            if (!segment.isEmpty()) {
                // Stamp segment onto active layer clone
                stampStroke(m_activeInProgressImage, segment, m_currentColorSnap, m_currentSizeSnap, m_currentModeSnap);
                // If incremental composite valid, also stamp directly into final buffer
                if (m_incrementalCompositeValid) {
                    stampStroke(m_buffer, segment, m_currentColorSnap, m_currentSizeSnap, m_currentModeSnap);
                    m_bufferDirty = true;
                } else {
                    // Fallback: rebuild final composite this frame
                    if (m_buffer.size() != m_viewportSize) m_buffer = QImage(m_viewportSize, QImage::Format_RGBA8888);
                    m_buffer.fill(Qt::transparent);
                    QPainter finalRe(&m_buffer);
                    if (!m_baseCompositeExcludingActive.isNull()) finalRe.drawImage(0,0,m_baseCompositeExcludingActive);
                    finalRe.drawImage(0,0,m_activeInProgressImage);
                    finalRe.end();
                    m_bufferDirty = true;
                    m_incrementalCompositeValid = true;
                }
                m_lastInProgressStampedCount = currentCount;
            }
        }
    }

    // Compose final buffer only if not drawing (or layers changed without incremental validity)
    if (!m_isDrawingSnap) {
        if (m_buffer.size() != m_viewportSize) m_buffer = QImage(m_viewportSize, QImage::Format_RGBA8888);
        m_buffer.fill(Qt::transparent);
        QPainter finalComp(&m_buffer);
        if (!m_baseCompositeExcludingActive.isNull()) finalComp.drawImage(0,0,m_baseCompositeExcludingActive);
        if (m_activeLayerIndexSnap >=0 && m_activeLayerIndexSnap < m_cachedLayerImages.size()) {
            const QImage &activeImg = m_cachedLayerImages[m_activeLayerIndexSnap];
            if (!activeImg.isNull()) finalComp.drawImage(0,0,activeImg);
        }
        finalComp.end();
        m_bufferDirty = true;
        m_activeInProgressImage = QImage();
        m_lastInProgressStampedCount = 0;
        m_incrementalCompositeValid = false;
    }
    m_prevWasDrawing = m_isDrawingSnap;

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
