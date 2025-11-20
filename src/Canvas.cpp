#include "Canvas.h"
#include "GLRenderer.h"
#include <QMouseEvent>
#include "Layer.h"
Canvas::~Canvas() {
    for (Layer* l : m_layers) {
        if (l) l->deleteLater();
    }
    m_layers.clear();
}

QQmlListProperty<Layer> Canvas::layers() {
    return QQmlListProperty<Layer>(this, this, &Canvas::layersCountFunc, &Canvas::layerAtFunc);
}

qsizetype Canvas::layersCountFunc(QQmlListProperty<Layer>* prop) {
    auto *c = static_cast<Canvas*>(prop->data);
    return static_cast<qsizetype>(c->m_layers.size());
}

Layer* Canvas::layerAtFunc(QQmlListProperty<Layer>* prop, qsizetype index) {
    auto *c = static_cast<Canvas*>(prop->data);
    if (index < 0 || index >= c->m_layers.size()) return nullptr;
    return c->m_layers.at(static_cast<int>(index));
}

Canvas::Canvas(QQuickItem *parent)
    : QQuickFramebufferObject(parent),
      m_brushColor(Qt::black),
      m_brushSize(5.0f),
      m_cursorPos(QVector2D(0,0))
{
    setAcceptedMouseButtons(Qt::AllButtons);
    // Create initial base layer
    addLayer("Layer 1");
    setActiveLayerIndex(0);
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
    if (activeLayer())
        activeLayer()->engine().beginStroke(QVector2D(event->position()), m_brushColor, m_brushSize);
    update();
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    m_cursorPos = QVector2D(event->position());
    emit cursorPosChanged();
    if (activeLayer())
        activeLayer()->engine().addPoint(QVector2D(event->position()));
    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    m_cursorPos = QVector2D(event->position());
    emit cursorPosChanged();
    if (activeLayer()) {
        activeLayer()->engine().endStroke();
        emit strokeCountChanged();
    }
    update();
}

bool Canvas::undoLastStroke() {
    if (!activeLayer()) return false;
    bool ok = activeLayer()->engine().removeLastStroke();
    if (ok) {
        emit strokeCountChanged();
        update();
    }
    return ok;
}

bool Canvas::removeStroke(int index) {
    if (!activeLayer()) return false;
    bool ok = activeLayer()->engine().removeStrokeAt(index);
    if (ok) {
        emit strokeCountChanged();
        update();
    }
    return ok;
}

void Canvas::clearAllStrokes() {
    if (!activeLayer()) return;
    if (activeLayer()->engine().strokeCount() == 0) return;
    activeLayer()->engine().clearStrokes();
    emit strokeCountChanged();
    update();
}

int Canvas::strokeCount() const {
    if (!activeLayer()) return 0;
    return activeLayer()->engine().strokeCount();
}

int Canvas::addLayer(const QString &name) {
    auto *layer = new Layer(const_cast<Canvas*>(this));
    if (!name.isEmpty()) layer->setName(name);
    m_layers.append(layer);
    emit layerCountChanged();
    return m_layers.size() - 1;
}

bool Canvas::removeLayer(int index) {
    if (index < 0 || index >= m_layers.size()) return false;
    Layer* l = m_layers.takeAt(index);
    if (l) l->deleteLater();
    if (m_activeLayerIndex == index) {
        m_activeLayerIndex = m_layers.isEmpty() ? -1 : 0;
        emit activeLayerIndexChanged();
        emit strokeCountChanged();
    }
    emit layerCountChanged();
    update();
    return true;
}

void Canvas::setActiveLayerIndex(int idx) {
    if (idx == m_activeLayerIndex) return;
    if (idx < 0 || idx >= m_layers.size()) return;
    m_activeLayerIndex = idx;
    emit activeLayerIndexChanged();
    emit strokeCountChanged();
    update();
}

Layer* Canvas::activeLayer() const {
    if (m_activeLayerIndex < 0 || m_activeLayerIndex >= m_layers.size()) return nullptr;
    return m_layers[m_activeLayerIndex];
}
