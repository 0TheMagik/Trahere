#pragma once
#include <QVector2D>
#include <QColor>
#include <QList>

struct BrushStroke {
    QColor color;
    float size;
    QList<QVector2D> points;
};

class BrushEngine {
public:
    void beginStroke(const QVector2D &pos, const QColor &color, float size);
    void addPoint(const QVector2D &pos);
    void endStroke();

    const QList<BrushStroke>& strokes() const { return m_strokes; }

    // Stroke management
    // Remove the most recently committed stroke. Returns true if removed.
    bool removeLastStroke();
    // Remove a stroke at index [0..count-1]. Returns true if removed.
    bool removeStrokeAt(int index);
    // Clear all committed strokes.
    void clearStrokes();
    // Number of committed strokes.
    int strokeCount() const { return m_strokes.size(); }

    // Expose current drawing state so renderer can draw in-progress stroke too
    bool isDrawing() const { return m_drawing; }
    const QList<QVector2D>& currentPoints() const { return m_currentStroke.points; }
    const QColor& currentColor() const { return m_currentStroke.color; }
    float currentSize() const { return m_currentStroke.size; }

private:
    QList<BrushStroke> m_strokes;
    BrushStroke m_currentStroke;
    bool m_drawing = false;
};
