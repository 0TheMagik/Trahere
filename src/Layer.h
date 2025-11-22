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
    void setRaster(const QImage &img) { m_raster = img; }
    void clearRaster() { m_raster = QImage(); }

signals:
    void nameChanged();
    void visibilityChanged();

private:
    QString m_name;
    bool m_visible;
    BrushEngine m_engine;
    QImage m_raster;
};
