import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

Item {
    width: ListView.view.width
    height: gridLayout.height + 8
    property int rowCount: ListView.view.count

    GridLayout {
        id: gridLayout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 4
        anchors.verticalCenter: parent.verticalCenter
        columns: 2

        Image {
            id: iconStrength
            width: height
            height: textColumn.height
            sourceSize.height: height
            sourceSize.width: height
            cache: true
            source: "image://icon/" + signalStrengthIcon

            Image {
                id: iconSecurity
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: height
                height: parent.height / 2
                sourceSize.height: height
                sourceSize.width: height
                cache: true
                source: security ? "image://icon/emblem-locked" : ""
            }
        }

        ColumnLayout {
            id: textColumn
            Layout.fillWidth: true
            spacing: 0
            Label {
                Layout.fillWidth: true
                id: name
                verticalAlignment: Text.AlignBottom
                elide: Text.ElideRight
                text: "<strong>" + ssid + "</strong>"
            }
            Label {
                Layout.fillWidth: true
                id: statusL
                verticalAlignment: Text.AlignTop
                elide: Text.ElideRight
                text: model.active ? model.status :
                               security ? "Protected by " + securityType : qsTr("Open")
            }
        }

        Rectangle {
            Layout.columnSpan: gridLayout.columns
            Layout.fillWidth: true
            height: 1
            opacity: 0.5
            visible: rowCount !== index + 1
        }
//        Component.onCompleted: {
//            console.debug("network " + networkId + " " + connectionPath + " signalStrength: " + signalStrength)
//        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (connectionPath) {
                Device.activateConnection(connectionPath)
            } else {
                console.debug("TODO add")
            }
        }
    }
}
