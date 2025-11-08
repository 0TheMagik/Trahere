import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Trahere 1.0

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
    // Optional: show an image inside the canvas (e.g., mergedimage from ORA)
    property url imageSource: ""

    Rectangle {
        anchors.fill: parent
        color: "#c0c0c0"

        Column {
            anchors.fill: parent
            spacing: 20

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
                    MenuItem {
                        text: "Undo"
                        onTriggered: glCanvas.undoLastStroke()
                        enabled: glCanvas.strokeCount > 0
                    }
                    MenuItem { text: "Redo" }
                    MenuSeparator {}
                    MenuItem { text: "Cut" }
                    MenuItem { text: "Copy" }
                    MenuItem { text: "Paste" }
                    MenuSeparator {}
                    MenuItem {
                        text: "Clear Canvas"
                        onTriggered: glCanvas.clearAllStrokes()
                        enabled: glCanvas.strokeCount > 0
                    }
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

            // Brush controls
            Row {
                id: brushControls
                width: parent.width
                height: 40
                spacing: 12
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.right: parent.right
                anchors.rightMargin: 20

                Text {
                    text: "Brush size"
                    color: "#333"
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                }

                Slider {
                    id: brushSizeSlider
                    from: 1
                    to: 100
                    value: glCanvas.brushSize
                    width: 240
                    onMoved: glCanvas.brushSize = value
                }

                Text {
                    text: Math.round(glCanvas.brushSize) + " px"
                    color: "#333"
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                }

                Rectangle { width: 1; height: parent.height; color: "#ddd" }

                Button {
                    id: undoBtn
                    text: "Undo"
                    enabled: glCanvas.strokeCount > 0
                    onClicked: glCanvas.undoLastStroke()
                }

                Button {
                    id: clearBtn
                    text: "Clear"
                    enabled: glCanvas.strokeCount > 0
                    onClicked: glCanvas.clearAllStrokes()
                }

                Text {
                    text: "Strokes: " + glCanvas.strokeCount
                    color: "#666"
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                }
            }

            // White drawing surface centered
                Rectangle {
                    id: drawingArea
                    width: Math.min(parent.width - 40, canvasWindow.initialWidth)
                    height: Math.min(parent.height - topMenuBar.height - 40, canvasWindow.initialHeight)
                    x: (parent.width - width) / 2
                    color: "white"
                    border.color: "#888"
                    border.width: 1

                    Canvas {
                        id: glCanvas
                        anchors.fill: parent
                        brushColor: "black"
                        brushSize: 5
                        z: 1
                    }
                }
        }
    }
}
