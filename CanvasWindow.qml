import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs 6.5
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
                z: 60
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
                    MenuItem { text: "Save As"; 
                        onTriggered: { savePopup.openMode = "all"; savePopup.open() } 
                        
                        }
                    MenuSeparator {}
                    MenuItem { text: "Export PNG..."; onTriggered: pngExportDialog.open() }
                    MenuItem { text: "Close" }
                }

                Menu { title: "Edit"
                    MenuItem { text: "Undo"; enabled: glCanvas.canUndo; onTriggered: glCanvas.undoLastStroke() }
                    MenuItem { text: "Redo"; enabled: glCanvas.canRedo; onTriggered: glCanvas.redoLastStroke() }
                    MenuSeparator {}
                    MenuItem { text: "Clear Canvas"; enabled: glCanvas.hasContent; onTriggered: glCanvas.clearAllStrokes() }
                }

                Menu { title: "View"
                    MenuItem { text: "Zoom In"; onTriggered: glCanvas.zoomIn() }
                    MenuItem { text: "Zoom Out"; onTriggered: glCanvas.zoomOut() }
                    MenuItem { text: "Reset Zoom"; onTriggered: glCanvas.resetView() }
                    MenuSeparator {}
                    MenuItem { text: glCanvas.debugOverlay ? "Hide Debug Overlay" : "Show Debug Overlay"; onTriggered: glCanvas.debugOverlay = !glCanvas.debugOverlay }
                }

                // Removed Image menu per request

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
                    // Reserve space on the right for Undo/Redo row
                    anchors.rightMargin: 160
                    spacing: 14

                    // Active tool display with icon
                    Row {
                        spacing: 6
                        anchors.verticalCenter: parent.verticalCenter
                        Image {
                            width: 20; height: 20; fillMode: Image.PreserveAspectFit
                            source: glCanvas.activeTool === Canvas.Brush ? "Images/brush-svgrepo-com.svg"
                                   : glCanvas.activeTool === Canvas.Eraser ? "Images/eraser-svgrepo-com.svg"
                                   : glCanvas.activeTool === Canvas.Fill ? "Images/fill-solid-svgrepo-com.svg" : ""
                        }
                        Text {
                            text: glCanvas.activeTool === Canvas.Brush ? "Brush" : (glCanvas.activeTool === Canvas.Eraser ? "Eraser" : (glCanvas.activeTool === Canvas.Fill ? "Fill" : "Tool"))
                            color: uiText
                            font.pixelSize: 12
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

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

                    // Removed Undo/Redo here and Clear button; moved Undo/Redo to right side
                    Rectangle { width: 1; height: 24; color: uiBorder; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "Strokes: " + glCanvas.strokeCount; color: uiSubText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }
                    Text { text: "Layers: " + glCanvas.layerCount + " • Active: " + (glCanvas.activeLayerIndex >=0 ? glCanvas.activeLayerIndex+1 : "-"); color: uiSubText; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }
                }

                // Top-right Undo/Redo controls
                Row {
                    id: undoRedoRow
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8
                    Button { id: undoBtnTop; text: "Undo"; enabled: glCanvas.canUndo; onClicked: glCanvas.undoLastStroke() }
                    Button { id: redoBtnTop; text: "Redo"; enabled: glCanvas.canRedo; onClicked: glCanvas.redoLastStroke() }
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
                        // Label at the top for the tools column
                        Text { text: "Tools"; font.bold: true; color: uiText }

                        // Quick-access colored tool swatches (top-to-bottom: Brush, Eraser, Fill)
                        Column {
                            spacing: 8
                            anchors.horizontalCenter: parent.horizontalCenter

                            Rectangle {
                                id: swatchBrush
                                width: 36
                                height: 36
                                radius: 6
                                color: "transparent"
                                border.color: uiBorder
                                border.width: 1
                                Image { anchors.centerIn: parent; source: "Images/brush-svgrepo-com.svg"; width: 24; height: 24; fillMode: Image.PreserveAspectFit }
                                MouseArea { anchors.fill: parent; onClicked: glCanvas.setActiveTool(Canvas.Brush) }
                            }

                            Rectangle {
                                id: swatchEraser
                                width: 36
                                height: 36
                                radius: 6
                                color: "transparent"
                                border.color: uiBorder
                                border.width: 1
                                Image { anchors.centerIn: parent; source: "Images/eraser-svgrepo-com.svg"; width: 24; height: 24; fillMode: Image.PreserveAspectFit }
                                MouseArea { anchors.fill: parent; onClicked: glCanvas.setActiveTool(Canvas.Eraser) }
                            }

                            Rectangle {
                                id: swatchFill
                                width: 36
                                height: 36
                                radius: 6
                                color: "transparent"
                                border.color: uiBorder
                                border.width: 1
                                Image { anchors.centerIn: parent; source: "Images/fill-solid-svgrepo-com.svg"; width: 24; height: 24; fillMode: Image.PreserveAspectFit }
                                MouseArea { anchors.fill: parent; onClicked: glCanvas.setActiveTool(Canvas.Fill) }
                            }
                        }
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
                                Row {
                                    anchors.centerIn: parent
                                    spacing: 6
                                    Image { source: "Images/brush-svgrepo-com.svg"; width: 18; height: 18; fillMode: Image.PreserveAspectFit }
                                    Text { text: "Brush"; color: glCanvas.activeTool === Canvas.Brush ? "white" : uiText; font.pixelSize: 12 }
                                }
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
                                Row {
                                    anchors.centerIn: parent
                                    spacing: 6
                                    Image { source: "Images/eraser-svgrepo-com.svg"; width: 18; height: 18; fillMode: Image.PreserveAspectFit }
                                    Text { text: "Eraser"; color: glCanvas.activeTool === Canvas.Eraser ? "white" : uiText; font.pixelSize: 12 }
                                }
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
                                Row {
                                    anchors.centerIn: parent
                                    spacing: 6
                                    Image { source: "Images/fill-solid-svgrepo-com.svg"; width: 18; height: 18; fillMode: Image.PreserveAspectFit }
                                    Text { text: "Fill"; color: glCanvas.activeTool === Canvas.Fill ? "white" : uiText; font.pixelSize: 12 }
                                }
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
                    clip: true
                    z: 10
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: Math.min(parent.width - leftToolBar.width - layerSidebar.width - 40, canvasWindow.initialWidth)
                    height: Math.min(parent.height - topMenuBar.height - 40, canvasWindow.initialHeight)
                    // Make the surrounding area gray so the document (paper) stands out as white
                    color: uiBg
                    border.color: uiBorder
                    border.width: 1
                    Canvas {
                        id: glCanvas
                        transformOrigin: Item.Center
                        anchors.fill: parent
                        brushColor: "black"
                        brushSize: 5
                        z: 1
                        Component.onCompleted: {
                            glCanvas.debugOverlay = true
                            // For newly created docs (no layerPaths), set document size from initialWidth/Height.
                            // When opening existing ORA (layerPaths provided), don't override; size will come from layers/stack.
                            if ((!canvasWindow.layerPaths || canvasWindow.layerPaths.length === 0)
                                && canvasWindow.initialWidth > 0 && canvasWindow.initialHeight > 0) {
                                glCanvas.setDocumentSize(canvasWindow.initialWidth, canvasWindow.initialHeight)
                            }
                            if (canvasWindow.layerPaths && canvasWindow.layerPaths.length > 0) {
                                // Opening ORA: set document size from stack.xml (w,h) if available
                                var w = oraLoader.imageWidth()
                                var h = oraLoader.imageHeight()
                                if (w > 0 && h > 0) {
                                    glCanvas.setDocumentSize(w, h)
                                }
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
                    // Document background (the white "paper" inside the gray drawing area)
                    Rectangle {
                        id: docOverlay
                        // Compute displayed document size in view coordinates (apply item scale)
                        property real docW: (glCanvas.documentWidth() > 0 ? glCanvas.documentWidth() * glCanvas.scale : Math.min(parent.width - 40, parent.height - 40))
                        property real docH: (glCanvas.documentHeight() > 0 ? glCanvas.documentHeight() * glCanvas.scale : Math.min(parent.height - 40, parent.width - 40))
                        width: docW
                        height: docH
                        x: (parent.width - width)/2 + glCanvas.panX * glCanvas.scale
                        y: (parent.height - height)/2 + glCanvas.panY * glCanvas.scale
                        color: "white"
                        border.color: uiBorder
                        border.width: 2
                        radius: 2
                        z: 0 // keep under the actual canvas so canvas drawing appears on top
                        enabled: false // don't block mouse events to underlying canvas
                    }
                    // Debug Overlay
                    Rectangle {
                        id: debugOverlay
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                        anchors.margins: 8
                        z: 10
                        color: "#66000000"
                        radius: 4
                        border.color: "#80ffffff"
                        visible: glCanvas.debugOverlay
                        property int pad: 6
                        width: contentRow.implicitWidth + pad * 2
                        height: contentRow.implicitHeight + pad * 2
                        Row {
                            id: contentRow
                            anchors.fill: parent
                            anchors.margins: debugOverlay.pad
                            spacing: 10
                            Text { text: "evt: " + glCanvas.debugEvent; color: "white"; font.pixelSize: 12 }
                            Text { text: "pressure: " + glCanvas.debugPressure.toFixed(3); color: "white"; font.pixelSize: 12 }
                            Text { text: "size: " + glCanvas.debugSize.toFixed(2) + " px"; color: "white"; font.pixelSize: 12 }
                            Text { text: "zoom: " + (glCanvas.zoom * 100).toFixed(0) + "%"; color: "white"; font.pixelSize: 12 }
                        }
                    }
                }

                // Layer list sidebar (moved to right)
                Rectangle {
                    id: layerSidebar
                    z: 40
                    width: 200 // widened for cleaner layout
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

                        Row { id: headerRow; spacing: 6; height: 28; Text { text: "Layers"; font.bold: true; color: uiText } }

                        // (Brush/Fill color moved to bottom-left indicator)

                        // Spacer under header
                        Item { height: 4; width: 1 }

                        ListView {
                            id: layerList
                            model: glCanvas.layers
                            clip: true
                            width: parent.width
                            spacing: 6
                            // Height: total height minus header & footer & margins
                            height: parent.height - headerRow.height - footerRow.height - 8 - 8 - 4
                            ScrollBar.vertical: ScrollBar { }
                            delegate: Rectangle {
                                id: layerDelegate
                                height: 36
                                width: parent.width - 12
                                anchors.left: parent.left
                                anchors.leftMargin: 6
                                anchors.right: parent.right
                                anchors.rightMargin: 6
                                color: index === glCanvas.activeLayerIndex ? uiAccent : (mouseArea.containsMouse ? "#f5f5f6" : "#ffffff")
                                border.color: index === glCanvas.activeLayerIndex ? uiAccentDark : uiBorder
                                border.width: 1
                                radius: 4
                                property bool editing: false

                                Row {
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    spacing: 8
                                    // Visibility toggle box (keeps checkbox fully inside card)
                                    Item {
                                        width: 24; height: 24
                                        anchors.verticalCenter: parent.verticalCenter
                                        CheckBox {
                                            anchors.centerIn: parent
                                            checked: modelData.visible
                                            onToggled: modelData.visible = checked
                                            leftPadding: 0; rightPadding: 0; topPadding: 0; bottomPadding: 0
                                        }
                                    }
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
                                        background: Rectangle { color: "#ffffff"; border.color: uiBorder; radius: 3 }
                                    }
                                }

                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: glCanvas.setLayer(index)
                                    onDoubleClicked: layerDelegate.editing = true
                                }
                            }
                        }

                        Row { id: footerRow; spacing: 8; height: 32
                            Button { id: addLayerBtn; text: "+"; width: 32; height: 28; onClicked: { const idx = glCanvas.addLayer(uniqueLayerName("Layer")); glCanvas.setLayer(idx) } }
                            Button { text: "Remove"; width: 70; height: 28; enabled: glCanvas.layerCount > 1; onClicked: glCanvas.removeLayer(glCanvas.activeLayerIndex) }
                        }

                    }
                }
            }
        }
    }
    // Small color indicator placed bottom-left (next to left toolbar)
    Rectangle {
        id: bottomLeftColorIndicator
        width: 40
        height: 40
        radius: 6
        color: glCanvas.brushColor
        border.color: uiBorder
        border.width: 1
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        z: 70

        MouseArea { anchors.fill: parent; onClicked: { colorDialog.selectedColor = glCanvas.brushColor; colorDialog.open() } }
    }

    // Custom color picker popup (replaces ColorDialog to avoid QuickDialogs dependency)
    // (Removed deprecated popup color picker; using inline palette)

    // Standard Qt color dialog for picking colors (with alpha support)
    ColorDialog {
        id: colorDialog
        title: "Choose Color"
        selectedColor: glCanvas.brushColor
        options: ColorDialog.ShowAlphaChannel
        onAccepted: {
            // Apply selected color to tools that use color (Brush and Fill use brushColor)
            glCanvas.brushColor = selectedColor
        }
    }

    FileDialog {
        id: pngExportDialog
        title: "Export PNG"
        fileMode: FileDialog.SaveFile
        nameFilters: ["PNG Image (*.png)"]
        onAccepted: {
            var url = selectedFile
            if (!url) return
            // Append .png if missing (robust)
            var lower = url.toString().toLowerCase()
            if (!lower.endsWith(".png")) {
                url = url + ".png"
            }
            var ok = glCanvas.exportPng(url)
            console.log(ok ? "PNG exported:" : "PNG export failed", url)
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
            Text { text: savePopup.openMode === "all" ? "Save All Layers (.ora)" : "Save Strokes Only (.ora)"; font.pixelSize: 14; font.bold: true; color: uiText }
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
                        var ok = (savePopup.openMode === "all") ? glCanvas.saveOraAllLayers(urlStr) : glCanvas.saveOraStrokesOnly(urlStr)
                        console.log(ok ? (openMode === "all" ? "Saved ALL layers ORA:" : "Saved strokes-only ORA:") : "Failed save", localPath)
                        savePopup.close()
                    }
                }
                Button { text: "Cancel"; onClicked: savePopup.close() }
            }
        }
    }
}
