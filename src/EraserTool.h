#pragma once
#include <functional>
#include <QVector2D>
#include <QColor>
#include "Tool.h"
#include "BrushEngine.h"

// Simple eraser implemented by delegating to BrushEngine but forcing a specific color (white for now).
// Note: This behaves as a visual eraser on an opaque composited buffer. For true transparency erasing,
// future work can add 'erase' stroke mode and adjust compositing.
class EraserTool : public Tool {
public:
    using EngineProvider = std::function<BrushEngine*()>;

    explicit EraserTool(EngineProvider provider)
        : m_provider(std::move(provider)) {}

    void onPress(const QVector2D &pos, const QColor &/*color*/, float size) override {
        if (auto *eng = m_provider())
            eng->beginStroke(pos, QColor(), size, BrushStroke::Erase); // transparent erase
    }
    void onMove(const QVector2D &pos) override {
        if (auto *eng = m_provider())
            eng->addPoint(pos);
    }
    void onRelease() override {
        if (auto *eng = m_provider())
            eng->endStroke();
    }
    bool isDrawing() const override {
        if (auto *eng = m_provider()) return eng->isDrawing();
        return false;
    }

private:
    EngineProvider m_provider;
};
