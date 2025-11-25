#pragma once
#include "Tool.h"
#include <functional>
#include <QImage>
#include <QVector2D>
#include <QColor>
#include <QQueue>
#include <vector>

class Layer;

// Flood fill tool operating on the active layer's raster.
// Ensures raster exists (flattening current strokes first) so boundaries include drawn content.
// Supports transparent clearing when fill color alpha == 0 (region erased).
// Undo/redo supported via Layer operation history (records before/after raster + strokes).
class FillTool : public Tool {
public:
    using LayerProvider = std::function<Layer*()>;
    using SizeProvider  = std::function<QSize()>; // canvas size / viewport

    FillTool(LayerProvider layerProv, SizeProvider sizeProv)
        : m_layerProvider(std::move(layerProv)), m_sizeProvider(std::move(sizeProv)) {}

    void onPress(const QVector2D &pos, const QColor &color, float /*size*/) override;
    void onMove(const QVector2D &/*pos*/) override {}
    void onRelease() override {}
    bool isDrawing() const override { return false; }

private:
    void flattenStrokesIntoRaster(Layer *layer, const QSize &size);
    void floodFill(QImage &img, int sx, int sy, const QColor &targetColor, const QColor &fillColor);

    LayerProvider m_layerProvider;
    SizeProvider  m_sizeProvider;
};