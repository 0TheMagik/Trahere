#include "FillTool.h"
#include "Layer.h"
#include "BrushEngine.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

void FillTool::onPress(const QVector2D &pos, const QColor &color, float /*unused*/) {
    Layer *layer = m_layerProvider ? m_layerProvider() : nullptr;
    if (!layer) return;
    QSize size = m_sizeProvider ? m_sizeProvider() : QSize();
    if (size.width() <= 0 || size.height() <= 0) {
        qWarning() << "FillTool: invalid canvas size" << size;
        return;
    }
    // Capture pre-fill state for undo
    QImage prevRaster = layer->raster();
    QList<BrushStroke> prevStrokes = layer->engine().strokes();
    // Ensure raster exists and flatten strokes so boundaries are respected.
    flattenStrokesIntoRaster(layer, size);
    QImage raster = layer->raster();
    if (raster.size() != size) {
        raster = raster.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    if (raster.format() != QImage::Format_RGBA8888)
        raster = raster.convertToFormat(QImage::Format_RGBA8888);

    int x = int(std::round(pos.x()));
    int y = int(std::round(pos.y()));
    if (x < 0 || x >= raster.width() || y < 0 || y >= raster.height()) return;

    QColor targetColor = QColor::fromRgba(raster.pixel(x,y));
    if (targetColor == color) return; // no-op if already filled

    floodFill(raster, x, y, targetColor, color);
    layer->setRaster(raster);
    // After fill, strokes flattened; clear strokes
    layer->engine().clearStrokes();
    // Record operation with before and after states
    layer->recordFillOperation(prevRaster, prevStrokes, raster, QList<BrushStroke>());
}

void FillTool::flattenStrokesIntoRaster(Layer *layer, const QSize &size) {
    if (!layer) return;
    // Start from existing raster or transparent base.
    QImage base(size, QImage::Format_RGBA8888);
    base.fill(Qt::transparent);
    if (layer->hasRaster()) {
        QImage r = layer->raster();
        if (r.size() != size)
            r = r.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (r.format() != QImage::Format_RGBA8888)
            r = r.convertToFormat(QImage::Format_RGBA8888);
        QPainter bp(&base); bp.drawImage(0,0,r); bp.end();
    }
    const auto &strokes = layer->engine().strokes();
    if (!strokes.isEmpty()) {
        QPainter painter(&base);
        painter.setRenderHint(QPainter::Antialiasing, true);
        for (const auto &stroke : strokes) {
            if (stroke.points.isEmpty()) continue;
            QPainterPath path(QPointF(stroke.points.first().x(), stroke.points.first().y()));
            for (int i=1;i<stroke.points.size();++i) path.lineTo(stroke.points[i].x(), stroke.points[i].y());
            if (stroke.mode == BrushStroke::Erase) {
                painter.setCompositionMode(QPainter::CompositionMode_Clear);
                QPen pen(Qt::transparent, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                painter.setPen(pen); painter.drawPath(path);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            } else {
                QPen pen(stroke.color, stroke.size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                painter.setPen(pen); painter.drawPath(path);
            }
        }
        painter.end();
    }
    layer->setRaster(base);
}

void FillTool::floodFill(QImage &img, int sx, int sy, const QColor &targetColor, const QColor &fillColor) {
    if (targetColor == fillColor) return;
    const int w = img.width();
    const int h = img.height();
    auto sameColor = [&](QRgb px) {
        return QColor::fromRgba(px) == targetColor;
    };
    std::vector<uint8_t> visited(size_t(w) * size_t(h), 0);
    QQueue<QPoint> q;
    q.enqueue(QPoint(sx, sy));
    visited[size_t(sy) * size_t(w) + size_t(sx)] = 1;

    const bool doClear = (fillColor.alpha() == 0); // support transparent clear fill
    const QRgb fillRgb = fillColor.rgba();
    while (!q.isEmpty()) {
        QPoint p = q.dequeue();
        int x = p.x(); int y = p.y();
        if (doClear) {
            img.setPixel(x, y, qRgba(0,0,0,0));
        } else {
            img.setPixel(x, y, fillRgb);
        }
        // 4-neighbors
        const int dx[4] = {1,-1,0,0};
        const int dy[4] = {0,0,1,-1};
        for (int k=0;k<4;++k) {
            int nx = x + dx[k]; int ny = y + dy[k];
            if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
            size_t idx = size_t(ny) * size_t(w) + size_t(nx);
            if (visited[idx]) continue;
            if (sameColor(img.pixel(nx, ny))) {
                visited[idx] = 1;
                q.enqueue(QPoint(nx, ny));
            }
        }
    }
}