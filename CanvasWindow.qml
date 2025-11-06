import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    id: canvasWindow
    visible: true
    flags: Qt.Window
    modality: Qt.WindowModal
    width: initialWidth > 0 ? initialWidth : 800
    height: initialHeight > 0 ? initialHeight : 600
    title: "Canvas - Trahere"
    color: "#ffffff"

    property int initialWidth: 0
    property int initialHeight: 0

    Rectangle {
        anchors.fill: parent
        color: "#c0c0c0"

        Column {
            anchors.fill: parent
            spacing: 0

            // Top menu bar with common menus
            MenuBar {
                id: topMenuBar
                width: parent.width

                Menu { title: "File"
                    MenuItem { text: "New" }
                    MenuItem { text: "Open..." }
                    MenuItem { text: "Save" }
                    MenuItem { text: "Save As..." }
                    MenuSeparator {}
                    MenuItem { text: "Export..." }
                    MenuItem { text: "Close" }
                }

                Menu { title: "Edit"
                    MenuItem { text: "Undo" }
                    MenuItem { text: "Redo" }
                    MenuSeparator {}
                    MenuItem { text: "Cut" }
                    MenuItem { text: "Copy" }
                    MenuItem { text: "Paste" }
                }

                Menu { title: "View"
                    MenuItem { text: "Zoom In" }
                    MenuItem { text: "Zoom Out" }
                    MenuItem { text: "Reset Zoom" }
                }

                Menu { title: "Image"
                    MenuItem { text: "Crop" }
                    MenuItem { text: "Resize" }
                }

                Menu { title: "Layer"
                    MenuItem { text: "New Layer" }
                    MenuItem { text: "Delete Layer" }
                }

                Menu { title: "Select"
                    MenuItem { text: "Select All" }
                    MenuItem { text: "Deselect" }
                }

                Menu { title: "Filter"
                    MenuItem { text: "Blur" }
                    MenuItem { text: "Sharpen" }
                }

                Menu { title: "Tools"
                    MenuItem { text: "Brush" }
                    MenuItem { text: "Eraser" }
                }

                Menu { title: "Setting"
                    MenuItem { text: "Preferences" }
                }

                Menu { title: "Window"
                    MenuItem { text: "Minimize" }
                    MenuItem { text: "Zoom" }
                }

                Menu { title: "Help"
                    MenuItem { text: "Documentation" }
                    MenuItem { text: "About" }
                }
            }

            // White drawing surface centered
            Rectangle {
                id: drawingArea
                width: Math.min(parent.width - 40, canvasWindow.initialWidth)
                height: Math.min(parent.height - topMenuBar.height - 40, canvasWindow.initialHeight)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: topMenuBar.bottom
                anchors.topMargin: 20
                color: "white"
                border.color: "#888"
                border.width: 1

                // Fallback drawing area: simple Canvas-like placeholder (some Qt builds may not include QtQuick.Canvas)
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                }
            }
        }
    }
}
