import QtQuick
import QtQuick.Controls
import QtWebEngine
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Item {
    id: root

    // Properties
    property string url: "https://google.com"
    property alias webView: webEngineView

    // Signals with unique names to avoid conflicts
    signal webUrlChanged(string url)
    signal webTitleChanged(string title)
    signal webIconChanged(var icon)

    Rectangle {
        id: container
        anchors.fill: parent
        color: "#24262e"
        radius: 8

        layer.enabled: true
        layer.effect: OpacityMask {
            id: opacityMaskInstance
            maskSource: Rectangle {
                id: maskedRect
                width: container.width
                height: container.height
                radius: container.radius
            }
        }

        // Clip content to rounded corners
        clip: true

        WebEngineView {
            id: webEngineView
            anchors.fill: parent

            // Handle signals
            onUrlChanged: {
                root.url = url.toString();
                root.webUrlChanged(url.toString());
            }

            onTitleChanged: {
                root.webTitleChanged(title);
            }

            onIconChanged: {
                root.webIconChanged(icon);
            }

            // Match Qt default settings from C++ code
            settings.playbackRequiresUserGesture: false
            settings.webGLEnabled: true
            settings.accelerated2dCanvasEnabled: true
            settings.scrollAnimatorEnabled: true
        }
    }

    function load(url) {
        webEngineView.url = url;
    }

    function back() {
        webEngineView.goBack();
    }

    function forward() {
        webEngineView.goForward();
    }

    function reload() {
        webEngineView.reload();
    }

    function stop() {
        webEngineView.stop();
    }
}
