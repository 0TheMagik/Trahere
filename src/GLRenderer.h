#pragma once
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

class Canvas;

class GLRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions {
public:
    explicit GLRenderer(Canvas *canvas);
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    Canvas *m_canvas;
    QOpenGLShaderProgram m_program;
    QSize m_viewportSize;
};
