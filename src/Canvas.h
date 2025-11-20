#pragma once
#include <QQuickFramebufferObject>
#include <QVector2D>
#include <QColor>
#include <QList>
#include <QImage>

#include "BrushEngine.h"

class GLRenderer;

class Canvas : public QQuickFramebufferObject {
    Q_OBJECT
    Q_PROPERTY(QColor brushColor READ brushColor WRITE setBrushColor NOTIFY brushColorChanged)
    Q_PROPERTY(float brushSize READ brushSize WRITE setBrushSize NOTIFY brushSizeChanged)
    Q_PROPERTY(int strokeCount READ strokeCount NOTIFY strokeCountChanged)
    Q_PROPERTY(QVector2D cursorPos READ cursorPos NOTIFY cursorPosChanged)

public:
    explicit Canvas(QQuickItem *parent = nullptr);

    Renderer *createRenderer() const override;

    QColor brushColor() const { return m_brushColor; }
    void setBrushColor(const QColor &color);

    float brushSize() const { return m_brushSize; }
    void setBrushSize(float size);

    int strokeCount() const { return m_brush.strokeCount(); }

    QVector2D cursorPos() const { return m_cursorPos; }

    Q_INVOKABLE bool undoLastStroke();
    Q_INVOKABLE bool removeStroke(int index);
    Q_INVOKABLE void clearAllStrokes();
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

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public:
    BrushEngine m_brush;

private:
    QColor m_brushColor;
    float m_brushSize;
    QVector2D m_cursorPos;
    QImage m_baseImage;
};
