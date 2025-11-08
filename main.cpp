#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>
#include <QQmlEngine>
#include "ora/OraCreator.h"
#include "ora/OraLoader.h"

int main(int argc, char *argv[])
{
    // Use a non-native Qt Quick Controls style so Control customization (e.g. background)
    // is supported. Call setStyle before creating the application / loading QML.
    QQuickStyle::setStyle("Basic");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Expose C++ types to QML under the Trahere 1.0 import
    qmlRegisterType<OraCreator>("Trahere", 1, 0, "OraCreator");
    qmlRegisterType<OraLoader>("Trahere", 1, 0, "OraLoader");
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Trahere", "Main");

    return app.exec();
}
