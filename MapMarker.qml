import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick.Controls 2.15

MapQuickItem {
    id: marker
    property string idStr: ""
    property string label: ""
    property string iconSource: ""

    coordinate: QtPositioning.coordinate(0, 0)
    anchorPoint.x: icon.width / 2
    anchorPoint.y: icon.height

    sourceItem:Column {
        spacing: 2
        Image {
            id: icon
            source: marker.iconSource
            width: 12
            height: 24
        }
        Text {
            text: marker.label
            font.pixelSize: 12
            color: "blue"
        }
    }
}

