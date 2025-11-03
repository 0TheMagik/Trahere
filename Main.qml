import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 1280
    height: 768
    visible: true
    title: qsTr("Trahere")
    color: "#31363b"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Top Menu Bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            color: "#31363b"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 16

                Repeater {
                    model: ["File", "Edit", "View", "Image", "Layer", "Select", "Filter", "Tools", "Settings", "Window", "Help"]
                    Button {
                        text: modelData
                        flat: true
                        contentItem: Text {
                            text: parent.text
                            color: "#eff0f1"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.hovered ? "#3daee9" : "transparent"
                        }
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }

        // Top Toolbar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: "#3c4248"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 4

                // Tool icons
                Repeater {
                    model: ["📁", "💾", "↶", "↷", "✂️", "📋", "🎨"]
                    Button {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        text: modelData
                        background: Rectangle {
                            color: parent.hovered ? "#4a5158" : "transparent"
                            radius: 4
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Brush settings
                Label {
                    text: "Opacity: 100%"
                    color: "#eff0f1"
                }
                Label {
                    text: "Size: 40.00 px"
                    color: "#eff0f1"
                    Layout.leftMargin: 16
                }
            }
        }

        // Main Content Area
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Left Toolbar
            Rectangle {
                Layout.preferredWidth: 48
                Layout.fillHeight: true
                color: "#31363b"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.topMargin: 8
                    spacing: 4

                    Repeater {
                        model: ["🖌️", "✏️", "📐", "⭕", "▭", "🔍", "✋", "🎨", "💧", "📊", "⚙️"]
                        Button {
                            Layout.preferredWidth: 40
                            Layout.preferredHeight: 40
                            Layout.alignment: Qt.AlignHCenter
                            text: modelData
                            background: Rectangle {
                                color: parent.hovered ? "#3daee9" : "#31363b"
                                radius: 4
                                border.color: parent.hovered ? "#3daee9" : "#545b64"
                                border.width: 1
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#eff0f1"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }
            }

            // Central Content
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#232629"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 40

                    // Start Section
                    ColumnLayout {
                        Layout.preferredWidth: 300
                        spacing: 16

                        Label {
                            text: "Start"
                            font.pixelSize: 24
                            color: "#eff0f1"
                        }

                        Button {
                            text: "📄 New File (Ctrl+N)"
                            Layout.fillWidth: true
                            flat: true
                            contentItem: Text {
                                text: parent.text
                                color: "#eff0f1"
                                horizontalAlignment: Text.AlignLeft
                            }
                            background: Rectangle {
                                color: parent.hovered ? "#31363b" : "transparent"
                            }
                        }

                        Button {
                            text: "📂 Open File (Ctrl+O)"
                            Layout.fillWidth: true
                            flat: true
                            contentItem: Text {
                                text: parent.text
                                color: "#eff0f1"
                                horizontalAlignment: Text.AlignLeft
                            }
                            background: Rectangle {
                                color: parent.hovered ? "#31363b" : "transparent"
                            }
                        }

                        Label {
                            text: "Recent Documents"
                            font.pixelSize: 20
                            color: "#eff0f1"
                            Layout.topMargin: 16
                        }

                        Button {
                            text: "Clear"
                            flat: true
                            contentItem: Text {
                                text: parent.text
                                color: "#3daee9"
                                font.underline: parent.hovered
                            }
                            background: Rectangle {
                                color: "transparent"
                            }
                        }

                        // Recent documents list
                        ColumnLayout {
                            spacing: 8
                            Repeater {
                                model: ["demo-apple.kra", "Krita-proect.kra", "clonelayer-transformmasks-azalea"]
                                RowLayout {
                                    Rectangle {
                                        Layout.preferredWidth: 48
                                        Layout.preferredHeight: 48
                                        color: "#31363b"
                                        radius: 4
                                    }
                                    Label {
                                        text: modelData
                                        color: "#eff0f1"
                                    }
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }

                    // Community Section
                    ColumnLayout {
                        Layout.preferredWidth: 250
                        spacing: 16

                        Label {
                            text: "Community"
                            font.pixelSize: 24
                            color: "#eff0f1"
                        }

                        Repeater {
                            model: [
                                "📖 User Manual",
                                "📖 Getting Started",
                                "📖 User Community",
                                "📖 Krita Website",
                                "📖 Source Code",
                                "❤️ Support Krita",
                                "⚙️ Powered by KDE"
                            ]
                            Button {
                                text: modelData
                                Layout.fillWidth: true
                                flat: true
                                contentItem: Text {
                                    text: parent.text
                                    color: "#eff0f1"
                                    horizontalAlignment: Text.AlignLeft
                                }
                                background: Rectangle {
                                    color: parent.hovered ? "#31363b" : "transparent"
                                }
                            }
                        }

                        // Drag and drop area
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 150
                            Layout.topMargin: 20
                            color: "transparent"
                            border.color: "#545b64"
                            border.width: 2
                            radius: 4

                            Label {
                                anchors.centerIn: parent
                                text: "Drag an image into the\nwindow to open"
                                color: "#eff0f1"
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }

                    // News Section
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        RowLayout {
                            Label {
                                text: "News"
                                font.pixelSize: 24
                                color: "#eff0f1"
                            }
                            Item { Layout.fillWidth: true }
                            Button {
                                text: "☰"
                                flat: true
                                contentItem: Text {
                                    text: parent.text
                                    color: "#eff0f1"
                                    font.pixelSize: 18
                                }
                            }
                        }

                        Label {
                            text: "You can enable news from krita.org\nin various languages with the menu\nabove"
                            color: "#7f8c8d"
                            Layout.fillWidth: true
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                // Bottom info text
                Label {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottomMargin: 16
                    text: "Krita is an open source and community-driven tool for digital artists everywhere.\nProgress is made possible thanks to ongoing support from our community of contributors, sponsors, and development fund members."
                    color: "#7f8c8d"
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 11
                }
            }

            // Right Sidebar
            Rectangle {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                color: "#31363b"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Advanced Color Selector Panel
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        color: "#232629"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8

                            Label {
                                text: "Advanced Color Selector"
                                color: "#eff0f1"
                                font.bold: true
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#545b64"
                            }
                        }
                    }

                    // Layers Panel
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 250
                        color: "#232629"
                        Layout.topMargin: 4

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8

                            Label {
                                text: "Layers"
                                color: "#eff0f1"
                                font.bold: true
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                color: "#31363b"
                                Label {
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    text: "Normal"
                                    color: "#eff0f1"
                                }
                            }

                            Label {
                                text: "Opacity:  100%"
                                color: "#eff0f1"
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }

                    // Brush Presets Panel
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#232629"
                        Layout.topMargin: 4

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "Brush Presets"
                                    color: "#eff0f1"
                                    font.bold: true
                                }
                                Item { Layout.fillWidth: true }
                                Button {
                                    text: "🏷️ Tag"
                                    flat: true
                                    contentItem: Text {
                                        text: parent.text
                                        color: "#eff0f1"
                                        font.pixelSize: 10
                                    }
                                }
                            }

                            // Brush thumbnails grid
                            GridLayout {
                                Layout.fillWidth: true
                                columns: 3
                                rowSpacing: 4
                                columnSpacing: 4

                                Repeater {
                                    model: 12
                                    Rectangle {
                                        Layout.preferredWidth: 50
                                        Layout.preferredHeight: 50
                                        color: "#545b64"
                                        border.color: index === 0 ? "#3daee9" : "#31363b"
                                        border.width: 2
                                    }
                                }
                            }

                            TextField {
                                Layout.fillWidth: true
                                placeholderText: "Search"
                                color: "#eff0f1"
                                background: Rectangle {
                                    color: "#31363b"
                                    border.color: "#545b64"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
