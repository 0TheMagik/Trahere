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
    if (activeLayer()) {
        for (const auto &stroke : activeLayer()->engine().strokes()) {
            if (stroke.points.isEmpty()) continue;
            QPen pen(stroke.color, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter.setPen(pen);
            QPainterPath path(QPointF(stroke.points.first().x(), stroke.points.first().y()));
            for (int i=1;i<stroke.points.size();++i) {
                path.lineTo(stroke.points[i].x(), stroke.points[i].y());
            }
            painter.drawPath(path);
        }
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
    if (activeLayer()) {
        for (const auto &stroke : activeLayer()->engine().strokes()) {
            if (stroke.points.isEmpty()) continue;
            QPen pen(stroke.color, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter.setPen(pen);
            QPainterPath path(QPointF(stroke.points.first().x(), stroke.points.first().y()));
            for (int i=1;i<stroke.points.size();++i) {
                path.lineTo(stroke.points[i].x(), stroke.points[i].y());
            }
            painter.drawPath(path);
        }
    }
    painter.end();
    OraCreator creator;
    bool ok = creator.saveOra(destinationUrl, buffer);
    if (!ok) {
        qWarning() << "Canvas.saveOraStrokesOnly: failed" << destinationUrl;
    }
    return ok;
}
