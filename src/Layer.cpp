#include "Layer.h"
#include <QDebug>

void Layer::recordStrokeOperation(const BrushStroke &strokeAdded) {
	// Trim redo tail if we are not at end
	while (m_history.size() > m_historyIndex) m_history.removeLast();
	Operation op; op.type = Operation::Stroke; op.strokeAdded = strokeAdded;
	m_history.append(op);
	m_historyIndex = m_history.size();
}

void Layer::recordFillOperation(const QImage &rasterBefore,
								const QList<BrushStroke> &strokesBefore,
								const QImage &rasterAfter,
								const QList<BrushStroke> &strokesAfter) {
	while (m_history.size() > m_historyIndex) m_history.removeLast();
	Operation op; op.type = Operation::Fill;
	op.fillData.rasterBefore = rasterBefore;
	op.fillData.strokesBefore = strokesBefore;
	op.fillData.rasterAfter = rasterAfter;
	op.fillData.strokesAfter = strokesAfter;
	m_history.append(op);
	m_historyIndex = m_history.size();
}

bool Layer::undoLastOperation() {
	if (!canUndo()) return false;
	// Move index back, then revert using before state
	m_historyIndex--; 
	const Operation &op = m_history[m_historyIndex];
	if (op.type == Operation::Stroke) {
		bool removed = m_engine.removeLastStroke();
		if (!removed) qWarning() << "Layer::undoLastOperation: stroke removal failed";
		return removed;
	} else if (op.type == Operation::Fill) {
		m_raster = op.fillData.rasterBefore;
		m_engine.replaceStrokes(op.fillData.strokesBefore);
		return true;
	}
	return false;
}

bool Layer::redoLastOperation() {
	if (!canRedo()) return false;
	const Operation &op = m_history[m_historyIndex];
	if (op.type == Operation::Stroke) {
		// Reapply stroke
		m_engine.appendStroke(op.strokeAdded);
	} else if (op.type == Operation::Fill) {
		m_raster = op.fillData.rasterAfter;
		m_engine.replaceStrokes(op.fillData.strokesAfter);
	}
	m_historyIndex++;
	return true;
}
