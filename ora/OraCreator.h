#pragma once

#include <QObject>
#include <QUrl>

class OraCreator : public QObject
{
    Q_OBJECT
public:
    explicit OraCreator(QObject *parent = nullptr);

    // Create a minimal .ora file at destinationPath with an empty layer of given size.
    Q_INVOKABLE bool createOra(const QString &destinationPath, int width = 100, int height = 100);

    // Overload for QML 'url' type; converts URL to local file path and delegates.
    Q_INVOKABLE bool createOra(const QUrl &destinationUrl, int width, int height);
};
