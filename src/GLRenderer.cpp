#include "GLRenderer.h"
#include "Canvas.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickWindow>
#include <cmath>

GLRenderer::GLRenderer(Canvas *canvas)
    : m_canvas(canvas)
{
    // Simple pass-through shader (OpenGL ES 2.0 style)
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex,
        R"(
        attribute vec2 a_position;
        uniform vec2 u_resolution;
        void main() {
            vec2 zeroToOne = a_position / u_resolution;      // logical -> 0..1
            vec2 zeroToTwo = zeroToOne * 2.0;                // 0..2
            vec2 clipSpace = zeroToTwo - 1.0;                // -1..+1
            gl_Position = vec4(clipSpace, 0.0, 1.0);         // no extra Y flip (QQuickFramebufferObject handles orientation)
        })");

    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment,
        R"(
        uniform vec4 u_color;
        void main() {
            gl_FragColor = u_color;
        })");

    m_program.bindAttributeLocation("a_position", 0);
    m_program.link();
}

QOpenGLFramebufferObject *GLRenderer::createFramebufferObject(const QSize &size) {
    m_viewportSize = size;
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

void GLRenderer::render() {
    // Initialize GL functions only when context current
    static bool glReady = false;
    if (!glReady) { initializeOpenGLFunctions(); glReady = true; }

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program.bind();
    m_program.setUniformValue("u_resolution", QVector2D(m_viewportSize.width(), m_viewportSize.height()));

    const qreal dpr = (m_canvas && m_canvas->window()) ? m_canvas->window()->effectiveDevicePixelRatio() : 1.0;
    const int posLoc = m_program.attributeLocation("a_position");

    // Draw filled circle (dot) at a position using a triangle fan approximation.
    auto drawCircle = [&](const QVector2D& centerLogical, const QColor& col, float size){
        float radius = qMax(0.5f, size * 0.5f) * dpr;
        const int SEGMENTS = 24; // smooth enough
        QVector2D center = centerLogical * dpr;
        m_program.setUniformValue("u_color", col);
        QVector<QVector2D> verts;
        verts.reserve(SEGMENTS + 2);
        verts.push_back(center); // fan center
        for (int i=0; i<=SEGMENTS; ++i) {
            float ang = (static_cast<float>(i) / SEGMENTS) * 2.0f * static_cast<float>(M_PI);
            verts.push_back(center + QVector2D(std::cos(ang) * radius, std::sin(ang) * radius));
        }
        m_program.enableAttributeArray(posLoc);
        m_program.setAttributeArray(posLoc, verts.constData());
        glDrawArrays(GL_TRIANGLE_FAN, 0, verts.size());
        m_program.disableAttributeArray(posLoc);
    };

    // Draw stroke by stamping circles along the path with occupancy grid to avoid overdraw.
    auto drawStrokeStamped = [&](const QList<QVector2D>& ptsIn, const QColor& col, float size){
        if (ptsIn.isEmpty()) return;
        const float radius = qMax(0.5f, size * 0.5f);
        const float step = qMax(1.0f, radius * 0.5f); // spacing in logical px
        const float tile = qMax(1.0f, radius * 0.6f); // occupancy tile size

        // Build a simplified/resampled path at near-constant spacing
        QVector<QVector2D> pts;
        pts.reserve(ptsIn.size());
        // Start with first point
        QVector2D acc = ptsIn.first();
        pts.push_back(acc);
        float carry = 0.0f;
        for (int i = 1; i < ptsIn.size(); ++i) {
            QVector2D a = acc;
            QVector2D b = ptsIn[i];
            QVector2D d = b - a;
            float len = std::sqrt(d.lengthSquared());
            if (len < 1e-4f) continue;
            QVector2D dir = d / len;
            float dist = carry + len;
            float t = 0.0f;
            // Place points every 'step'
            while (dist >= step) {
                float adv = step - carry;
                t += adv / len;
                QVector2D p = a + dir * (t * len);
                pts.push_back(p);
                dist -= step;
                carry = 0.0f;
            }
            carry = dist;
            acc = b;
        }

        // Ensure last point included
        if ((pts.isEmpty()) || (pts.last() - ptsIn.last()).lengthSquared() > 1e-6f)
            pts.push_back(ptsIn.last());

        // Occupancy grid (logical coords)
        QSet<quint64> occupied;
        auto keyOf = [&](const QVector2D &p)->quint64{
            int gx = static_cast<int>(std::floor(p.x() / tile));
            int gy = static_cast<int>(std::floor(p.y() / tile));
            return (static_cast<quint64>(static_cast<quint32>(gx)) << 32) | static_cast<quint32>(gy);
        };

        // Stamp circles, skip if tile already filled
        for (const auto &p : pts) {
            quint64 k = keyOf(p);
            if (occupied.contains(k)) continue;
            drawCircle(p, col, size);
            occupied.insert(k);
        }
    };

    // Committed strokes
    for (const auto &stroke : m_canvas->m_brush.strokes()) {
        drawStrokeStamped(stroke.points, stroke.color, stroke.size);
    }

    // In-progress stroke
    if (m_canvas->m_brush.isDrawing()) {
        drawStrokeStamped(m_canvas->m_brush.currentPoints(), m_canvas->m_brush.currentColor(), m_canvas->m_brush.currentSize());
    }

    m_program.release();
    update(); // continuous repaint while drawing
}
