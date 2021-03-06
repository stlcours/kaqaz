/*
    Copyright (C) 2014 Sialan Labs
    http://labs.sialan.org

    Kaqaz is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Kaqaz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import Kaqaz 1.0
import SialanTools 1.0

Item {
    id: edit_dialog
    anchors.fill: parent
    anchors.topMargin: View.statusBarHeight
    anchors.bottomMargin: View.navigationBarHeight

    property variant item
    property variant paperItem: item? item.paperItem : 0

    property variant coo: edit_dialog.item? database.paperLocation(edit_dialog.item.paperItem.paperItem) : 0
    property real longitude: coo? coo.longitude : 0
    property real latitude: coo? coo.latitude : 0

    Connections {
        target: database
        onPaperChanged: {
            if( !item )
                return
            if( id != item.paperItem.paperItem )
                return

            var tmp = item
            item = 0
            item = tmp
        }
    }

    MouseArea {
        anchors.fill: parent
    }

    Header {
        id: header
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        height: Devices.isAndroid? 0 : 60*physicalPlatformScale
    }

    Flickable {
        id: edit_flick
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.topMargin: 20*physicalPlatformScale
        anchors.bottom: parent.bottom
        contentHeight: column.height
        contentWidth: width
        bottomMargin: 20*physicalPlatformScale
        clip: true

        Column {
            id: column
            width: edit_flick.width

            Text {
                id: title
                x: 40*physicalPlatformScale
                y: 60*physicalPlatformScale
                width: parent.width-2*x
                text: paperItem && paperItem.text.length!=0? paperItem.text: qsTr("Untitled Paper")
                font.pixelSize: Devices.isMobile? 22*fontsScale : 25*fontsScale
                font.weight: Font.Light
                font.family: SApp.globalFontFamily
                color: "#333333"
                elide: Text.ElideRight
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
            }

            Text {
                id: body
                x: 50*physicalPlatformScale
                width: parent.width-2*x
                text: paperItem? paperItem.bodyText : ""
                font.pixelSize: Devices.isMobile? 9*fontsScale : 10*fontsScale
                font.weight: Font.Light
                font.family: SApp.globalFontFamily
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                maximumLineCount: 2
                elide: Text.ElideRight
                color: "#666666"
            }

            Grid {
                id: button_box
                anchors.horizontalCenter: parent.horizontalCenter
                columns: Math.floor((parent.width - 20*physicalPlatformScale)/(92*physicalPlatformScale))

                PaperTypeButton {
                    paperObject: edit_dialog.item? edit_dialog.item.paperItem : 0
                    paperItem: edit_dialog.item? edit_dialog.item.paperItem.paperItem : 0
                }
                PaperCalendarButton {
                    paperObject: edit_dialog.item? edit_dialog.item.paperItem : 0
                    paperItem: edit_dialog.item? edit_dialog.item.paperItem.paperItem : 0
                }
                PaperClockButton {
                    paperObject: edit_dialog.item? edit_dialog.item.paperItem : 0
                    paperItem: edit_dialog.item? edit_dialog.item.paperItem.paperItem : 0
                }
                PaperWeatherButton {
                    paperObject: edit_dialog.item? edit_dialog.item.paperItem : 0
                    paperItem: edit_dialog.item? edit_dialog.item.paperItem.paperItem : 0
                    visible: kaqaz.proBuild
                }
                PaperMapButton {
                    paperObject: edit_dialog.item? edit_dialog.item.paperItem : 0
                    paperItem: edit_dialog.item? edit_dialog.item.paperItem.paperItem : 0
                }
                PaperTemperatureButton {
                    paperObject: edit_dialog.item? edit_dialog.item.paperItem : 0
                    paperItem: edit_dialog.item? edit_dialog.item.paperItem.paperItem : 0
                    visible: kaqaz.proBuild
                }
            }

            PaperShareButton {
                anchors.horizontalCenter: parent.horizontalCenter
                width: button_box.width
                height: 52*physicalPlatformScale
                paperObject: edit_dialog.item? edit_dialog.item.paperItem : 0
                paperItem: edit_dialog.item? edit_dialog.item.paperItem.paperItem : 0
            }

            PaperMapView {
                id: map_image
                anchors.horizontalCenter: parent.horizontalCenter
                width: button_box.width
                height: 3*width/4
                latitude: edit_dialog.latitude
                longitude: edit_dialog.longitude
                visible: !unknown
            }

            PaperDeleteButton {
                anchors.horizontalCenter: parent.horizontalCenter
                width: button_box.width
                height: 52*physicalPlatformScale
                paperObject: edit_dialog.item? edit_dialog.item.paperItem : 0
                paperItem: edit_dialog.item? edit_dialog.item.paperItem.paperItem : 0
            }
        }
    }

    ScrollBar {
        scrollArea: edit_flick; height: edit_flick.height; width: 6*physicalPlatformScale
        anchors.right: parent.right; anchors.top: edit_flick.top; color: "#000000"
        anchors.rightMargin: 4*physicalPlatformScale
    }

    MouseArea {
        id: back_marea
        anchors.fill: parent
        visible: bottomPanel.item? true : false
        onClicked: hideBottomPanel()
    }
}
