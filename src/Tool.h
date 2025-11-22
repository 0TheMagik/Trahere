#pragma once
#include <QVector2D>
#include <QColor>

// Base interface for drawing tools. Each tool reacts to pointer lifecycle.
// onPress: begin an action at position with given color & size (size optional semantics depend on tool).
// onMove: continue action as pointer moves.
// onRelease: finalize the action.
// isDrawing: returns true if tool currently has an in-progress action (for preview rendering).
class Tool {
public:
    virtual ~Tool() = default;
    virtual void onPress(const QVector2D &pos, const QColor &color, float size) = 0;
    virtual void onMove(const QVector2D &pos) = 0;
    virtual void onRelease() = 0;

    virtual bool isDrawing() const { return false; }
};
