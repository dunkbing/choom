import QtQuick 2.15

Item {
    id: root

    width: 16
    height: 16

    property color color: "#007acc"
    property int segmentCount: 12
    property int animationDuration: 720
    property int stepDuration: Math.max(16, Math.round(animationDuration / segmentCount))
    property real minOpacity: 0.15
    property real opacityDecay: 0.65
    property int currentIndex: 0

    Timer {
        id: spinnerTimer
        interval: root.stepDuration
        running: true
        repeat: true
        onTriggered: root.currentIndex = (root.currentIndex + 1) % root.segmentCount
    }

    Repeater {
        model: root.segmentCount

        delegate: Rectangle {
            width: root.width * 0.16
            height: root.height * 0.48
            radius: width / 2
            color: root.color
            antialiasing: true

            readonly property int distance: (root.segmentCount + index - root.currentIndex) % root.segmentCount
            opacity: Math.max(root.minOpacity, Math.pow(root.opacityDecay, distance))

            x: root.width / 2 - width / 2
            y: root.height / 2 - height

            transform: [
                Rotation {
                    origin.x: width / 2
                    origin.y: height
                    angle: index * 360 / root.segmentCount
                }
            ]

            Behavior on opacity {
                NumberAnimation {
                    duration: root.stepDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
