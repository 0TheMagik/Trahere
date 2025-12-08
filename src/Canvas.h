#pragma once
#include <QQuickFramebufferObject>
#include <QTabletEvent>
#include <QVector2D>
#include <QColor>
#include <QList>
#include <QQmlListProperty>
#include <QImage>
#include <QSize>
#include <QString>
#include <memory>

#include "ToolManager.h"
#include "EraserTool.h"

class GLRenderer;
class QWheelEvent; // forward declaration

class Layer; // forward declaration

class Canvas : public QQuickFramebufferObject {
    Q_OBJECT
    Q_PROPERTY(QColor brushColor READ brushColor WRITE setBrushColor NOTIFY brushColorChanged)
    Q_PROPERTY(float brushSize READ brushSize WRITE setBrushSize NOTIFY brushSizeChanged)
    Q_PROPERTY(int strokeCount READ strokeCount NOTIFY strokeCountChanged)
    Q_PROPERTY(QVector2D cursorPos READ cursorPos NOTIFY cursorPosChanged)
    Q_PROPERTY(int layerCount READ layerCount NOTIFY layerCountChanged)
    Q_PROPERTY(int activeLayerIndex READ activeLayerIndex WRITE setActiveLayerIndex NOTIFY activeLayerIndexChanged)
    Q_PROPERTY(QQmlListProperty<Layer> layers READ layers NOTIFY layerCountChanged)
    // Optional: expose active tool as integer (matches ToolKind ordinal)
    Q_PROPERTY(int activeTool READ activeTool NOTIFY activeToolChanged)
    // Expose derived state for UI enablement
    Q_PROPERTY(bool hasContent READ hasContent NOTIFY strokeCountChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY strokeCountChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY strokeCountChanged)
    // Zoom view (visual scale only; stroke coordinates remain logical)
    Q_PROPERTY(float zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    // Panning (visual translation)
    Q_PROPERTY(float panX READ panX WRITE setPanX NOTIFY panChanged)
    Q_PROPERTY(float panY READ panY WRITE setPanY NOTIFY panChanged)
    // Debug overlay controls and data
    Q_PROPERTY(bool debugOverlay READ debugOverlay WRITE setDebugOverlay NOTIFY debugOverlayChanged)
    Q_PROPERTY(QString debugEvent READ debugEvent NOTIFY debugChanged)
    Q_PROPERTY(float debugPressure READ debugPressure NOTIFY debugChanged)
    Q_PROPERTY(float debugSize READ debugSize NOTIFY debugChanged)
    // Expose enum to QML for readability: Canvas.Brush, Canvas.Eraser, ...

public:
    explicit Canvas(QQuickItem *parent = nullptr);

    // Tool enum exposed to QML
    enum ToolType { Brush = 0, Eraser = 1, Fill = 2 };
    Q_ENUM(ToolType)

    Renderer *createRenderer() const override;

    QColor brushColor() const { return m_brushColor; }
    void setBrushColor(const QColor &color);

    float brushSize() const { return m_brushSize; }
    void setBrushSize(float size);

    int strokeCount() const; // strokes in active layer

    int layerCount() const { return m_layers.size(); }
    int activeLayerIndex() const { return m_activeLayerIndex; }
    void setActiveLayerIndex(int idx);
    const QList<Layer*>& rawLayers() const { return m_layers; }
    QQmlListProperty<Layer> layers();
        // QQmlListProperty helpers (use qsizetype as required by Qt6)
        static qsizetype layersCountFunc(QQmlListProperty<Layer>* prop);
        static Layer* layerAtFunc(QQmlListProperty<Layer>* prop, qsizetype index);

        ~Canvas();
    Layer* activeLayer() const;

    QVector2D cursorPos() const { return m_cursorPos; }
    // Debug accessors
    bool debugOverlay() const { return m_debugOverlay; }
    void setDebugOverlay(bool on) { if (m_debugOverlay != on) { m_debugOverlay = on; emit debugOverlayChanged(); } }
    QString debugEvent() const { return m_debugEvent; }
    float debugPressure() const { return m_debugPressure; }
    float debugSize() const { return m_debugSize; }

    Q_INVOKABLE bool undoLastStroke();
    Q_INVOKABLE bool redoLastStroke();
    Q_INVOKABLE bool removeStroke(int index);
    Q_INVOKABLE void clearAllStrokes();
    Q_INVOKABLE bool hasContent() const; // strokes or raster in active layer
    Q_INVOKABLE bool canUndo() const;    // wrapper for active layer
    Q_INVOKABLE bool canRedo() const;    // wrapper for active layer
    Q_INVOKABLE int addLayer(const QString &name = QString()); // returns new layer index
    Q_INVOKABLE bool removeLayer(int index);
    Q_INVOKABLE void setLayer(int index) { setActiveLayerIndex(index); }
    // Tool management
    Q_INVOKABLE void setActiveTool(int kind);
    Q_INVOKABLE int activeTool() const;
    // Load a base image (PNG/JPEG) that will be drawn under strokes
    Q_INVOKABLE bool loadBaseImage(const QUrl &imageUrl);
    // Save current composited canvas (base + strokes) to .ora
    Q_INVOKABLE bool saveOra(const QUrl &destinationUrl);
    // Save only the painted strokes (transparent background, no base image)
    Q_INVOKABLE bool saveOraStrokesOnly(const QUrl &destinationUrl);
    // Save all layers individually into a multi-layer .ora (OpenRaster) file.
    // Each layer becomes data/layerN.png with N matching its index in internal list.
    // Layer stacking: top-most layer first in stack.xml (reverse of storage order if appended).
    Q_INVOKABLE bool saveOraAllLayers(const QUrl &destinationUrl);
    // Export composited image as QImage (for testing / other saves)
    Q_INVOKABLE QImage compositedImage() const;
    // Export flattened (background putih) PNG dengan semua layer & base image
    Q_INVOKABLE bool exportPng(const QUrl &destinationUrl);
    // Load raster layers from extracted ORA layer image paths (absolute).
    Q_INVOKABLE bool loadOraLayers(const QStringList &layerImagePaths);
    // Zoom accessors
    float zoom() const { return m_zoom; }
    Q_INVOKABLE void setZoom(float z);
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    // Pan accessors
    float panX() const { return m_panX; }
    float panY() const { return m_panY; }
    Q_INVOKABLE void setPanX(float x);
    Q_INVOKABLE void setPanY(float y);
    Q_INVOKABLE void resetView() { setZoom(1.0f); setPanX(0.0f); setPanY(0.0f); }
    
    // Explicitly set the intended document size (pixels) for saving/export
    Q_INVOKABLE void setDocumentSize(int w, int h);
    Q_INVOKABLE int documentWidth() const { return m_documentSize.width(); }
    Q_INVOKABLE int documentHeight() const { return m_documentSize.height(); }
    // Document visual offset (centering) used to keep stroke coords stable across save/load
    Q_INVOKABLE int docOffsetX() const;
    Q_INVOKABLE int docOffsetY() const;

    const QImage &baseImage() const { return m_baseImage; }
    bool hasBaseImage() const { return !m_baseImage.isNull(); }

signals:
    void brushColorChanged();
    void brushSizeChanged();
    void strokeCountChanged();
    void cursorPosChanged();
    void layerCountChanged();
    void activeLayerIndexChanged();
    void activeToolChanged();
    void zoomChanged();
    void panChanged();
    // Debug signals
    void debugOverlayChanged();
    void debugChanged();

protected:
    bool event(QEvent *event) override; // generic; tablet handled in tabletEvent
    void tabletEvent(QTabletEvent *event) ;
    void touchEvent(QTouchEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QColor m_brushColor;
    float m_brushSize;
    QVector2D m_cursorPos;
    QList<Layer*> m_layers;
    int m_activeLayerIndex = -1;
    QImage m_baseImage;
    std::unique_ptr<ToolManager> m_toolMgr;
    std::unique_ptr<EraserTool> m_eraserTool;
    std::unique_ptr<class FillTool> m_fillTool;

    // Zoom state
    float m_zoom = 1.0f;            // current zoom factor
    float m_initialPinchDist = 0.0f; // distance at pinch begin
    float m_initialZoom = 1.0f;      // zoom at pinch begin
    bool m_pinchActive = false;
    // Pan state (logical units)
    float m_panX = 0.0f;
    float m_panY = 0.0f;
    bool m_isPanning = false;
    QVector2D m_lastPanPos;

    // Helpers for unified pointer release handling
    void finalizePointerRelease();
    void updateDebug(const QString &evt, float pressure, float size);
    void handleTablet(QEvent::Type type, const QVector2D &pos, float pressure);

    // Debug state
    bool m_debugOverlay = false;
    QString m_debugEvent;
    float m_debugPressure = 0.0f;
    float m_debugSize = 0.0f;
    // Track active tablet stroke to ignore synthesized mouse
    bool m_inTabletStroke = false;

    // Logical document size (pixels). Used for saving/export to avoid using current view size.
    QSize m_documentSize;
};
