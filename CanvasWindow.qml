import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import Trahere 1.0

Window {
    id: canvasWindow
    visible: true
    flags: Qt.Window
    modality: Qt.NonModal
    width: initialWidth > 0 ? initialWidth : 800
    height: initialHeight > 0 ? initialHeight : 600
    title: "Canvas - Trahere"
    color: "#ffffff"

    property int initialWidth: 0
    property int initialHeight: 0
    property url imageSource: ""
    property url fallbackImageSource: ""
    property var layerPaths: []
    property string lastOraPath: ""
    // Default size override (optional)
    // Remove duplicate width/height assignment that was accidentally inserted
    // width: 1000; height: 650  (removed)

    // Theme colors (restored after accidental removal)
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

            MenuBar {
                id: topMenuBar
                width: parent.width
                background: Rectangle { color: uiPanel; border.color: uiBorder; height: parent.height }

                Menu { title: "File"
                    MenuItem { text: "New" }
                    MenuItem { text: "Open..." }
                    MenuItem {
                        text: "Save"
                        enabled: lastOraPath.length > 0
                        onTriggered: {
                            if (lastOraPath.length === 0) return
                            var ok = glCanvas.saveOraAllLayers("file:///" + lastOraPath.replace(/\\/g,"/"))
                            console.log(ok ? "Saved ALL layers ORA:" : "Failed multi-layer save", lastOraPath)
                        }
                    }
                    MenuItem { text: "Save As..."; onTriggered: { savePopup.openMode = "all"; savePopup.open() } }
                    MenuItem { text: "Save Strokes Only..."; onTriggered: { savePopup.openMode = "strokes"; savePopup.open() } }
                    MenuItem { text: "Save All Layers As..."; onTriggered: { savePopup.openMode = "all"; savePopup.open() } }
                    MenuSeparator {}
                    MenuItem { text: "Export..." }
                    MenuItem { text: "Close" }
                }

                Menu { title: "Edit"
                    MenuItem { text: "Undo"; enabled: glCanvas.canUndo; onTriggered: glCanvas.undoLastStroke() }
                    MenuItem { text: "Redo"; enabled: glCanvas.canRedo; onTriggered: glCanvas.redoLastStroke() }
                    MenuSeparator {}
                    MenuItem { text: "Clear Canvas"; enabled: glCanvas.hasContent; onTriggered: glCanvas.clearAllStrokes() }
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
                    MenuItem { text: "New Layer"; onTriggered: { const idx = glCanvas.addLayer(uniqueLayerName("Layer")); glCanvas.setLayer(idx) } }
                    MenuItem { text: "Delete Layer"; enabled: glCanvas.layerCount > 1; onTriggered: glCanvas.removeLayer(glCanvas.activeLayerIndex) }
                }

                Menu { title: "Tools"
                    MenuItem { text: "Brush"; checkable: true; checked: glCanvas.activeTool === Canvas.Brush; onTriggered: glCanvas.setActiveTool(Canvas.Brush) }
                    MenuItem { text: "Eraser"; checkable: true; checked: glCanvas.activeTool === Canvas.Eraser; onTriggered: glCanvas.setActiveTool(Canvas.Eraser) }
                    MenuItem { text: "Fill"; checkable: true; checked: glCanvas.activeTool === Canvas.Fill; onTriggered: glCanvas.setActiveTool(Canvas.Fill) }
                }

                Menu { title: "Help"
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
                    }

                    Text { text: Math.round(glCanvas.brushSize) + " px"; color: uiText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }

                    Rectangle { width: 1; height: 24; color: uiBorder; anchors.verticalCenter: parent.verticalCenter }

                    Button { id: undoBtn; text: "Undo"; enabled: glCanvas.canUndo; onClicked: glCanvas.undoLastStroke() }
                    Button { id: redoBtn; text: "Redo"; enabled: glCanvas.canRedo; onClicked: glCanvas.redoLastStroke() }

                    Button { id: clearBtn; text: "Clear"; enabled: glCanvas.hasContent; onClicked: glCanvas.clearAllStrokes() }

                    Rectangle { width: 1; height: 24; color: uiBorder; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "Strokes: " + glCanvas.strokeCount; color: uiSubText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }
                    Text { text: "Layers: " + glCanvas.layerCount + " • Active: " + (glCanvas.activeLayerIndex >=0 ? glCanvas.activeLayerIndex+1 : "-"); color: uiSubText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }
                }
            }

            // Central area with left tool bar, canvas, right layer sidebar
            Row {
                id: centralRow
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                // Left tools toolbar
                Rectangle {
                    id: leftToolBar
                    width: 70
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    color: uiPanel
                    border.color: uiBorder
                    radius: 4
                    Column {
                        id: toolColumn
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 10
                        Text { text: "Tools"; font.bold: true; color: uiText }
                        Column {
                            spacing: 8
                            // Brush button
                            Rectangle {
                                id: btnBrush
                                width: parent.width - 4
                                height: 34
                                radius: 6
                                color: glCanvas.activeTool === Canvas.Brush ? uiAccent : "transparent"
                                border.color: glCanvas.activeTool === Canvas.Brush ? uiAccentDark : uiBorder
                                border.width: 1
                                Text { text: "Brush"; anchors.centerIn: parent; color: glCanvas.activeTool === Canvas.Brush ? "white" : uiText; font.pixelSize: 12 }
                                MouseArea { anchors.fill: parent; onClicked: glCanvas.setActiveTool(Canvas.Brush) }
                            }
                            // Eraser button
                            Rectangle {
                                id: btnEraser
                                width: parent.width - 4
                                height: 34
                                radius: 6
                                color: glCanvas.activeTool === Canvas.Eraser ? uiAccent : "transparent"
                                border.color: glCanvas.activeTool === Canvas.Eraser ? uiAccentDark : uiBorder
                                border.width: 1
                                Text { text: "Eraser"; anchors.centerIn: parent; color: glCanvas.activeTool === Canvas.Eraser ? "white" : uiText; font.pixelSize: 12 }
                                MouseArea { anchors.fill: parent; onClicked: glCanvas.setActiveTool(Canvas.Eraser) }
                            }
                            // Future tool placeholders (disabled)
                            // Fill button
                            Rectangle {
                                id: btnFill
                                width: parent.width - 4
                                height: 34
                                radius: 6
                                color: glCanvas.activeTool === Canvas.Fill ? uiAccent : "transparent"
                                border.color: glCanvas.activeTool === Canvas.Fill ? uiAccentDark : uiBorder
                                border.width: 1
                                Text { text: "Fill"; anchors.centerIn: parent; color: glCanvas.activeTool === Canvas.Fill ? "white" : uiText; font.pixelSize: 12 }
                                MouseArea { anchors.fill: parent; onClicked: glCanvas.setActiveTool(Canvas.Fill) }
                            }
                            Rectangle {
                                width: parent.width - 4; height: 28; radius: 6
                                color: "#fafafb"; border.color: uiBorder; opacity: 0.5
                                Text { text: "Shape"; anchors.centerIn: parent; color: uiSubText; font.pixelSize: 11 }
                            }
                        }
                    }
                }

                // Drawing area (center)
                Rectangle {
                    id: drawingArea
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: Math.min(parent.width - leftToolBar.width - layerSidebar.width - 40, canvasWindow.initialWidth)
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
                        Component.onCompleted: {
                            if (canvasWindow.layerPaths && canvasWindow.layerPaths.length > 0) {
                                glCanvas.loadOraLayers(canvasWindow.layerPaths)
                            } else if (canvasWindow.imageSource !== "") {
                                if (!glCanvas.loadBaseImage(canvasWindow.imageSource) && canvasWindow.fallbackImageSource !== "") {
                                    glCanvas.loadBaseImage(canvasWindow.fallbackImageSource)
                                }
                            } else if (canvasWindow.fallbackImageSource !== "") {
                                glCanvas.loadBaseImage(canvasWindow.fallbackImageSource)
                            }
                        }
                    }
                }

                // Layer list sidebar (moved to right)
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

                        // Brush / Fill color selection (moved above list for guaranteed visibility)
                        Rectangle {
                            id: colorPanel
                            width: parent.width - 4
                            height: 140
                            radius: 4
                            color: uiPanel
                            border.color: uiBorder
                            property real alphaVal: glCanvas.brushColor.a
                            Column {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 6
                                Row { spacing: 8
                                    Text { text: "Color"; font.pixelSize: 12; color: uiText; font.bold: true }
                                    Rectangle { width: 32; height: 32; radius: 4; border.color: uiBorder; color: glCanvas.brushColor }
                                }
                                Flow { width: parent.width; spacing: 4
                                    Repeater { model: ["#000000", "#404040", "#808080", "#FFFFFF", "#FF0000", "#FFA500", "#FFFF00", "#00FF00", "#00FFFF", "#0000FF", "#FF00FF", "#8B00FF"]
                                        delegate: Rectangle {
                                            width: 22; height: 22; radius: 4
                                            color: modelData
                                            border.color: uiBorder
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: {
                                                    // Explicit hex to RGB parsing to avoid any channel ordering surprises
                                                    var hex = modelData
                                                    if (hex.charAt(0) === '#') hex = hex.substring(1)
                                                    if (hex.length === 3) { // expand shorthand #rgb
                                                        hex = hex[0]+hex[0]+hex[1]+hex[1]+hex[2]+hex[2]
                                                    }
                                                    var r = parseInt(hex.substring(0,2),16)/255.0
                                                    var g = parseInt(hex.substring(2,4),16)/255.0
                                                    var b = parseInt(hex.substring(4,6),16)/255.0
                                                    glCanvas.brushColor = Qt.rgba(r,g,b,colorPanel.alphaVal)
                                                }
                                            }
                                        }
                                    }
                                }
                                Row { spacing: 6
                                    Text { text: "Alpha"; color: uiSubText }
                                    Slider { id: alphaSlider; from:0; to:1; stepSize:0.01; value: colorPanel.alphaVal; width: 100; onMoved: { colorPanel.alphaVal = value; var c=glCanvas.brushColor; glCanvas.brushColor = Qt.rgba(c.r,c.g,c.b,value) } }
                                    Text { text: Math.round(alphaSlider.value*100) + "%"; font.pixelSize: 11; color: uiSubText }
                                }
                                Row { spacing: 10
                                    Text { text: Math.round(glCanvas.brushColor.r*255); color: uiText }
                                    Text { text: Math.round(glCanvas.brushColor.g*255); color: uiText }
                                    Text { text: Math.round(glCanvas.brushColor.b*255); color: uiText }
                                    Text { text: Math.round(glCanvas.brushColor.a*100) + "%"; color: uiText }
                                }
                            }
                        }

                        ListView {
                            id: layerList
                            model: glCanvas.layers
                            clip: true
                            width: parent.width
                            height: parent.height - headerRow.height - footerRow.height - colorPanel.height - 16
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
            }
        }
    }

    // Custom color picker popup (replaces ColorDialog to avoid QuickDialogs dependency)
    // (Removed deprecated popup color picker; using inline palette)

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

    Popup {
        id: savePopup
        modal: true
        focus: true
        x: (canvasWindow.width - width)/2
        y: (canvasWindow.height - height)/2
        width: 360
        height: 170
        padding: 12
        property string openMode: "all" // "all" or "strokes"
        background: Rectangle { color: uiPanel; border.color: uiBorder; radius: 6 }
        contentItem: Column {
            spacing: 8
            Text { text: openMode === "all" ? "Save All Layers (.ora)" : "Save Strokes Only (.ora)"; font.pixelSize: 14; font.bold: true; color: uiText }
            TextField {
                id: savePathField
                placeholderText: "Enter output path (e.g. C:/path/file.ora)"
                text: lastOraPath.length > 0 ? lastOraPath : ""
                selectByMouse: true
                background: Rectangle { color: "white"; border.color: uiBorder; radius: 4 }
            }
            Row { spacing: 12
                Button {
                    text: "Save"
                    onClicked: {
                        var localPath = savePathField.text.trim()
                        if (localPath.length === 0) return
                        if (!localPath.toLowerCase().endsWith(".ora")) localPath += ".ora"
                        lastOraPath = localPath
                        var urlStr = "file:///" + localPath.replace(/\\/g,"/")
                        var ok = (openMode === "all") ? glCanvas.saveOraAllLayers(urlStr) : glCanvas.saveOraStrokesOnly(urlStr)
                        console.log(ok ? (openMode === "all" ? "Saved ALL layers ORA:" : "Saved strokes-only ORA:") : "Failed save", localPath)
                        savePopup.close()
                    }
                }
                Button { text: "Cancel"; onClicked: savePopup.close() }
            }
        }
    }
}
