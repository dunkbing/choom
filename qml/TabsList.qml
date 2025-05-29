import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: tabsListContainer

    property alias model: tabsModel

    signal tabClicked(int index)
    signal tabCloseClicked(int index)

    color: "#24262e"
    
    // Ensure we have a minimum size
    width: parent ? parent.width : 220
    height: parent ? parent.height : 200

    Component.onCompleted: {
        console.log("TabsList created, size:", width, "x", height)
    }

    ListView {
        id: tabsList
        anchors.fill: parent
        anchors.bottomMargin: 20
        spacing: 2
        clip: true
        model: tabsModel
        
        delegate: Rectangle {
            width: tabsList.width
            height: 32
            color: (model.isSelected || false) ? "#454750" : "#36383e"
            radius: 5
            border.width: 1
            border.color: "#666666"

            Text {
                anchors.centerIn: parent
                text: (model.title || "New Tab") + " [" + (model.index || 0) + "]"
                color: "#ffffff"
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: tabsListContainer.tabClicked(model.index || 0)
            }
            
            Component.onCompleted: {
                console.log("Simple delegate created:", model.title, "at index:", model.index)
            }
        }
    }

    ListModel {
        id: tabsModel

        onCountChanged: {
            console.log("TabsModel count changed to:", count)
        }
    }

    // Add a simple text to verify the container is visible
    Text {
        id: debugText
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 5
        text: "Tabs: " + tabsModel.count
        color: "#ffffff"
        font.pixelSize: 10
        z: 1000
    }

    function addTab(title, iconUrl, isSelected) {
        console.log("QML addTab called with:", title, iconUrl, isSelected)
        tabsModel.append({
            "title": title,
            "iconUrl": iconUrl,
            "isSelected": isSelected
        })
        console.log("TabsModel count after append:", tabsModel.count)
    }

    function removeTab(index) {
        console.log("QML removeTab called with index:", index)
        if (index >= 0 && index < tabsModel.count) {
            tabsModel.remove(index)
        }
    }

    function updateTab(index, title, iconUrl, isSelected) {
        console.log("QML updateTab called with:", index, title, iconUrl, isSelected)
        if (index >= 0 && index < tabsModel.count) {
            tabsModel.setProperty(index, "title", title)
            tabsModel.setProperty(index, "iconUrl", iconUrl)
            tabsModel.setProperty(index, "isSelected", isSelected)
        }
    }

    function clearTabs() {
        console.log("QML clearTabs called")
        tabsModel.clear()
        console.log("TabsModel count after clear:", tabsModel.count)
    }

    function setTabSelected(index, selected) {
        console.log("QML setTabSelected called with:", index, selected)
        for (var i = 0; i < tabsModel.count; i++) {
            tabsModel.setProperty(i, "isSelected", i === index && selected)
        }
    }
}
