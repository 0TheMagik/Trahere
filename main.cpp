#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include "src/Canvas.h"
#include "src/Layer.h"
#include <QQmlEngine>
#include <QQmlContext>
#include "ora/OraCreator.h"
#include "ora/OraLoader.h"
#include "recentfilesmanager.h"

int main(int argc, char *argv[])
{
    // Use a non-native Qt Quick Controls style so Control customization (e.g. background)
    // is supported. Call setStyle before creating the application / loading QML.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    // Allow mouse synthesis for controls; Canvas ignores synth mouse during tablet strokes
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
    QGuiApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTabletEvents, true);
#endif
    // Force Wintab path for tablets (user reported Wintab drivers)
    // Set TRAHERE_FORCE_INK=1 to override and use Windows Ink instead.
#if defined(Q_OS_WIN)
    if (qEnvironmentVariableIntValue("TRAHERE_FORCE_INK") == 1) {
        qputenv("QT_TABLET_WINDOWS_INK", "1");
    } else {
        qputenv("QT_TABLET_WINDOWS_INK", "0");
    }
#endif
    QQuickStyle::setStyle("Basic");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<Canvas>("Trahere", 1, 0, "Canvas");
    qmlRegisterType<Layer>("Trahere", 1, 0, "Layer");

    // Expose C++ types to QML under the Trahere 1.0 import
    qmlRegisterType<OraCreator>("Trahere", 1, 0, "OraCreator");
    qmlRegisterType<OraLoader>("Trahere", 1, 0, "OraLoader");
    qmlRegisterType<RecentFilesModel>("Trahere", 1, 0, "RecentFilesModel");

    // Create and set a singleton instance for easy access
    RecentFilesModel *recentFilesModel = new RecentFilesModel(&engine);
    engine.rootContext()->setContextProperty("recentFilesModel", recentFilesModel);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Trahere", "Main");

    return app.exec();
}
