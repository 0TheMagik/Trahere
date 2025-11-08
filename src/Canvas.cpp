#include "Canvas.h"
#include "GLRenderer.h"
#include <QMouseEvent>

Canvas::Canvas(QQuickItem *parent)
    : QQuickFramebufferObject(parent),
      m_brushColor(Qt::black),
      m_brushSize(5.0f)
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
    m_brush.beginStroke(QVector2D(event->position()), m_brushColor, m_brushSize);
    update();
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    m_brush.addPoint(QVector2D(event->position()));
    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
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
