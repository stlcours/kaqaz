import QtQuick 2.2

Item {
    id: color_list
    clip: true

    property color currentColor: "dodgerblue"

    GridView {
        id: color_grid
        anchors.fill: parent
        cellWidth: width/(cellCount<3? 3 : cellCount)
        cellHeight: cellWidth - 15*physicalPlatformScale

        property int cellCount: Math.floor(width/(100*physicalPlatformScale))

        model: ListModel {}
        delegate: Item {
            id: item
            width: color_grid.cellWidth
            height: width

            Rectangle{
                id: rectangle
                anchors.fill: parent
                anchors.margins: 15*physicalPlatformScale
                radius: width/2
                border.color: "#ffffff"
                border.width: 3*physicalPlatformScale
                color: itemColor
            }

            MouseArea{
                id: mousearea
                anchors.fill: parent
                onClicked: {
                    color_list.currentColor = itemColor
                }
            }
        }

        Component.onCompleted: {
            model.clear()

            model.append({"itemColor": "darkgray"})
            model.append({"itemColor": "dodgerblue"})
            model.append({"itemColor": "darkgreen"})
            model.append({"itemColor": "gold"})

            model.append({"itemColor": "maroon"})
            model.append({"itemColor": "purple"})
            model.append({"itemColor": "orangered"})
            model.append({"itemColor": "magenta"})

            model.append({"itemColor": "darkslateblue"})
            model.append({"itemColor": "violet"})
            model.append({"itemColor": "saddlebrown"})
            model.append({"itemColor": "black"})

            model.append({"itemColor": "chocolate"})
            model.append({"itemColor": "firebrick"})
            model.append({"itemColor": "teal"})
            model.append({"itemColor": "darkviolet"})

            model.append({"itemColor": "olive"})
            model.append({"itemColor": "mediumvioletred"})
            model.append({"itemColor": "darkorange"})
            model.append({"itemColor": "darkslategray"})
        }
    }

    ScrollBar {
        scrollArea: color_grid; height: color_grid.height; width: 8
        anchors.right: color_grid.right; anchors.top: color_grid.top; color: "#000000"
    }
}