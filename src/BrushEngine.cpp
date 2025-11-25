#include "BrushEngine.h"
#include <utility>
#include <cmath>

void BrushEngine::beginStroke(const QVector2D &pos, const QColor &color, float size, BrushStroke::StrokeMode mode) {
    BrushStroke stroke;
    stroke.color = color;
    stroke.size = size;
    stroke.points = {pos};
    stroke.widths = {size};
    stroke.mode = mode;
    m_currentStroke = std::move(stroke);
    m_drawing = true;
}

void BrushEngine::addPoint(const QVector2D &pos) {
    if (m_drawing) {
        // Thin the point stream to reduce memory and stamping cost.
        // Use a size-proportional threshold (quarter of current diameter).
        const float threshold = std::max(0.5f, m_currentStroke.size * 0.25f);
        bool append = true;
        if (!m_currentStroke.points.isEmpty()) {
            QVector2D last = m_currentStroke.points.back();
            if ((pos - last).length() < threshold) append = false;
        }
        if (append) {
            m_currentStroke.points.append(pos);
            m_currentStroke.widths.append(m_currentStroke.size);
        }
    }
}

void BrushEngine::endStroke() {
    if (m_drawing) {
        // Move the current stroke into the list to avoid unnecessary copies
        m_strokes.append(std::move(m_currentStroke));
        // Reset current stroke to defaults
        m_currentStroke = BrushStroke{};
        m_drawing = false;
    }
}

bool BrushEngine::removeLastStroke() {
    if (m_strokes.isEmpty())
        return false;
    m_strokes.removeLast();
    return true;
}

bool BrushEngine::removeStrokeAt(int index) {
    if (index < 0 || index >= m_strokes.size())
        return false;
    m_strokes.removeAt(index);
    return true;
}

void BrushEngine::clearStrokes() {
    m_strokes.clear();
}

void BrushEngine::replaceStrokes(const QList<BrushStroke> &strokes) {
    m_strokes = strokes;
}

void BrushEngine::appendStroke(const BrushStroke &stroke) {
    m_strokes.append(stroke);
}

void BrushEngine::setCurrentSize(float size) {
    if (m_drawing) {
        m_currentStroke.size = size;
    }
}
