#pragma once
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QImage>

class Canvas;

class GLRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions {
public:
    explicit GLRenderer(Canvas *canvas);
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

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
};
