import QtQuick 2.15

Item {
    id: root

    property color color: "#007acc"

    Repeater {
        model: 8

        Rectangle {
            width: root.width * 0.15
            height: root.height * 0.3
            radius: width / 2
            color: root.color
            opacity: 1.0 - (index * 0.12)

            x: root.width / 2 - width / 2
            y: root.height / 2 - height / 2

            transform: [
                Rotation {
                    origin.x: root.width / 2
                    origin.y: root.height / 2
                    angle: index * 45 + root.rotation
                }
            ]
        }
    }

    NumberAnimation on rotation {
        from: 0
        to: 360
        duration: 800
        loops: Animation.Infinite
        running: root.visible
    }
}
