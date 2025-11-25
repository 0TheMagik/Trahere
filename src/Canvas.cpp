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
#include <QTabletEvent>
#include <QTouchEvent>
#include <QQuickWindow>
#include <algorithm>
#include "Layer.h"
#include "FillTool.h"
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
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    // Create initial base layer
    addLayer("Layer 1");
    setActiveLayerIndex(0);

    // Initialize ToolManager with resolver to the current active tool instance (Brush for now)
    m_eraserTool = std::make_unique<EraserTool>([this]() -> BrushEngine* {
        Layer* layer = activeLayer();
        return layer ? &layer->engine() : nullptr;
    });
    m_fillTool = std::make_unique<FillTool>(
        [this]() -> Layer* { return activeLayer(); },
        [this]() -> QSize { return QSize(int(width()), int(height())); }
    );
    m_toolMgr = std::make_unique<ToolManager>([this](ToolKind kind) -> ::Tool* {
        Layer* layer = activeLayer();
        if (!layer) return nullptr;
        switch (kind) {
            case ToolKind::Brush:  return static_cast<::Tool*>(&layer->engine());
            case ToolKind::Eraser: return static_cast<::Tool*>(m_eraserTool.get());
            case ToolKind::Fill:   return static_cast<::Tool*>(m_fillTool.get());
            default: return static_cast<::Tool*>(&layer->engine());
        }
    });

    // Install window event filter when window becomes available
    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *w){
        if (w) w->installEventFilter(this);
    });
}

// Zoom helpers
void Canvas::setZoom(float z) {
    float clamped = std::clamp(z, 0.1f, 10.0f); // reasonable bounds
    if (qFuzzyCompare(clamped, m_zoom)) return;
    m_zoom = clamped;
    setScale(m_zoom); // leverage QQuickItem transform
    emit zoomChanged();
}

void Canvas::zoomIn() {
    setZoom(m_zoom * 1.2f);
}

void Canvas::zoomOut() {
    setZoom(m_zoom / 1.2f);
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

void Canvas::setActiveTool(int kind) {
    if (!m_toolMgr) return;
    ToolKind tk = ToolKind::Brush; // default/fallback
    switch (kind) {
        case Canvas::Brush:  tk = ToolKind::Brush; break;
        case Canvas::Eraser: tk = ToolKind::Eraser; break;
        case Canvas::Fill:   tk = ToolKind::Fill; break;
        default: tk = ToolKind::Brush; break;
    }
    if (static_cast<int>(m_toolMgr->activeTool()) == static_cast<int>(tk)) return;
    m_toolMgr->setActiveTool(tk);
    emit activeToolChanged();
}

// Pan setters
void Canvas::setPanX(float x) {
    if (qFuzzyCompare(m_panX, x)) return;
    m_panX = x;
    emit panChanged();
    update();
}

void Canvas::setPanY(float y) {
    if (qFuzzyCompare(m_panY, y)) return;
    m_panY = y;
    emit panChanged();
    update();
}

int Canvas::activeTool() const {
    if (!m_toolMgr) return 0;
    return static_cast<int>(m_toolMgr->activeTool());
}

bool Canvas::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::TabletPress:
    case QEvent::TabletMove:
    case QEvent::TabletRelease:
    case QEvent::TabletEnterProximity:
    case QEvent::TabletLeaveProximity:
        tabletEvent(static_cast<QTabletEvent*>(event));
        return true;
    default:
        break;
    }
    return QQuickFramebufferObject::event(event);
}

void Canvas::tabletEvent(QTabletEvent *te) {
    const QVector2D pos(te->position());
    const float pressure = std::clamp((float)te->pressure(), 0.0f, 1.0f);
    handleTablet(te->type(), pos, pressure);
    te->accept();
}

