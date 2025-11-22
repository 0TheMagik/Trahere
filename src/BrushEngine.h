#pragma once
#include <QVector2D>
#include <QColor>
#include <QList>
#include "Tool.h"

struct BrushStroke {
    QColor color;
    float size;
    QList<QVector2D> points;
    enum StrokeMode { Draw, Erase } mode = Draw;
};

// BrushEngine implements the generic Tool interface as the first concrete tool.
// It records brush strokes composed of multiple sampled points.
class BrushEngine : public Tool {
public:
    // Begin a stroke with optional mode (default Draw). Color ignored for Erase.
    void beginStroke(const QVector2D &pos, const QColor &color, float size, BrushStroke::StrokeMode mode = BrushStroke::Draw);
    void addPoint(const QVector2D &pos);
    void endStroke();

    // Tool interface overrides
    void onPress(const QVector2D &pos, const QColor &color, float size) override { beginStroke(pos, color, size, BrushStroke::Draw); }
    void onMove(const QVector2D &pos) override { addPoint(pos); }
    void onRelease() override { endStroke(); }
    bool isDrawing() const override { return m_drawing; }

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

    // Replace all committed strokes (used for undo of fill operations restoring prior state)
    void replaceStrokes(const QList<BrushStroke> &strokes);
    // Append stroke without treating as current drawing (used for redo)
    void appendStroke(const BrushStroke &stroke);

    // Expose current drawing state so renderer can draw in-progress stroke too
    // isDrawing override provided above for Tool API; kept here for convenience.
    const QList<QVector2D>& currentPoints() const { return m_currentStroke.points; }
    const QColor& currentColor() const { return m_currentStroke.color; }
    float currentSize() const { return m_currentStroke.size; }
    BrushStroke::StrokeMode currentMode() const { return m_currentStroke.mode; }

private:
    QList<BrushStroke> m_strokes;
    BrushStroke m_currentStroke;
    bool m_drawing = false;
};
