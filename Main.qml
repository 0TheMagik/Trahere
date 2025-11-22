import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Trahere 1.0
import QtQml

ApplicationWindow {
    id: window
    visible: true
    width: 1920
    height: 1080
    title: "Trahere"
    color: "#DCDCDD"

    // Paper size presets with dimensions in pixels at different DPI
    property var paperSizes: {
        "Custom": {width: 4961, height: 7016, dpi: 600},
        "A1 (300 ppi)": {width: 7008, height: 9933, dpi: 300},
        "A1 (600 ppi)": {width: 7008, height: 11811, dpi: 600},
        "A2 (300 ppi)": {width: 4960, height: 7016, dpi: 300},
        "A2 (600 ppi)": {width: 9922, height: 14032, dpi: 600},
        "A3 (300 ppi)": {width: 3508, height: 4960, dpi: 300},
        "A3 (600 ppi)": {width: 7016, height: 9920, dpi: 600},
        "A4 (300 ppi)": {width: 2480, height: 3508, dpi: 300},
        "A4 (600 ppi)": {width: 4961, height: 7016, dpi: 600},
        "A5 (300 ppi)": {width: 1754, height: 2480, dpi: 300},
        "A5 (600 ppi)": {width: 3508, height: 4960, dpi: 600},
        "A6 (300 ppi)": {width: 1240, height: 1754, dpi: 300},
        "A6 (600 ppi)": {width: 2480, height: 3496, dpi: 600}
    }

    // Main Content Area
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left Sidebar
        Rectangle {
            Layout.preferredWidth: 80
            Layout.fillHeight: true
            color: "#FFFFFF"

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 16
                spacing: 8

                // Logo
                Image {
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    Layout.alignment: Qt.AlignHCenter
                    source: "Images/Trahere_logo.png"
                    fillMode: Image.PreserveAspectFit
                }

                Item { Layout.preferredHeight: 24 }

                // Draw Tool
                ToolButton {
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    Layout.alignment: Qt.AlignHCenter

                    contentItem: Column {
                        spacing: 4

                        Image {
                            id: draw_logo
                            width: 40
                            height: 40
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "Images/draw_logo.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Draw"
                            color: "#3daee9"
                            font.pixelSize: 11
                        }
                    }

                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        createDocWindow.show()
                    }
                }

                // Open Tool
                ToolButton {
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    Layout.alignment: Qt.AlignHCenter

                    contentItem: Column {
                        spacing: 4

                        Image {
                            width: 40
                            height: 40
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "Images/open_logo.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Open"
                            color: "#3daee9"
                            font.pixelSize: 11
                        }
                    }

                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        fileDialog.open()
                    }
                }

                Item { Layout.fillHeight: true }

                // Settings Button
                ToolButton {
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 16

                    contentItem: Image {
                        source: "Images/setting_logo.png"
                        fillMode: Image.PreserveAspectFit
                    }

                    background: Rectangle {
                        color: parent.hovered ? "#f0f0f0" : "transparent"
                        radius: 4
                    }
                }
            }
        }
    }

    // Title and Search Bar - Overlayed on top
    Column {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 16
        anchors.leftMargin: 110
        spacing: 12
        z: 1

        // Title
        Text {
            text: "Your Project"
            font.pixelSize: 48
            font.family: "Brush Script MT, cursive"
            color: "#31363b"
        }

        // Search Bar and Refresh Button Row
        RowLayout {
            spacing: 8

            // Search Bar
            Rectangle {
                Layout.preferredWidth: 320
                Layout.preferredHeight: 44
                color: "white"
                radius: 22
                border.color: "#bdc3c7"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 8

                    Text {
                        text: "🔍"
                        font.pixelSize: 18
                        color: "#7f8c8d"
                    }

                    TextField {
                        id: searchField
                        Layout.fillWidth: true
                        placeholderText: "Search..."
                        color: "#2c3e50"
                        font.pixelSize: 14

                        background: Rectangle {
                            color: "transparent"
                        }

                        onTextChanged: {
                            filterProxy.filterText = text
                        }
                    }
                }
            }

            // Refresh Button (red box style)
            Rectangle {
                id: refreshButtonRect
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                // Dynamic hover/refresh color binding
                color: refreshState.isRefreshing ? "#DCDCDD" : (refreshMouseArea.containsMouse ? "#E0E0E0" : "#DCDCDD")
                radius: 4
                border.color: refreshState.isRefreshing ? "#DCDCDD" : "#DCDCDD"
                border.width: 2

                MouseArea {
                    id: refreshMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !refreshState.isRefreshing
                    onClicked: {
                        console.log("Refreshing recent files...")
                        refreshState.isRefreshing = true
                        recentFilesModel.refresh()
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: refreshState.isRefreshing ? "⟳" : "⟳"
                    font.pixelSize: 20
                    color: "black"
                    opacity: refreshState.isRefreshing ? 0.7 : 1.0
                    
                    // Rotating animation during refresh
                    RotationAnimation on rotation {
                        running: refreshState.isRefreshing
                        from: 0
                        to: 360
                        duration: 1500
                        loops: Animation.Infinite
                    }
                }

                // No imperative color updates needed; binding above reacts automatically
            }
        }
    }

    // Filter proxy for search functionality
    QtObject {
        id: filterProxy
        property string filterText: ""

        function matches(fileName, filePath) {
            if (filterText === "") return true
            var lowerFilter = filterText.toLowerCase()
            return fileName.toLowerCase().includes(lowerFilter) || 
                   filePath.toLowerCase().includes(lowerFilter)
        }
    }

    // Track refresh state
    QtObject {
        id: refreshState
        property bool isRefreshing: false
    }

    // Connect to background scan finished signal
    Connections {
        target: recentFilesModel
        function onBackgroundScanFinished() {
            console.log("Background scan finished")
            refreshState.isRefreshing = false
        }
    }

    // Recent Files Section (flat style, not boxed)
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 180
        anchors.leftMargin: 110
        // Make recent files occupy the main workspace area (subtract margins)
        width: parent.width - 220
        height: parent.height - 240
        color: "transparent"
        border.width: 0
        radius: 0
        z: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 4
            spacing: 6

            // Title (kept visually separate but not boxed)
            // Removed anchors.* inside layout-managed item to avoid warnings
            Text {
                text: "Recent Files"
                font.pixelSize: 16
                font.bold: true
                color: "#31363b"
                Layout.alignment: Qt.AlignLeft
            }

            // ListView for recent files (flat rows with separators)
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: recentFilesModel
                clip: true
                spacing: 0

                delegate: Item {
                    width: ListView.view.width
                    height: visible ? 52 : 0
                    visible: filterProxy.matches(model.fileName, model.filePath)

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            console.log("Opening:", model.filePath)
                            let dirPath = oraLoader.loadOra(model.filePath)
                            if (dirPath.length === 0) {
                                console.log("Failed to load .ora")
                                return
                            }
                            console.log("Extracted .ora to temp:", dirPath, "stack:", oraLoader.stackXmlPath())
                            function pathToUrl(p) { return "file:///" + p.replace(/\\\\/g, "/") }
                            var merged = pathToUrl(dirPath + "/mergedimage.png")
                            var layer0 = pathToUrl(dirPath + "/data/layer0.png")
                            var paths = oraLoader.layerImagePaths()

                            var comp = Qt.createComponent("CanvasWindow.qml")
                            if (comp.status === Component.Ready) {
                                var win = comp.createObject(window, { initialWidth: 1200, initialHeight: 800, imageSource: merged, fallbackImageSource: layer0, layerPaths: paths })
                            } else {
                                console.log("Canvas component not ready:", comp.status, comp.errorString())
                            }
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: model.fileName
                                font.pixelSize: 13
                                font.bold: true
                                color: mouseArea.containsMouse ? "#222222" : "#31363b"
                                elide: Text.ElideRight
                            }

                            Text {
                                text: model.filePath
                                font.pixelSize: 10
                                color: "#7f8c8d"
                                elide: Text.ElideMiddle
                                Layout.fillWidth: true
                            }
                        }

                        Text {
                            text: model.dateModified
                            font.pixelSize: 10
                            color: "#95a5a6"
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    // Separator line
                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 1
                        color: "#e9e9e9"
                    }
                }

                ScrollBar.vertical: ScrollBar { active: true }

                // Empty state overlay
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    visible: recentFilesModel.rowCount() === 0

                    Text {
                        anchors.centerIn: parent
                        text: "No recent files"
                        color: "#bfc4c6"
                        font.pixelSize: 12
                    }
                }
            }
        }
    }

    // File Dialog for Opening Files
    FileDialog {
        id: fileDialog
        title: "Open File - Trahere"
        nameFilters: ["OpenRaster (*.ora)", "Image files (*.png *.jpg *.jpeg *.bmp *.kra)", "All files (*)"]
        onAccepted: {
            console.log("Selected file:", fileDialog.selectedFile)
            if (String(fileDialog.selectedFile).toLowerCase().endsWith(".ora")) {
                let dirPath = oraLoader.loadOra(fileDialog.selectedFile)
                if (dirPath.length === 0) {
                    console.log("Failed to load .ora")
                    return
                }
                console.log("Extracted .ora to temp:", dirPath, "stack:", oraLoader.stackXmlPath())
                // Quick preview: show mergedimage.png if exists, else first layer (data/layer0.png)
                function pathToUrl(p) { return "file:///" + p.replace(/\\\\/g, "/") }
                var merged = pathToUrl(dirPath + "/mergedimage.png")
                var layer0 = pathToUrl(dirPath + "/data/layer0.png")
                var paths = oraLoader.layerImagePaths()

                // Derive local path of opened ORA (strip file:/// prefix) for auto-save target
                var openedUrl = String(fileDialog.selectedFile)
                var localOpenedPath = openedUrl.startsWith("file:///") ? openedUrl.substring(8) : openedUrl

                // Create a preview window using CanvasWindow
                var comp = Qt.createComponent("CanvasWindow.qml")
                if (comp.status === Component.Ready) {
                    var win = comp.createObject(window, { initialWidth: 1200, initialHeight: 800, imageSource: merged, fallbackImageSource: layer0, lastOraPath: localOpenedPath, layerPaths: paths })
                } else {
                    console.log("Canvas component not ready:", comp.status, comp.errorString())
                }
            } else {
                loadedImage.source = fileDialog.selectedFile
            }
        }
        onRejected: console.log("File selection canceled")
    }

    // Hidden image used to store the loaded file path so FileDialog can set it
    Image {
        id: loadedImage
        visible: false
    }

    // Create New Document Window (Draggable)
    Window {
        id: createDocWindow
        title: "Create new document - Trahere"
        width: 650
        height: 700
        modality: Qt.NonModal
        flags: Qt.Window
        color: "#3c3f41"

        // Custom Title Bar for Dragging
        Rectangle {
            id: titleBar
            width: parent.width
            height: 40
            color: "#3c3f41"
            z: 100

            MouseArea {
                id: titleBarMouseArea
                anchors.fill: parent
                property point clickPos: Qt.point(0, 0)

                onPressed: (mouse) => {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }

                onPositionChanged: (mouse) => {
                    var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                    createDocWindow.x += delta.x
                    createDocWindow.y += delta.y
                }
            }

            // RowLayout {
            //     anchors.fill: parent
            //     anchors.leftMargin: 12
            //     anchors.rightMargin: 8

            //     Image {
            //         Layout.preferredWidth: 24
            //         Layout.preferredHeight: 24
            //         source: "Images/Trahere_logo.png"
            //         fillMode: Image.PreserveAspectFit
            //     }

            //     Text {
            //         text: "Create new document - Krita"
            //         color: "#bbbbbb"
            //         font.pixelSize: 13
            //         Layout.fillWidth: true
            //     }

            //     Button {
            //         text: "✕"
            //         flat: true
            //         Layout.preferredWidth: 30
            //         Layout.preferredHeight: 30
            //         onClicked: createDocWindow.close()

            //         background: Rectangle {
            //             color: parent.hovered ? "#e81123" : "transparent"
            //         }

            //         contentItem: Text {
            //             text: parent.text
            //             color: "white"
            //             horizontalAlignment: Text.AlignHCenter
            //             verticalAlignment: Text.AlignVCenter
            //         }
            //     }
            // }
        }

        // Main Content
        RowLayout {
            anchors.fill: parent
            anchors.topMargin: 40
            spacing: 0

            // Left Sidebar with Templates
            Rectangle {
                Layout.preferredWidth: 180
                Layout.fillHeight: true
                color: "#2b2b2b"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    Repeater {
                        model: [
                            {icon: "📄", text: "Custom Document", selected: true},
                            {icon: "💬", text: "Comic Templates", selected: false},
                            {icon: "🎨", text: "Design Templates", selected: false},
                        ]

                        Button {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40

                            background: Rectangle {
                                color: modelData.selected ? "#4a4a4a" : (parent.hovered ? "#3a3a3a" : "transparent")
                                radius: 4
                            }

                            contentItem: RowLayout {
                                spacing: 8

                                Text {
                                    text: modelData.icon
                                    font.pixelSize: 16
                                }

                                Text {
                                    text: modelData.text
                                    color: "#cccccc"
                                    font.pixelSize: 12
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }
            }

            // Right Content Area
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#3c3f41"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Flickable {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        contentHeight: contentColumn.height
                        clip: true

                        ColumnLayout {
                            id: contentColumn
                            width: parent.width - 32
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.topMargin: 16
                            spacing: 16

                            // Tabs
                            RowLayout {
                                spacing: 8

                                Repeater {
                                    model: ["Dimensions", "Content"]

                                    Button {
                                        text: modelData
                                        flat: true

                                        background: Rectangle {
                                            color: index === 0 ? "#4a4a4a" : "transparent"
                                            border.color: index === 0 ? "#5a5a5a" : "transparent"
                                            border.width: 1
                                            radius: 4
                                        }

                                        contentItem: Text {
                                            text: parent.text
                                            color: "#cccccc"
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            font.pixelSize: 12
                                        }
                                    }
                                }
                            }

                            // Image Size Section
                            GroupBox {
                                Layout.fillWidth: true

                                background: Rectangle {
                                    color: "#2b2b2b"
                                    border.color: "#1a1a1a"
                                    radius: 4
                                }

                                label: Text {
                                    text: "Image Size"
                                    color: "#cccccc"
                                    font.pixelSize: 13
                                    padding: 4
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    spacing: 12

                                    // Predefined
                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: "Predefined:"
                                            color: "#aaaaaa"
                                            font.pixelSize: 12
                                            Layout.preferredWidth: 100
                                        }

                                        ComboBox {
                                            id: predefinedCombo
                                            Layout.fillWidth: true
                                            model: [
                                                "Custom",
                                                "A1 (300 ppi)",
                                                "A1 (600 ppi)",
                                                "A2 (300 ppi)",
                                                "A2 (600 ppi)",
                                                "A3 (300 ppi)",
                                                "A3 (600 ppi)",
                                                "A4 (300 ppi)",
                                                "A4 (600 ppi)",
                                                "A5 (300 ppi)",
                                                "A5 (600 ppi)",
                                                "A6 (300 ppi)",
                                                "A6 (600 ppi)"
                                            ]

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.displayText
                                                color: "#cccccc"
                                                verticalAlignment: Text.AlignVCenter
                                                leftPadding: 8
                                            }

                                            onCurrentTextChanged: {
                                                var size = paperSizes[currentText]
                                                if (size) {
                                                    widthSpinBox.value = size.width
                                                    heightSpinBox.value = size.height
                                                    resolutionSpinBox.value = size.dpi
                                                }
                                            }
                                        }

                                        Button {
                                            text: "▭"
                                            Layout.preferredWidth: 30
                                            Layout.preferredHeight: 30

                                            background: Rectangle {
                                                color: parent.hovered ? "#5a5a5a" : "#4a4a4a"
                                                border.color: "#6a6a6a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.text
                                                color: "#cccccc"
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }

                                            onClicked: {
                                                // Portrait orientation
                                                if (widthSpinBox.value > heightSpinBox.value) {
                                                    var temp = widthSpinBox.value
                                                    widthSpinBox.value = heightSpinBox.value
                                                    heightSpinBox.value = temp
                                                }
                                            }
                                        }

                                        Button {
                                            text: "▯"
                                            Layout.preferredWidth: 30
                                            Layout.preferredHeight: 30

                                            background: Rectangle {
                                                color: parent.hovered ? "#5a5a5a" : "#4a4a4a"
                                                border.color: "#6a6a6a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.text
                                                color: "#cccccc"
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }

                                            onClicked: {
                                                // Landscape orientation
                                                if (widthSpinBox.value < heightSpinBox.value) {
                                                    var temp = widthSpinBox.value
                                                    widthSpinBox.value = heightSpinBox.value
                                                    heightSpinBox.value = temp
                                                }
                                            }
                                        }
                                    }

                                    // Width
                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: "Width:"
                                            color: "#aaaaaa"
                                            font.pixelSize: 12
                                            Layout.preferredWidth: 100
                                        }

                                        SpinBox {
                                            id: widthSpinBox
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 99999
                                            value: 4961
                                            editable: true

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: TextInput {
                                                text: parent.textFromValue(parent.value, parent.locale)
                                                color: "#cccccc"
                                                horizontalAlignment: Qt.AlignLeft
                                                verticalAlignment: Qt.AlignVCenter
                                                leftPadding: 8
                                            }

                                            onValueChanged: {
                                                predefinedCombo.currentIndex = 0
                                            }
                                        }

                                        ComboBox {
                                            id: widthUnitCombo
                                            Layout.preferredWidth: 120
                                            model: ["Pixels (px)", "Inches", "cm"]

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.displayText
                                                color: "#cccccc"
                                                verticalAlignment: Text.AlignVCenter
                                                leftPadding: 8
                                            }
                                        }
                                    }

                                    // Height
                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: "Height:"
                                            color: "#aaaaaa"
                                            font.pixelSize: 12
                                            Layout.preferredWidth: 100
                                        }

                                        SpinBox {
                                            id: heightSpinBox
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 99999
                                            value: 7016
                                            editable: true

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: TextInput {
                                                text: parent.textFromValue(parent.value, parent.locale)
                                                color: "#cccccc"
                                                horizontalAlignment: Qt.AlignLeft
                                                verticalAlignment: Qt.AlignVCenter
                                                leftPadding: 8
                                            }

                                            onValueChanged: {
                                                predefinedCombo.currentIndex = 0
                                            }
                                        }

                                        ComboBox {
                                                id: heightUnitCombo
                                                Layout.preferredWidth: 120
                                                model: ["Pixels (px)", "Inches", "cm"]

                                                background: Rectangle {
                                                    color: "#4a4a4a"
                                                    border.color: "#5a5a5a"
                                                    radius: 3
                                                }

                                                contentItem: Text {
                                                    text: parent.displayText
                                                    color: "#cccccc"
                                                    verticalAlignment: Text.AlignVCenter
                                                    leftPadding: 8
                                                }
                                            }
                                    }

                                    // Resolution
                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: "Resolution:"
                                            color: "#aaaaaa"
                                            font.pixelSize: 12
                                            Layout.preferredWidth: 100
                                        }

                                        SpinBox {
                                            id: resolutionSpinBox
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 9999
                                            value: 600
                                            editable: true

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: TextInput {
                                                text: parent.textFromValue(parent.value, parent.locale)
                                                color: "#cccccc"
                                                horizontalAlignment: Qt.AlignLeft
                                                verticalAlignment: Qt.AlignVCenter
                                                leftPadding: 8
                                            }

                                            onValueChanged: {
                                                predefinedCombo.currentIndex = 0
                                            }
                                        }

                                        ComboBox {
                                            Layout.preferredWidth: 120
                                            model: ["Pixels/Inch", "Pixels/cm"]

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.displayText
                                                color: "#cccccc"
                                                verticalAlignment: Text.AlignVCenter
                                                leftPadding: 8
                                            }
                                        }
                                    }

                                    // Save Image Size
                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: "Save Image Size as:"
                                            color: "#aaaaaa"
                                            font.pixelSize: 12
                                        }

                                        TextField {
                                            Layout.fillWidth: true
                                            placeholderText: ""

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            color: "#cccccc"
                                        }

                                        Button {
                                            text: "Save"

                                            background: Rectangle {
                                                color: parent.hovered ? "#5a5a5a" : "#4a4a4a"
                                                border.color: "#6a6a6a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.text
                                                color: "#cccccc"
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                    }
                                }
                            }

                            // Color Section
                            GroupBox {
                                Layout.fillWidth: true

                                background: Rectangle {
                                    color: "#2b2b2b"
                                    border.color: "#1a1a1a"
                                    radius: 4
                                }

                                label: Text {
                                    text: "Color"
                                    color: "#cccccc"
                                    font.pixelSize: 13
                                    padding: 4
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    spacing: 12

                                    // Model
                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: "Model:"
                                            color: "#aaaaaa"
                                            font.pixelSize: 12
                                            Layout.preferredWidth: 80
                                        }

                                        ComboBox {
                                            Layout.fillWidth: true
                                            model: ["RGB/Alpha", "CMYK", "Grayscale"]

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.displayText
                                                color: "#cccccc"
                                                verticalAlignment: Text.AlignVCenter
                                                leftPadding: 8
                                            }
                                        }
                                    }

                                    // Depth
                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: "Depth:"
                                            color: "#aaaaaa"
                                            font.pixelSize: 12
                                            Layout.preferredWidth: 80
                                        }

                                        ComboBox {
                                            Layout.fillWidth: true
                                            model: ["8-bit integer/channel", "16-bit integer/channel", "32-bit float/channel"]

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.displayText
                                                color: "#cccccc"
                                                verticalAlignment: Text.AlignVCenter
                                                leftPadding: 8
                                            }
                                        }
                                    }

                                    // Profile
                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: "Profile:"
                                            color: "#aaaaaa"
                                            font.pixelSize: 12
                                            Layout.preferredWidth: 80
                                        }

                                        ComboBox {
                                            Layout.fillWidth: true
                                            model: ["sRGB-elle-V2-srgbtrc.icc (Default)"]

                                            background: Rectangle {
                                                color: "#4a4a4a"
                                                border.color: "#5a5a5a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.displayText
                                                color: "#cccccc"
                                                verticalAlignment: Text.AlignVCenter
                                                leftPadding: 8
                                            }
                                        }

                                        Button {
                                            text: "🔄"
                                            Layout.preferredWidth: 30
                                            Layout.preferredHeight: 30

                                            background: Rectangle {
                                                color: parent.hovered ? "#5a5a5a" : "#4a4a4a"
                                                border.color: "#6a6a6a"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.text
                                                color: "#cccccc"
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                    }

                                    // Color Space Browser Button
                                    Button {
                                        Layout.fillWidth: true
                                        text: "Color Space Browser"

                                        background: Rectangle {
                                            color: parent.hovered ? "#5a5a5a" : "#4a4a4a"
                                            border.color: "#6a6a6a"
                                            radius: 3
                                        }

                                        contentItem: Text {
                                            text: parent.text
                                            color: "#cccccc"
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                }
                            }

                            // Info Text
                            Text {
                                Layout.fillWidth: true
                                text: "This document will be " + widthSpinBox.value + " pixels by " + heightSpinBox.value + " pixels in RGB/Alpha (8-bit integer/channel). The pixel size is 32 bit. A single paint layer will use " +
                                      ((widthSpinBox.value * heightSpinBox.value * 4) / (1024 * 1024)).toFixed(1) + " MiB of RAM."
                                color: "#aaaaaa"
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }

                            Item { Layout.preferredHeight: 16 }
                        }
                    }

                    // Footer
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        color: "#3c3f41"

                        RowLayout {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 16
                            spacing: 8

                            Button {
                                text: "Create"

                                background: Rectangle {
                                    color: parent.hovered ? "#3daee9" : "#2980b9"
                                    radius: 3
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: {
                                    // Open a save file dialog to choose .ora destination, then create document + .ora archive.
                                    saveOraDialog.open()
                                }
                            }

                            Button {
                                text: "Cancel"

                                background: Rectangle {
                                    color: parent.hovered ? "#5a5a5a" : "#4a4a4a"
                                    border.color: "#6a6a6a"
                                    radius: 3
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: "#cccccc"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: createDocWindow.close()
                            }
                        }
                    }
                }
            }
        }
    }

    // Save File Dialog for creating .ora
    FileDialog {
        id: saveOraDialog
        title: "Save New .ora File"
        fileMode: FileDialog.SaveFile
        nameFilters: ["OpenRaster (*.ora)", "All files (*)"]
        onAccepted: {
            // Compute pixel dimensions based on selected units prior to closing create window.
            function toPixels(value, unit) {
                var dpi = resolutionSpinBox.value
                if (!unit) return Math.round(value)
                if (unit.indexOf("Pixels") === 0) return Math.round(value)
                if (unit.indexOf("Inches") !== -1) return Math.round(value * dpi)
                if (unit.indexOf("cm") !== -1) return Math.round((value / 2.54) * dpi)
                return Math.round(value)
            }
            var pixelWidth = toPixels(widthSpinBox.value, widthUnitCombo.currentText)
            var pixelHeight = toPixels(heightSpinBox.value, heightUnitCombo.currentText)

            // Log and pass QML url directly; C++ overload accepts QUrl and will normalize/append .ora
            console.log("Save .ora to:", String(saveOraDialog.selectedFile))

            var helper = oraCreator
            if (helper.createOra(saveOraDialog.selectedFile, pixelWidth, pixelHeight)) {
                console.log("Created .ora file")
                createDocWindow.close()
                var comp = Qt.createComponent("CanvasWindow.qml")
                if (comp.status === Component.Ready) {
                    var createdUrl = String(saveOraDialog.selectedFile)
                    var localCreatedPath = createdUrl.startsWith("file:///") ? createdUrl.substring(8) : createdUrl
                    var win = comp.createObject(window, { initialWidth: pixelWidth, initialHeight: pixelHeight, lastOraPath: localCreatedPath })
                    if (!win) console.log("Failed to create CanvasWindow:", comp.errorString())
                } else {
                    console.log("Canvas component not ready:", comp.status, comp.errorString())
                }
            } else {
                console.log("Failed to create .ora file")
            }
        }
        onRejected: {
            console.log("Save .ora canceled")
        }
    }

    // Singleton instance for OraCreator
    OraCreator { id: oraCreator }
    OraLoader { id: oraLoader }
}
