#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>
#include <QQmlEngine>
#include "OraCreator.h"

int main(int argc, char *argv[])
{
    // Use a non-native Qt Quick Controls style so Control customization (e.g. background)
    // is supported. Call setStyle before creating the application / loading QML.
    QQuickStyle::setStyle("Basic");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Expose OraCreator to QML as Trahere.OraCreator
    qmlRegisterType<OraCreator>("Trahere", 1, 0, "OraCreator");
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Trahere", "Main");

    return app.exec();
}
