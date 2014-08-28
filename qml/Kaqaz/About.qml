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

import QtQuick 2.2

Item {
    id: about_page

    Header {
        id: title
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
    }

    Column {
        spacing: 2
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        Image{
            id: kaqaz_logo
            anchors.horizontalCenter: parent.horizontalCenter
            width: 192*physicalPlatformScale
            height: width
            smooth: true
            source: "files/kaqaz.png"
        }

        Text{
            id: about_name_text
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: devices.isMobile? 13*fontsScale : 15*fontsScale
            font.family: globalFontFamily
            color: "#333333"
        }

        Text{
            id: about_license_text
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: devices.isMobile? 11*fontsScale : 13*fontsScale
            font.family: globalFontFamily
            color: "#333333"

            MouseArea{
                anchors.fill: parent
                onClicked: Qt.openUrlExternally("http://www.gnu.org/licenses/gpl-3.0.en.html")
            }
        }

        Text{
            id: about_framework_text
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: devices.isMobile? 11*fontsScale : 13*fontsScale
            font.family: globalFontFamily
            color: "#333333"
        }

        Text{
            id: about_powered_text
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: devices.isMobile? 11*fontsScale : 13*fontsScale
            font.family: globalFontFamily
            color: "#333333"

            MouseArea{
                anchors.fill: parent
                onClicked: Qt.openUrlExternally("http://labs.sialan.org/")
            }
        }
    }

    Button {
        id: about_home
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.leftMargin: 40*physicalPlatformScale
        anchors.rightMargin: 40*physicalPlatformScale
        anchors.bottomMargin: 20*physicalPlatformScale
        height: 42*physicalPlatformScale
        normalColor: "#4098bf"
        highlightColor: "#337fa2"
        onClicked: Qt.openUrlExternally("http://kaqaz.org/")
    }

    Connections{
        target: kaqaz
        onLanguageChanged: initTranslations()
    }

    function initTranslations(){
        about_name_text.text      = qsTr("Kaqaz") + " " + kaqaz.version()
        about_license_text.text   = qsTr("License: GNU GPL Version 3")
        about_powered_text.text   = qsTr("(C) 2014 Sialan Labs")
        about_framework_text.text = qsTr("Based on") + " Qt" + kaqaz.qtVersion()
        about_home.text           = qsTr("Home Page")
        title.text                = qsTr("About")
    }

    Component.onCompleted: {
        initTranslations()
    }
}
