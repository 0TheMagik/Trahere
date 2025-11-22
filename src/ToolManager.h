#pragma once
#include <functional>
#include <QVector2D>
#include <QColor>
#include "Tool.h"

// Kind/type of active tool. Extend as new tools are implemented.
enum class ToolKind {
    Brush,
    Eraser,
    Fill
    // Shape,
};

// A resolver that returns the Tool instance for the current context (e.g., active layer)
// for a given ToolKind. This lets ToolManager stay decoupled from Canvas/Layer.
using ToolResolver = std::function<Tool*(ToolKind)>;

class ToolManager {
public:
    explicit ToolManager(ToolResolver resolver)
        : m_resolver(std::move(resolver)) {}

    void setActiveTool(ToolKind kind) { m_active = kind; }
    ToolKind activeTool() const { return m_active; }

    void onPress(const QVector2D &pos, const QColor &color, float size) {
        if (Tool* t = resolve()) t->onPress(pos, color, size);
    }
    void onMove(const QVector2D &pos) {
        if (Tool* t = resolve()) t->onMove(pos);
    }
    void onRelease() {
        if (Tool* t = resolve()) t->onRelease();
    }
    bool isDrawing() const {
        if (Tool* t = const_cast<ToolManager*>(this)->resolve()) return t->isDrawing();
        return false;
    }

private:
    Tool* resolve() { return m_resolver ? m_resolver(m_active) : nullptr; }

    ToolResolver m_resolver;
    ToolKind m_active { ToolKind::Brush };
};
