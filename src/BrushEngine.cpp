#include "BrushEngine.h"
#include <utility>

void BrushEngine::beginStroke(const QVector2D &pos, const QColor &color, float size) {
    m_currentStroke = BrushStroke{color, size, {pos}};
    m_drawing = true;
}

void BrushEngine::addPoint(const QVector2D &pos) {
    if (m_drawing)
        m_currentStroke.points.append(pos);
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
