#pragma once
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QImage>
// Needed for BrushStroke definition used in snapshots
#include "BrushEngine.h"

class Canvas;

class GLRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions {
public:
    explicit GLRenderer(Canvas *canvas);
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void synchronize(QQuickFramebufferObject *item) override;

private:
    Canvas *m_canvas;
    QOpenGLShaderProgram m_program;
    QOpenGLShaderProgram m_overlayProgram;
    QSize m_viewportSize;
    bool m_initialized = false;
    QImage m_buffer; // CPU canvas buffer
    GLuint m_texture = 0; // GL texture backing the buffer
    int m_rebuildVersion = -1; // track stroke count processed
    QSize m_textureSize; // track texture allocation size
    bool m_bufferDirty = false; // track whether CPU buffer changed and needs GPU upload

    // Snapshots synchronized from GUI thread to render thread
    QList<BrushStroke> m_strokesSnap;
    QList<QVector2D> m_currentPointsSnap;
    QColor m_currentColorSnap;
    float m_currentSizeSnap = 0.0f;
    bool m_isDrawingSnap = false;
    QVector2D m_cursorPosSnap;
    QColor m_brushColorSnap;
    float m_brushSizeSnap = 0.0f;
    qreal m_dpr = 1.0;
};
