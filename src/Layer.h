#pragma once
#include <QObject>
#include <QString>
#include <QImage>
#include "BrushEngine.h"

class Layer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibilityChanged)
public:
    explicit Layer(QObject* parent = nullptr)
        : QObject(parent), m_name("Unnamed"), m_visible(true) {}

    QString name() const { return m_name; }
    void setName(const QString &n) { if (n != m_name) { m_name = n; emit nameChanged(); } }

    bool isVisible() const { return m_visible; }
    void setVisible(bool v) { if (v != m_visible) { m_visible = v; emit visibilityChanged(); } }

    BrushEngine& engine() { return m_engine; }
    const BrushEngine& engine() const { return m_engine; }

    // Optional raster content for this layer (used when importing ORA)
    bool hasRaster() const { return !m_raster.isNull(); }
    const QImage& raster() const { return m_raster; }
    void setRaster(const QImage &img) { m_raster = img; ++m_rasterRevision; }
    void clearRaster() { m_raster = QImage(); ++m_rasterRevision; }
    int rasterRevision() const { return m_rasterRevision; }

    // Operation history (undo/redo) storing before/after states.
    struct FillUndoData {
        QImage rasterBefore;
        QImage rasterAfter;
        QList<BrushStroke> strokesBefore;
        QList<BrushStroke> strokesAfter; // typically empty after fill flatten
    };
    struct Operation {
        enum Type { Stroke, Fill } type; 
        BrushStroke strokeAdded; // valid if type==Stroke
        FillUndoData fillData;   // valid if type==Fill
    };
    void recordStrokeOperation(const BrushStroke &strokeAdded);
    void recordFillOperation(const QImage &rasterBefore,
                             const QList<BrushStroke> &strokesBefore,
                             const QImage &rasterAfter,
                             const QList<BrushStroke> &strokesAfter);
    bool undoLastOperation();
    bool redoLastOperation();
    bool canUndo() const { return m_historyIndex > 0; }
    bool canRedo() const { return m_historyIndex < m_history.size(); }
    void clearHistory() { m_history.clear(); m_historyIndex = 0; }
    void clearAllContent() { m_engine.clearStrokes(); clearRaster(); clearHistory(); }

signals:
    void nameChanged();
    void visibilityChanged();

private:
    QString m_name;
    bool m_visible;
    BrushEngine m_engine;
    QImage m_raster;
    QList<Operation> m_history; // operations applied up to m_historyIndex
    int m_historyIndex = 0;     // points to next operation (0..m_history.size())
    int m_rasterRevision = 0;   // increments whenever raster content changes
};
