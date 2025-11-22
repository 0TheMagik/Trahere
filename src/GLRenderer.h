#pragma once
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QImage>
// Needed for BrushStroke definition used in snapshots
#include "BrushEngine.h"
#include <QList>

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
    int m_rebuildVersion = -1; // track content version processed
    QSize m_textureSize; // track texture allocation size
    bool m_bufferDirty = false; // track whether CPU buffer changed and needs GPU upload

    // Snapshots synchronized from GUI thread to render thread
    struct LayerSnap {
        QImage raster;                // optional raster image of this layer
        QList<BrushStroke> strokes;   // committed strokes for this layer
        bool visible = true;
    };
    QList<LayerSnap> m_layersSnap;    // stacking order: bottom -> top
    QList<QVector2D> m_currentPointsSnap;
    QColor m_currentColorSnap;
    float m_currentSizeSnap = 0.0f;
    BrushStroke::StrokeMode m_currentModeSnap = BrushStroke::Draw;
    bool m_isDrawingSnap = false;
    int m_activeLayerIndexSnap = -1;
    QVector2D m_cursorPosSnap;
    QColor m_brushColorSnap;
    float m_brushSizeSnap = 0.0f;
    qreal m_dpr = 1.0;

    // Performance caching
    // Cached per-layer rendered image (all committed strokes + raster) excluding any in-progress stroke.
    QVector<QImage> m_cachedLayerImages;
    // Version signature per layer to know when to rebuild the cached image.
    // Computed from stroke count, per-stroke point counts, and erase stroke count.
    QVector<qint64> m_cachedLayerVersions;
    // Track last in-progress point count stamped (to allow incremental stamping if desired).
    int m_lastInProgressStampedCount = 0;
    // Track previous drawing state to decide composition path.
    bool m_prevWasDrawing = false;
    // Composite of all cached layers except active (rebuilt only when those layers change)
    QImage m_baseCompositeExcludingActive;
    // Active layer clone with incremental in-progress stroke stamping
    QImage m_activeInProgressImage;
    // Track whether m_buffer currently represents baseComposite + active layer so we can stamp deltas
    bool m_incrementalCompositeValid = false;
};
