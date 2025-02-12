// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Dialog {
    id: aboutDialog
    width: 500
    height: 150
    title: "WebAssembly Dialog box"
    modal: true
    Rectangle {
        width: parent.width * 0.8
        height: parent.height * 0.4
        color: "lightgray"
        anchors.fill: parent
        ColumnLayout {
            spacing: 2
            anchors.fill: parent

            Label {
                id: labelInfo
                Layout.alignment: Qt.AlignCenter
                text: "Accessibility Demo sample application developed in QML."
                Accessible.role: Accessible.StaticText
                horizontalAlignment: Text.AlignHCenter
                Accessible.name: text
                Accessible.description: "Purpose of this application."
                wrapMode: Text.WordWrap
            }

            Button {
                id: closeButton
                text: "Close"
                Layout.alignment: Qt.AlignCenter
                Accessible.role: Accessible.Button
                Accessible.name: text
                Accessible.description: "To close the About Dialog box."
                onClicked: {
                    aboutDialog.close()
                }
            }
        }
    }
}