void Canvas::touchEvent(QTouchEvent *event) {
    if (!m_toolMgr) { event->ignore(); return; }
    const auto &points = event->points();
    if (points.isEmpty()) { event->ignore(); return; }
    
    // Pinch zoom handling (two-finger gesture)
    if (points.size() >= 2) {
        const auto &p1 = points.at(0);
        const auto &p2 = points.at(1);
        float dist = (p1.position() - p2.position()).manhattanLength();
        switch (event->type()) {
        case QEvent::TouchBegin:
            m_pinchActive = true;
            m_initialPinchDist = dist;
            m_initialZoom = m_zoom;
            break;
        case QEvent::TouchUpdate:
            if (m_pinchActive && m_initialPinchDist > 0.0f) {
                float factor = dist / m_initialPinchDist;
                setZoom(m_initialZoom * factor);
            }
            break;
        case QEvent::TouchEnd:
        case QEvent::TouchCancel:
            m_pinchActive = false;
            break;
        default: break;
        }
        // If pinch active, do not draw strokes with these touches
        if (m_pinchActive) {
            event->accept();
            return;
        }
    }
    
    const auto &p = points.first();
    const QVector2D pos(p.position());
    m_cursorPos = pos;
    emit cursorPosChanged();
    const float pressure = std::clamp((float)p.pressure(), 0.0f, 1.0f);
    const float size = m_brushSize * (0.1f + 0.9f * pressure);

    switch (event->type()) {
    case QEvent::TouchBegin:
        updateDebug(QStringLiteral("TouchBegin"), pressure, size);
        m_toolMgr->onPress(pos - QVector2D(m_panX, m_panY), m_brushColor, size);
        break;
    case QEvent::TouchUpdate:
        updateDebug(QStringLiteral("TouchUpdate"), pressure, size);
        m_toolMgr->setDynamicSize(size);
        m_toolMgr->onMove(pos - QVector2D(m_panX, m_panY));
        break;
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        updateDebug(event->type() == QEvent::TouchEnd ? QStringLiteral("TouchEnd") : QStringLiteral("TouchCancel"), pressure, size);
        m_toolMgr->onRelease();
        finalizePointerRelease();
        break;
    default: break;
    }
    update();
    event->accept();
}

void Canvas::wheelEvent(QWheelEvent *event) {
    // Zoom with Ctrl + wheel (common pattern)
    if (event->modifiers() & Qt::ControlModifier) {
        const int deltaY = event->angleDelta().y();
        if (deltaY != 0) {
            float stepFactor = 1.1f; // base factor per notch
            if (deltaY > 0) zoomIn(); else zoomOut();
            event->accept();
            return;
        }
    }
    QQuickFramebufferObject::wheelEvent(event);
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    if (m_inTabletStroke) { event->ignore(); return; }
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastPanPos = QVector2D(event->position());
        event->accept();
        return;
    }
    m_cursorPos = QVector2D(event->position());
    emit cursorPosChanged();
    updateDebug(QStringLiteral("MousePress"), 1.0f, m_brushSize);
    if (m_toolMgr)
        m_toolMgr->onPress(QVector2D(event->position()) - QVector2D(m_panX, m_panY), m_brushColor, m_brushSize);
    update();
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (m_inTabletStroke) { event->ignore(); return; }
    if (m_isPanning || (event->buttons() & Qt::MiddleButton)) {
        QVector2D cur(event->position());
        QVector2D delta = cur - m_lastPanPos;
        m_lastPanPos = cur;
        m_panX += delta.x();
        m_panY += delta.y();
        emit panChanged();
        update();
        event->accept();
        return;
    }
    m_cursorPos = QVector2D(event->position());
    emit cursorPosChanged();
    updateDebug(QStringLiteral("MouseMove"), 1.0f, m_brushSize);
    if (m_toolMgr)
        m_toolMgr->onMove(QVector2D(event->position()) - QVector2D(m_panX, m_panY));
    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (m_inTabletStroke) { event->ignore(); return; }
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        event->accept();
        return;
    }
    m_cursorPos = QVector2D(event->position());
    emit cursorPosChanged();
    updateDebug(QStringLiteral("MouseRelease"), 1.0f, m_brushSize);
    if (m_toolMgr)
        m_toolMgr->onRelease();
    finalizePointerRelease();
    update();
}

