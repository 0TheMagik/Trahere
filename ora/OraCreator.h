#pragma once

#include <QObject>
#include <QUrl>
#include <QImage>
#include <QList>
#include <QStringList>

class OraCreator : public QObject
{
    Q_OBJECT
public:
    explicit OraCreator(QObject *parent = nullptr);

    // Create a minimal .ora file at destinationPath with an empty layer of given size.
    Q_INVOKABLE bool createOra(const QString &destinationPath, int width = 100, int height = 100);

    // Overload for QML 'url' type; converts URL to local file path and delegates.
    Q_INVOKABLE bool createOra(const QUrl &destinationUrl, int width, int height);

    // Save an ORA using provided raster image as single layer (data/layer0.png).
    Q_INVOKABLE bool saveOra(const QString &destinationPath, const QImage &layerImg);
    Q_INVOKABLE bool saveOra(const QUrl &destinationUrl, const QImage &layerImg);

    // Save an ORA with multiple layers. The first image in layerImages is the top layer.
    // layerNames (same size) provides names; visibility flags indicate if layer is visible.
    // All layers are positioned at (0,0) with full opacity for now.
    Q_INVOKABLE bool saveOraMulti(const QString &destinationPath,
                                  const QList<QImage> &layerImages,
                                  const QStringList &layerNames,
                                  const QList<bool> &visibilityFlags);
    Q_INVOKABLE bool saveOraMulti(const QUrl &destinationUrl,
                                  const QList<QImage> &layerImages,
                                  const QStringList &layerNames,
                                  const QList<bool> &visibilityFlags);
};
