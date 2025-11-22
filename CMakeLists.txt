cmake_minimum_required(VERSION 3.16)

project(Trahere VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Concurrent)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appTrahere
    main.cpp
    src/Canvas.h
    src/Canvas.cpp
    src/Tool.h
    src/ToolManager.h
    src/EraserTool.h
    src/FillTool.h
    src/FillTool.cpp
    src/Layer.h
    src/Layer.cpp
    src/BrushEngine.cpp
    src/BrushEngine.h
    src/GLRenderer.cpp
    src/GLRenderer.h
    ora/OraCreator.h
    ora/OraCreator.cpp
    ora/OraLoader.h
    ora/OraLoader.cpp
    recentfilesmanager.h
    recentfilesmanager.cpp
)

qt_add_qml_module(appTrahere
    URI Trahere
    QML_FILES
        Main.qml
        CanvasWindow.qml
    RESOURCES
        Images/Trahere_logo.png
        Images/open_logo.png
        Images/setting_logo.png
        Images/draw_logo.png
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appTrahere PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appTrahere
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appTrahere
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
    Qt6::Concurrent
)

include(GNUInstallDirs)
install(TARGETS appTrahere
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
