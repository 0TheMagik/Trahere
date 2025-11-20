#include "Canvas.h"
#include "GLRenderer.h"
#include "../ora/OraCreator.h"
#include <QUrl>
#include <QFileInfo>
#include <QImage>
#include <QBuffer>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QMouseEvent>

Canvas::Canvas(QQuickItem *parent)
    : QQuickFramebufferObject(parent),
      m_brushColor(Qt::black),
      m_brushSize(5.0f),
      m_cursorPos(QVector2D(0,0))
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

QQuickFramebufferObject::Renderer *Canvas::createRenderer() const {
    return new GLRenderer(const_cast<Canvas*>(this));
}

void Canvas::setBrushColor(const QColor &color) {
    if (color != m_brushColor) {
        m_brushColor = color;
        emit brushColorChanged();
    }
}

void Canvas::setBrushSize(float size) {
    if (size != m_brushSize) {
        m_brushSize = size;
        emit brushSizeChanged();
    }
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    m_cursorPos = QVector2D(event->position());
    emit cursorPosChanged();
    m_brush.beginStroke(QVector2D(event->position()), m_brushColor, m_brushSize);
    update();
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    m_cursorPos = QVector2D(event->position());
    emit cursorPosChanged();
    m_brush.addPoint(QVector2D(event->position()));
    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    m_cursorPos = QVector2D(event->position());
    emit cursorPosChanged();
    m_brush.endStroke();
    emit strokeCountChanged();
    update();
}

bool Canvas::undoLastStroke() {
    bool ok = m_brush.removeLastStroke();
    if (ok) {
        emit strokeCountChanged();
        update();
    }
    return ok;
}

bool Canvas::removeStroke(int index) {
    bool ok = m_brush.removeStrokeAt(index);
    if (ok) {
        emit strokeCountChanged();
        update();
    }
    return ok;
}

void Canvas::clearAllStrokes() {
    if (m_brush.strokeCount() == 0) return;
    m_brush.clearStrokes();
    emit strokeCountChanged();
    update();
}

bool Canvas::loadBaseImage(const QUrl &imageUrl) {
    if (!imageUrl.isValid()) return false;
    QString local = imageUrl.isLocalFile() ? imageUrl.toLocalFile() : imageUrl.toString();
    QImage img(local);
    if (img.isNull()) {
        qWarning() << "Canvas.loadBaseImage: failed to load" << local;
        return false;
    }
    m_baseImage = img.convertToFormat(QImage::Format_RGBA8888);
    update();
    return true;
}

// Duplicate lightweight rasterization similar to GLRenderer (white bg + base image + strokes)
QImage Canvas::compositedImage() const {
    // Use window size if available otherwise base image size
    QSize targetSize = QSize(int(width()), int(height()));
    if (targetSize.width() <= 0 || targetSize.height() <= 0) {
        if (!m_baseImage.isNull()) targetSize = m_baseImage.size();
        else targetSize = QSize(512, 512);
    }
    QImage buffer(targetSize, QImage::Format_RGBA8888);
    buffer.fill(Qt::white);
    if (!m_baseImage.isNull()) {
        QImage scaled = (m_baseImage.size() == targetSize) ? m_baseImage : m_baseImage.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QPainter p(&buffer);
        p.drawImage(0, 0, scaled);
        p.end();
    }
    // Simple stroke rendering using QPainter path (does not perfectly match GL stamping but acceptable)
    QPainter painter(&buffer);
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (const auto &stroke : m_brush.strokes()) {
        if (stroke.points.isEmpty()) continue;
        QPen pen(stroke.color, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        QPainterPath path(QPointF(stroke.points.first().x(), stroke.points.first().y()));
        for (int i=1;i<stroke.points.size();++i) {
            path.lineTo(stroke.points[i].x(), stroke.points[i].y());
        }
        painter.drawPath(path);
    }
    painter.end();
    return buffer;
}

bool Canvas::saveOra(const QUrl &destinationUrl) {
    QImage img = compositedImage();
    OraCreator creator;
    bool ok = creator.saveOra(destinationUrl, img);
    if (!ok) {
        qWarning() << "Canvas.saveOra: failed" << destinationUrl;
    }
    return ok;
}

bool Canvas::saveOraStrokesOnly(const QUrl &destinationUrl) {
    // Determine size from existing base image or current item size
    QSize targetSize = !m_baseImage.isNull() ? m_baseImage.size() : QSize(int(width()), int(height()));
    if (targetSize.width() <= 0 || targetSize.height() <= 0) targetSize = QSize(512, 512);
    QImage buffer(targetSize, QImage::Format_RGBA8888);
    buffer.fill(Qt::transparent); // start transparent
    // Preserve previously saved content (flattened strokes) if base image exists
    if (!m_baseImage.isNull()) {
        QImage base = m_baseImage;
        if (base.size() != targetSize) {
            base = base.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        QPainter basePainter(&buffer);
        basePainter.drawImage(0, 0, base);
        basePainter.end();
    }
    QPainter painter(&buffer);
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (const auto &stroke : m_brush.strokes()) {
        if (stroke.points.isEmpty()) continue;
        QPen pen(stroke.color, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        QPainterPath path(QPointF(stroke.points.first().x(), stroke.points.first().y()));
        for (int i=1;i<stroke.points.size();++i) {
            path.lineTo(stroke.points[i].x(), stroke.points[i].y());
        }
        painter.drawPath(path);
    }
    painter.end();
    OraCreator creator;
    bool ok = creator.saveOra(destinationUrl, buffer);
    if (!ok) {
        qWarning() << "Canvas.saveOraStrokesOnly: failed" << destinationUrl;
    }
    return ok;
}