bool Canvas::undoLastStroke() {
    if (!activeLayer()) return false;
    bool ok = activeLayer()->undoLastOperation();
    if (ok) {
        emit strokeCountChanged();
        update();
    }
    return ok;
}

Q_INVOKABLE bool Canvas::redoLastStroke() {
    if (!activeLayer()) return false;
    bool ok = activeLayer()->redoLastOperation();
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

bool Canvas::hasContent() const {
    Layer* al = activeLayer();
    if (!al) return false;
    return al->engine().strokeCount() > 0 || al->hasRaster();
}

bool Canvas::canUndo() const {
    Layer* al = activeLayer();
    return al ? al->canUndo() : false;
}

bool Canvas::canRedo() const {
    Layer* al = activeLayer();
    return al ? al->canRedo() : false;
}

void Canvas::clearAllStrokes() {
    if (!activeLayer()) return;
    // Full clear: strokes, raster, and history so redo/undo stack resets.
    activeLayer()->engine().clearStrokes();
    activeLayer()->clearRaster();
    activeLayer()->clearHistory();
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

void Canvas::finalizePointerRelease() {
    // If active tool produced a stroke (Brush or Eraser), record stroke operation for undo
    if (activeLayer()) {
        int tool = activeTool();
        if (tool == Canvas::Brush || tool == Canvas::Eraser) {
            const auto &strokes = activeLayer()->engine().strokes();
            if (!strokes.isEmpty()) {
                activeLayer()->recordStrokeOperation(strokes.last());
                emit strokeCountChanged();
            }
        } else if (tool == Canvas::Fill) {
            // Fill operation already recorded inside FillTool
            emit strokeCountChanged(); // reflect raster change & history update
        }
    }
}

void Canvas::updateDebug(const QString &evt, float pressure, float size) {
    m_debugEvent = evt;
    m_debugPressure = pressure;
    m_debugSize = size;
    emit debugChanged();
}

void Canvas::handleTablet(QEvent::Type type, const QVector2D &pos, float pressure) {
    m_cursorPos = pos;
    emit cursorPosChanged();
    const float size = m_brushSize * (0.1f + 0.9f * pressure);
    switch (type) {
    case QEvent::TabletPress:
        m_inTabletStroke = true;
        qInfo() << "TabletPress" << "pressure=" << pressure << "pos=" << pos;
        updateDebug(QStringLiteral("TabletPress"), pressure, size);
        if (m_toolMgr) m_toolMgr->onPress(pos - QVector2D(m_panX, m_panY), m_brushColor, size);
        break;
    case QEvent::TabletMove:
        qInfo() << "TabletMove" << "pressure=" << pressure << "pos=" << pos;
        updateDebug(QStringLiteral("TabletMove"), pressure, size);
        if (m_toolMgr) { m_toolMgr->setDynamicSize(size); m_toolMgr->onMove(pos - QVector2D(m_panX, m_panY)); }
        break;
    case QEvent::TabletRelease:
        m_inTabletStroke = false;
        qInfo() << "TabletRelease" << "pressure=" << pressure << "pos=" << pos;
        updateDebug(QStringLiteral("TabletRelease"), pressure, size);
        if (m_toolMgr) m_toolMgr->onRelease();
        finalizePointerRelease();
        break;
    default:
        break;
    }
    update();
}

bool Canvas::eventFilter(QObject *obj, QEvent *event) {
    if (obj == window()) {
        switch (event->type()) {
        case QEvent::TabletPress:
        case QEvent::TabletMove:
        case QEvent::TabletRelease: {
            auto *te = static_cast<QTabletEvent*>(event);
            const QPointF winPos = te->position();
            const QPointF itemPos = mapFromScene(winPos);
            if (itemPos.x() >= 0 && itemPos.y() >= 0 && itemPos.x() <= width() && itemPos.y() <= height()) {
                const float pressure = std::clamp((float)te->pressure(), 0.0f, 1.0f);
                handleTablet(event->type(), QVector2D(itemPos), pressure);
                return true;
            }
            break;
        }
        default:
            break;
        }
    }
    return QQuickFramebufferObject::eventFilter(obj, event);
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
    // Initialize/override document size from base image if not set yet
    if (m_documentSize.width() <= 0 || m_documentSize.height() <= 0) {
        m_documentSize = m_baseImage.size();
    }
    update();
    return true;
}

// Duplicate lightweight rasterization similar to GLRenderer (white bg + base image + strokes)
QImage Canvas::compositedImage() const {
    // Determine target size prioritizing document size, then base/layer sizes, then view size, then fallback
    QSize targetSize = m_documentSize;
    if (targetSize.width() <= 0 || targetSize.height() <= 0) {
        if (!m_baseImage.isNull()) {
            targetSize = m_baseImage.size();
        } else {
            for (Layer* l : m_layers) {
                if (l && l->hasRaster()) { targetSize = l->raster().size(); break; }
            }
        }
    }
    if (targetSize.width() <= 0 || targetSize.height() <= 0) {
        targetSize = QSize(int(width()), int(height()));
    }
    if (targetSize.width() <= 0 || targetSize.height() <= 0) {
        targetSize = QSize(512,512);
    }
    QImage buffer(targetSize, QImage::Format_RGBA8888);
    // Start with opaque white background (similar to renderer) then draw base image if present
    buffer.fill(Qt::white);
    if (!m_baseImage.isNull()) {
        QImage base = m_baseImage;
        if (base.size() != targetSize)
            base = base.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (base.format() != QImage::Format_RGBA8888)
            base = base.convertToFormat(QImage::Format_RGBA8888);
        QPainter bp(&buffer); bp.drawImage(0,0,base); bp.end();
    }
    // Composite all layers bottom -> top including each layer's raster and strokes
    QPainter compPainter(&buffer); compPainter.setRenderHint(QPainter::Antialiasing, true);
    for (int li = 0; li < m_layers.size(); ++li) {
        Layer* layer = m_layers.at(li);
        if (!layer || !layer->isVisible()) continue;
        // Prepare layer image
        QImage layerImg(targetSize, QImage::Format_RGBA8888);
        layerImg.fill(Qt::transparent);
        QPainter lp(&layerImg); lp.setRenderHint(QPainter::Antialiasing, true);
        if (layer->hasRaster()) {
            QImage r = layer->raster();
            if (r.size() != targetSize)
                r = r.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            if (r.format() != QImage::Format_RGBA8888)
                r = r.convertToFormat(QImage::Format_RGBA8888);
            lp.drawImage(0,0,r);
        }
        const auto &strokes = layer->engine().strokes();
        for (const auto &stroke : strokes) {
            if (stroke.points.isEmpty()) continue;
            QPainterPath path(QPointF(stroke.points.first().x(), stroke.points.first().y()));
            for (int i=1;i<stroke.points.size();++i) path.lineTo(stroke.points[i].x(), stroke.points[i].y());
            if (stroke.mode == BrushStroke::Erase) {
                lp.setCompositionMode(QPainter::CompositionMode_Clear);
                QPen pen(Qt::transparent, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                lp.setPen(pen); lp.drawPath(path);
                lp.setCompositionMode(QPainter::CompositionMode_SourceOver);
            } else {
                QPen pen(stroke.color, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                lp.setPen(pen); lp.drawPath(path);
            }
        }
        lp.end();
        compPainter.drawImage(0,0,layerImg);
    }
    compPainter.end();
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
    // Determine size; prefer document size; no base image compositing (transparent background)
    QSize targetSize = m_documentSize;
    if (targetSize.width() <= 0 || targetSize.height() <= 0) {
        if (!m_baseImage.isNull()) targetSize = m_baseImage.size();
        if (targetSize.width() <= 0 || targetSize.height() <= 0) {
            for (Layer* l : m_layers) { if (l && l->hasRaster()) { targetSize = l->raster().size(); break; } }
        }
    }
    if (targetSize.width() <= 0 || targetSize.height() <= 0) targetSize = QSize(int(width()), int(height()));
    if (targetSize.width() <= 0 || targetSize.height() <= 0) targetSize = QSize(512,512);
    QImage buffer(targetSize, QImage::Format_RGBA8888);
    buffer.fill(Qt::transparent);
    QPainter painter(&buffer); painter.setRenderHint(QPainter::Antialiasing, true);
    // Include raster content from all visible layers (since fills produce raster) then strokes
    for (int li=0; li<m_layers.size(); ++li) {
        Layer* layer = m_layers.at(li);
        if (!layer || !layer->isVisible()) continue;
        if (layer->hasRaster()) {
            QImage r = layer->raster();
            if (r.size() != targetSize)
                r = r.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            if (r.format() != QImage::Format_RGBA8888)
                r = r.convertToFormat(QImage::Format_RGBA8888);
            painter.drawImage(0,0,r);
        }
        for (const auto &stroke : layer->engine().strokes()) {
            if (stroke.points.isEmpty()) continue;
            QPainterPath path(QPointF(stroke.points.first().x(), stroke.points.first().y()));
            for (int i=1;i<stroke.points.size();++i) path.lineTo(stroke.points[i].x(), stroke.points[i].y());
            if (stroke.mode == BrushStroke::Erase) {
                painter.setCompositionMode(QPainter::CompositionMode_Clear);
                QPen pen(Qt::transparent, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                painter.setPen(pen); painter.drawPath(path); painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            } else {
                QPen pen(stroke.color, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                painter.setPen(pen); painter.drawPath(path);
            }
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

bool Canvas::saveOraAllLayers(const QUrl &destinationUrl) {
    if (!destinationUrl.isValid()) return false;
    // Determine target size (prefer document size, then base/layer size, then view size, then fallback)
    QSize targetSize = m_documentSize;
    if (targetSize.width() <= 0 || targetSize.height() <= 0) {
        if (!m_baseImage.isNull()) targetSize = m_baseImage.size();
        if (targetSize.width() <= 0 || targetSize.height() <= 0) {
            for (Layer* l : m_layers) { if (l && l->hasRaster()) { targetSize = l->raster().size(); break; } }
        }
    }
    if (targetSize.width() <= 0 || targetSize.height() <= 0) targetSize = QSize(int(width()), int(height()));
    if (targetSize.width() <= 0 || targetSize.height() <= 0) targetSize = QSize(512, 512);

    QList<QImage> layerImages; // first element will be top-most for ORA
    QStringList layerNames;
    QList<bool> visibilityFlags;

    // Build per-layer images. Internal m_layers is assumed bottom->top (new appended layers over earlier ones)
    // For ORA we need top-most first, so iterate reversed.
    for (int li = m_layers.size() - 1; li >= 0; --li) { // top-first for ORA
        Layer* layer = m_layers.at(li);
        if (!layer) continue;
        QImage img(targetSize, QImage::Format_RGBA8888);
        img.fill(Qt::transparent);
        QPainter painter(&img); painter.setRenderHint(QPainter::Antialiasing, true);
        // Draw raster first (contains flattened fills)
        if (layer->hasRaster()) {
            QImage r = layer->raster();
            if (r.size() != targetSize)
                r = r.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            if (r.format() != QImage::Format_RGBA8888)
                r = r.convertToFormat(QImage::Format_RGBA8888);
            painter.drawImage(0,0,r);
        }
        // Then strokes
        const auto &strokes = layer->engine().strokes();
        for (const auto &stroke : strokes) {
            if (stroke.points.isEmpty()) continue;
            QPainterPath path(QPointF(stroke.points.first().x(), stroke.points.first().y()));
            for (int i=1;i<stroke.points.size();++i) path.lineTo(stroke.points[i].x(), stroke.points[i].y());
            if (stroke.mode == BrushStroke::Erase) {
                painter.setCompositionMode(QPainter::CompositionMode_Clear);
                QPen pen(Qt::transparent, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                painter.setPen(pen); painter.drawPath(path); painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            } else {
                QPen pen(stroke.color, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                painter.setPen(pen); painter.drawPath(path);
            }
        }
        painter.end();
        layerImages.append(img);
        layerNames.append(layer->name());
        visibilityFlags.append(layer->isVisible());
    }

    // Optionally include base image as bottom-most layer (appears last in stack.xml, so push back now)
    if (!m_baseImage.isNull()) {
        QImage base = m_baseImage;
        if (base.size() != targetSize) {
            base = base.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        // Base image should be bottom layer => appears last in stack.xml; since we added top-first previously,
        // we append it now so it is logically at the end of <stack>.
        layerImages.append(base);
        layerNames.append(QStringLiteral("Base"));
        visibilityFlags.append(true);
    }

    if (layerImages.isEmpty()) {
        qWarning() << "Canvas.saveOraAllLayers: no layers to save";
        return false;
    }

    // Debug listing
    qWarning() << "Canvas.saveOraAllLayers: preparing" << layerImages.size() << "layers";
    for (int i=0;i<layerNames.size();++i) {
        qWarning() << "  Layer" << i << ": name=" << layerNames[i] << " visible=" << visibilityFlags[i];
    }

    OraCreator creator;
    bool ok = creator.saveOraMulti(destinationUrl, layerImages, layerNames, visibilityFlags);
    if (!ok) {
        qWarning() << "Canvas.saveOraAllLayers: failed" << destinationUrl;
    }
    return ok;
}

bool Canvas::exportPng(const QUrl &destinationUrl) {
    if (!destinationUrl.isValid()) {
        qWarning() << "Canvas.exportPng: invalid url" << destinationUrl;
        return false;
    }
    // Ambil path lokal
    QString localPath = destinationUrl.isLocalFile() ? destinationUrl.toLocalFile() : destinationUrl.toString();
    if (localPath.isEmpty()) {
        qWarning() << "Canvas.exportPng: empty path" << destinationUrl;
        return false;
    }
    // Pastikan extensi .png
    if (!localPath.toLower().endsWith(".png")) {
        localPath += ".png";
    }
    QImage img = compositedImage(); // sudah latar putih + base image + semua layer visible
    if (img.isNull()) {
        qWarning() << "Canvas.exportPng: composited image null";
        return false;
    }
    bool ok = img.save(localPath, "PNG");
    if (!ok) {
        qWarning() << "Canvas.exportPng: gagal menyimpan" << localPath;
    } else {
        qWarning() << "Canvas.exportPng: berhasil" << localPath;
    }
    return ok;
}

bool Canvas::loadOraLayers(const QStringList &layerImagePaths) {
    if (layerImagePaths.isEmpty()) return false;
    // Clear current layers
    while (!m_layers.isEmpty()) {
        Layer* l = m_layers.takeLast();
        if (l) l->deleteLater();
    }
    m_activeLayerIndex = -1;
    emit layerCountChanged();
    emit activeLayerIndexChanged();

    // According to spec, first layer in stack.xml is top-most.
    // We need to append bottom-first so stacking in m_layers is bottom->top.
    for (int i = layerImagePaths.size() - 1; i >= 0; --i) {
        const QString &path = layerImagePaths.at(i);
        QImage img(path);
        if (img.isNull()) {
            qWarning() << "Canvas.loadOraLayers: failed to load layer image" << path;
            continue;
        }
        Layer* layer = new Layer(const_cast<Canvas*>(this));
        layer->setName(QString("Layer %1").arg(m_layers.size()));
        layer->setRaster(img.convertToFormat(QImage::Format_RGBA8888));
        m_layers.append(layer);
        if (m_documentSize.width() <= 0 || m_documentSize.height() <= 0) {
            m_documentSize = img.size();
        }
    }
    emit layerCountChanged();
    if (!m_layers.isEmpty()) {
        setActiveLayerIndex(m_layers.size() - 1); // top layer active
    }
    update();
    return !m_layers.isEmpty();
}
