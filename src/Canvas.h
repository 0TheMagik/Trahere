#pragma once
#include <QQuickFramebufferObject>
#include <QVector2D>
#include <QColor>
#include <QList>
#include <QQmlListProperty>
#include <QImage>

#include "BrushEngine.h"

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

public:
    explicit Canvas(QQuickItem *parent = nullptr);

    Renderer *createRenderer() const override;

    QColor brushColor() const { return m_brushColor; }
    void setBrushColor(const QColor &color);

    float brushSize() const { return m_brushSize; }
    void setBrushSize(float size);

    int strokeCount() const; // strokes in active layer

    int layerCount() const { return m_layers.size(); }
    int activeLayerIndex() const { return m_activeLayerIndex; }
    void setActiveLayerIndex(int idx);
    QList<Layer*> rawLayers() const { return m_layers; }
    QQmlListProperty<Layer> layers();
        // QQmlListProperty helpers (use qsizetype as required by Qt6)
        static qsizetype layersCountFunc(QQmlListProperty<Layer>* prop);
        static Layer* layerAtFunc(QQmlListProperty<Layer>* prop, qsizetype index);

        ~Canvas();
    Layer* activeLayer() const;

    QVector2D cursorPos() const { return m_cursorPos; }

    Q_INVOKABLE bool undoLastStroke();
    Q_INVOKABLE bool removeStroke(int index);
    Q_INVOKABLE void clearAllStrokes();
    Q_INVOKABLE int addLayer(const QString &name = QString()); // returns new layer index
    Q_INVOKABLE bool removeLayer(int index);
    Q_INVOKABLE void setLayer(int index) { setActiveLayerIndex(index); }
    // Load a base image (PNG/JPEG) that will be drawn under strokes
    Q_INVOKABLE bool loadBaseImage(const QUrl &imageUrl);
    // Save current composited canvas (base + strokes) to .ora
    Q_INVOKABLE bool saveOra(const QUrl &destinationUrl);
    // Save only the painted strokes (transparent background, no base image)
    Q_INVOKABLE bool saveOraStrokesOnly(const QUrl &destinationUrl);
    // Export composited image as QImage (for testing / other saves)
    Q_INVOKABLE QImage compositedImage() const;

    const QImage &baseImage() const { return m_baseImage; }
    bool hasBaseImage() const { return !m_baseImage.isNull(); }

signals:
    void brushColorChanged();
    void brushSizeChanged();
    void strokeCountChanged();
    void cursorPosChanged();
    void layerCountChanged();
    void activeLayerIndexChanged();

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
};
