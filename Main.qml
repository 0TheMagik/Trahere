import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1920
    height: 1080
    title: "Trahere"
    color: "#31363b"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Main Content Area
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
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

            // Main Canvas Area
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#eff0f1"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 32
                    spacing: 16

                    // Title
                    Text {
                        text: "Your Project"
                        font.pixelSize: 48
                        font.family: "Brush Script MT, cursive"
                        color: "#31363b"
                    }

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

                    // Canvas Area
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "white"
                        radius: 8
                        border.color: "#bdc3c7"
                        border.width: 1
                    }
                }
            }
        }
    }
}
