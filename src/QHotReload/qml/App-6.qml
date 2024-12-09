import QtQuick
import QtQuick.Controls

ApplicationWindow {
    x: 0
    y: 0
    width: 1
    height: 1
    visible: true
    flags: Qt.WindowStaysOnTopHint
    title: Qt.application.name + ' ' + Qt.application.version

    Loader { id: loader }

    Connections {
        target: fileWatchListener
        function onFileChanged(filename) {
            const path = "file:///" + filename + "?t=" + Date.now()
            loader.source = path
            console.log("[QHotReload] " + path)
        }
    }
}
