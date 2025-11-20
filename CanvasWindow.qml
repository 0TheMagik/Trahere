import QtQuick 
import QtQuick.Window
import QtQuick.Controls 
import QtQuick.Dialogs
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
    // Fallback layer image (e.g., data/layer0.png) if mergedimage.png not present
    property url fallbackImageSource: ""
    // Stores local filesystem path (without file:/// prefix)
    property string lastOraPath: ""

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
                    MenuItem {
                        text: "Save"
                        onTriggered: {
                            if (lastOraPath.length === 0) {
                                console.log("No ORA path set; ignoring Save")
                                return
                            }
                            var ok = glCanvas.saveOraStrokesOnly("file:///" + lastOraPath.replace(/\\/g,"/"))
                            console.log(ok ? "Saved strokes-only ORA:" : "Failed strokes-only save", lastOraPath)
                        }
                    }
                    MenuItem {
                        text: "Save As..."
                        onTriggered: saveOraDialog.open()
                    }
                    MenuItem {
                        text: "Save Strokes Only..."
                        onTriggered: saveStrokesDialog.open()
                    }
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
                        Component.onCompleted: {
                            if (canvasWindow.imageSource !== "") {
                                if (!glCanvas.loadBaseImage(canvasWindow.imageSource) && canvasWindow.fallbackImageSource !== "") {
                                    glCanvas.loadBaseImage(canvasWindow.fallbackImageSource)
                                }
                            } else if (canvasWindow.fallbackImageSource !== "") {
                                glCanvas.loadBaseImage(canvasWindow.fallbackImageSource)
                            }
                        }
                    }
                }
        }
    }

    FileDialog {
        id: saveOraDialog
        title: "Save Canvas as .ora"
        fileMode: FileDialog.SaveFile
        nameFilters: ["OpenRaster (*.ora)", "All files (*)"]
        onAccepted: {
            var urlStr = String(selectedFile)
            // Extract local path from file URL
            var localPath = urlStr.startsWith("file:///") ? urlStr.substring(8) : urlStr
            // Ensure Windows drive keeps colon (substring above preserves it)
            if (!localPath.toLowerCase().endsWith(".ora")) localPath += ".ora"
            lastOraPath = localPath
            var saveOk = glCanvas.saveOraStrokesOnly("file:///" + lastOraPath.replace(/\\/g,"/"))
            console.log(saveOk ? "Saved strokes-only ORA:" : "Failed strokes-only ORA", lastOraPath)
        }
    }

    FileDialog {
        id: saveStrokesDialog
        title: "Save Strokes (transparent) as .ora"
        fileMode: FileDialog.SaveFile
        nameFilters: ["OpenRaster (*.ora)", "All files (*)"]
        onAccepted: {
            var urlStr = String(selectedFile)
            var localPath = urlStr.startsWith("file:///") ? urlStr.substring(8) : urlStr
            if (!localPath.toLowerCase().endsWith(".ora")) localPath += ".ora"
            lastOraPath = localPath
            var ok = glCanvas.saveOraStrokesOnly("file:///" + localPath.replace(/\\/g,"/"))
            console.log(ok ? "Saved strokes-only ORA:" : "Failed save strokes-only ORA", localPath)
        }
    }
}
