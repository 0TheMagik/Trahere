#pragma once
#include <QQuickFramebufferObject>
#include <QVector2D>
#include <QColor>
#include <QList>
#include <QQmlListProperty>
#include <QImage>
#include <memory>

#include "ToolManager.h"
#include "EraserTool.h"

class GLRenderer;

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
    // Load raster layers from extracted ORA layer image paths (absolute).
    Q_INVOKABLE bool loadOraLayers(const QStringList &layerImagePaths);

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

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

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
};
