#pragma once
#include <QQuickFramebufferObject>
#include <QVector2D>
#include <QColor>
#include <QList>

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
};
