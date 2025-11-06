import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

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

        // Search Bar
        Rectangle {
            width: 320
            height: 44
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
                    Layout.fillWidth: true
                    placeholderText: "Search..."
                    color: "#2c3e50"
                    font.pixelSize: 14

                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }
    }

    // File Dialog for Opening Files
    FileDialog {
        id: fileDialog
        title: "Open File - Trahere"
        nameFilters: ["Image files (*.png *.jpg *.jpeg *.bmp *.kra)", "All files (*)"]
        onAccepted: {
            console.log("Selected file:", fileDialog.selectedFile)
            loadedImage.source = fileDialog.selectedFile
        }
        onRejected: {
            console.log("File selection canceled")
        }
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
        modality: Qt.ApplicationModal
        flags: Qt.Dialog
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

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 8

                Image {
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    source: "Images/Trahere_logo.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: "Create new document - Krita"
                    color: "#bbbbbb"
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }

                Button {
                    text: "✕"
                    flat: true
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    onClicked: createDocWindow.close()

                    background: Rectangle {
                        color: parent.hovered ? "#e81123" : "transparent"
                    }

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
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
                                    // Helper: convert given value+unit to pixels using resolutionSpinBox (dpi)
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

                                    createDocWindow.close()

                                    var comp = Qt.createComponent("CanvasWindow.qml")
                                    if (comp.status === Component.Ready) {
                                        var win = comp.createObject(window, { initialWidth: pixelWidth, initialHeight: pixelHeight })
                                        if (!win) console.log("Failed to create CanvasWindow:", comp.errorString())
                                    } else {
                                        console.log("Canvas component not ready:", comp.status, comp.errorString())
                                    }
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
}
