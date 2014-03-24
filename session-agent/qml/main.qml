import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import org.land 1.0

Item {
    id: mainWindow

    SystemPalette {
        id: sysPallete
    }

    Rectangle {
        id: background
        anchors.fill: parent
        anchors.margins: 4
        color: sysPallete.window
//        opacity: 0.5
        radius: 10
    }

    AvailableConnectionsModel {
        id: availableConnectionsModel
        device: deviceUni
        onIconChanged: Device.setIcon(icon)
        onToolTipChanged: Device.setToolTip(toolTip)
    }

    ColumnLayout {
        anchors.fill: background
        anchors.margins: 4

        RowLayout {
            anchors.fill: parent
            id: deviceLayout
            Label {
                Layout.fillWidth: true
                text: "WiFi"
            }

            Switch {
                id: wifiSwitch
                checked: Manager.wirelessEnabled
                onCheckedChanged: {
                    if (Manager.wirelessEnabled !== checked) {
                        Manager.wirelessEnabled = checked
                    }
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                anchors.fill: parent
                model: AvailableConnectionsSortModel {
                    sourceModel: availableConnectionsModel
                }
                delegate: Connection { }
            }
        }
    }

    Connections {
        target: Manager
        onWirelessEnabledChanged: {
            wifiSwitch.checked = Manager.wirelessEnabled
        }
    }
}
