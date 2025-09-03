import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick.Controls 2.15

Rectangle {
    width: 800
    height: 600
    anchors.fill: parent

    Map {
        id: map
        objectName: "map"
        anchors.fill: parent
        activeMapType: map.supportedMapTypes[1]
        center: QtPositioning.coordinate(61.27136, 73.33983) // Сургут
        zoomLevel: 2
        minimumZoomLevel: 2
        maximumZoomLevel: 20

        plugin: Plugin {
            name: "osm";
            allowExperimental: true

            PluginParameter {
                name: "osm.mapping.providersrepository.disabled";
                value: true
            }

            PluginParameter {
                name: "osm.mapping.offline.directory";
                value: ":/offline_tiles/"
            }
        }

        property var markerMap: ({})

        function addMarker(id, lat, lon, label, iconSource) {
            var component = Qt.createComponent("qrc:/MapMarker.qml");
            if (component.status === Component.Ready) {
                var marker = component.createObject(map, {
                  idStr: id,
                  coordinate: QtPositioning.coordinate(lat, lon),
                  label: label,
                  iconSource: iconSource
                 });
                markerMap[id] = marker;
                map.addMapItem(marker);
            }
        }

        function removeMarker(id) {
            if (markerMap[id]) {
                map.removeMapItem(markerMap[id]);
                markerMap[id].destroy();
                delete markerMap[id];
            }
        }

        function clearMarkers() {
            markerMap = []
            map.clearMapItems()
        }

        function haversineDistance(coord1, coord2) {
            const R = 6371e3; // Радиус Земли в метрах
            const lat1 = coord1.latitude * Math.PI / 180;
            const lat2 = coord2.latitude * Math.PI / 180;
            const deltaLat = (coord2.latitude - coord1.latitude) * Math.PI / 180;
            const deltaLon = (coord2.longitude - coord1.longitude) * Math.PI / 180;

            const a = Math.sin(deltaLat / 2) * Math.sin(deltaLat / 2) +
                      Math.cos(lat1) * Math.cos(lat2) *
                      Math.sin(deltaLon / 2) * Math.sin(deltaLon / 2);
            const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
            const distance = R * c; // расстояние в метрах
            const distanceInKilometers = distance / 1000; // перевод в километры

            const azimuth = calculateAzimuth(coord1.latitude, coord1.longitude, coord2.latitude, coord2.longitude);
            mapController.receiveAzimuth(azimuth);
            mapController.receiveCallLatLon(coord2);
            return distanceInKilometers;
        }

        function calculateTotalDistance(path) {
            let totalDistance = 0;
            for (let i = 0; i < path.length - 1; i++) {
                totalDistance += haversineDistance(path[i], path[i + 1]);
            }
            return totalDistance; // Возвращается сумма расстояний в метрах
        }

        function addLine(path, color, width) {
            var polyline = Qt.createQmlObject('import QtLocation 5.15; MapPolyline {}', map);
            polyline.path = [];
            polyline.path = path;
            polyline.line.color = color;
            polyline.line.width = width;
            map.addMapItem(polyline);
            const totalDistance = Math.round(calculateTotalDistance(path));
            mapController.receiveDistance(totalDistance);
        }

        function calculateAzimuth(lat1, lon1, lat2, lon2) {
            var toRadians = Math.PI / 180;
            var toDegrees = 180 / Math.PI;
            var f1 = lat1 * toRadians;
            var f2 = lat2 * toRadians;
            var deltaLambda = (lon2 - lon1) * toRadians;
            var y = Math.sin(deltaLambda) * Math.cos(f2);
            var x = Math.cos(f1) * Math.sin(f2) - Math.sin(f1) * Math.cos(f2) * Math.cos(deltaLambda);
            var theta = Math.atan2(y, x);
            var azimuth = (theta * toDegrees + 360) % 360;
            return azimuth;
        }
    }
}
