import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
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

    // Palette centralization
    readonly property color uiBg: "#f2f2f3"
    readonly property color uiPanel: "#ffffff"
    readonly property color uiBorder: "#d0d0d2"
    readonly property color uiAccent: "#3daee9"
    readonly property color uiAccentDark: "#1b6fa8"
    readonly property color uiText: "#2e2e2f"
    readonly property color uiSubText: "#666"

    Rectangle {
        anchors.fill: parent
        color: uiBg

        ColumnLayout {
            anchors.fill: parent
            spacing: 20

            // Top menu bar with common menus
            MenuBar {
                id: topMenuBar
                width: parent.width
                background: Rectangle { color: uiPanel; border.color: uiBorder; height: parent.height }

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
                    MenuItem {
                        text: "New Layer"
                        onTriggered: {
                            const idx = glCanvas.addLayer(uniqueLayerName("Layer"))
                            glCanvas.setLayer(idx)
                        }
                    }
                    MenuItem {
                        text: "Delete Layer"
                        enabled: glCanvas.layerCount > 1
                        onTriggered: {
                            // remove active layer (keep at least one)
                            if (glCanvas.layerCount > 1) {
                                glCanvas.removeLayer(glCanvas.activeLayerIndex)
                            }
                        }
                    }
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

            // Brush & status bar
            Rectangle {
                id: brushControls
                Layout.fillWidth: true
                Layout.preferredHeight: 42
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                height: 42
                color: uiPanel
                border.color: uiBorder
                radius: 4

                Row {
                    id: brushControlsRow
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 14

                    Text { text: "Brush"; color: uiText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }

                    Slider {
                        id: brushSizeSlider
                        from: 1
                        to: 100
                        value: glCanvas.brushSize
                        width: 240
                        onMoved: glCanvas.brushSize = value
                        background: Rectangle { implicitHeight: 6; radius: 3; color: "#d8d8da" }
                        handle: Rectangle { width: 18; height: 18; radius: 9; color: uiAccent; border.color: uiAccentDark }
                    }

                    Text { text: Math.round(glCanvas.brushSize) + " px"; color: uiText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }

                    Rectangle { width: 1; height: 24; color: uiBorder; anchors.verticalCenter: parent.verticalCenter }

                    Button { id: undoBtn; text: "Undo"; enabled: glCanvas.strokeCount > 0; onClicked: glCanvas.undoLastStroke(); }

                    Button { id: clearBtn; text: "Clear"; enabled: glCanvas.strokeCount > 0; onClicked: glCanvas.clearAllStrokes() }

                    Rectangle { width: 1; height: 24; color: uiBorder; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "Strokes: " + glCanvas.strokeCount; color: uiSubText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }
                    Text { text: "Layers: " + glCanvas.layerCount + " • Active: " + (glCanvas.activeLayerIndex >=0 ? glCanvas.activeLayerIndex+1 : "-"); color: uiSubText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }
                }
            }

            // Central area with canvas and layer sidebar
            Row {
                id: centralRow
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                // Layer list sidebar
                Rectangle {
                    id: layerSidebar
                    width: 180
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    color: uiPanel
                    border.color: uiBorder
                    radius: 4

                    Column {
                        id: sidebarColumn
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 6

                        Row { id: headerRow; spacing: 6; height: 28; Text { text: "Layers"; font.bold: true; color: uiText }
                            Button { id: addLayerBtn; text: "+"; width: 28; height: 24; onClicked: { const idx = glCanvas.addLayer(uniqueLayerName("Layer")); glCanvas.setLayer(idx) } }
                        }

                        ListView {
                            id: layerList
                            model: glCanvas.layers
                            clip: true
                            width: parent.width
                            height: parent.height - headerRow.height - footerRow.height - 8
                            delegate: Rectangle {
                                id: layerDelegate
                                height: 30
                                width: parent.width
                                color: index === glCanvas.activeLayerIndex ? uiAccent : (mouseArea.hovered ? "#e9e9ea" : "transparent")
                                border.color: index === glCanvas.activeLayerIndex ? uiAccentDark : uiBorder
                                border.width: 1
                                radius: 3
                                property bool editing: false

                                Row {
                                    anchors.fill: parent; anchors.leftMargin: 6; anchors.rightMargin: 6; spacing: 6
                                    CheckBox { checked: modelData.visible; onToggled: modelData.visible = checked }
                                    Text { visible: !layerDelegate.editing; text: modelData.name; color: index === glCanvas.activeLayerIndex ? "white" : uiText; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter }
                                    TextField {
                                        id: renameEdit
                                        visible: layerDelegate.editing
                                        text: modelData.name
                                        selectByMouse: true
                                        onEditingFinished: {
                                            const newName = uniqueLayerName(text.trim().length > 0 ? text.trim() : "Layer", index)
                                            modelData.name = newName
                                            layerDelegate.editing = false
                                        }
                                        color: uiText
                                        background: Rectangle { color: "#ffffff"; border.color: uiBorder; radius: 2 }
                                    }
                                }

                                MouseArea { id: mouseArea; anchors.fill: parent; hoverEnabled: true; onClicked: glCanvas.setLayer(index); onDoubleClicked: layerDelegate.editing = true }
                            }
                        }

                        Row { id: footerRow; spacing: 6; height: 28; Button { text: "Remove"; enabled: glCanvas.layerCount > 1; onClicked: glCanvas.removeLayer(glCanvas.activeLayerIndex) } }
                    }
                }

                // Drawing area
                Rectangle {
                    id: drawingArea
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: Math.min(parent.width - layerSidebar.width - 40, canvasWindow.initialWidth)
                    height: Math.min(parent.height - topMenuBar.height - 40, canvasWindow.initialHeight)
                    color: uiPanel
                    border.color: uiBorder
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

    function uniqueLayerName(base, excludeIndex) {
        let names = [];
        for (let i = 0; i < glCanvas.layerCount; ++i) {
            if (i === excludeIndex) continue;
            names.push(glCanvas.layers[i].name);
        }
        if (names.indexOf(base) === -1)
            return base;
        let n = 2;
        let candidate = base + " " + n;
        while (names.indexOf(candidate) !== -1) {
            n++;
            candidate = base + " " + n;
        }
        return candidate;
    }
}
