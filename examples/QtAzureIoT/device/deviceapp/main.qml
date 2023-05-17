/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

Window {
    visible: true
    width: 800
    height: 480
    flags: Qt.FramelessWindowHint
    color: "black"
    GridLayout {
        anchors.fill: parent
        columns: 2
        Text {
            Layout.alignment: Qt.AlignCenter
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            font { bold: true; pointSize: 32 }
            color: "white"
            text: sensor.temperature.toFixed(2) + " deg.C."
        }
        Text {
            Layout.alignment: Qt.AlignCenter
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            font { bold: true; pointSize: 32 }
            color: "white"
            text: card.cardInReader ? "CARD DETECTED" : "No card";
        }
        Text {
            Layout.alignment: Qt.AlignCenter
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            font { bold: true; pointSize: 32 }
            color: "white"
            text: sensor.pressure.toFixed(2) + " hPa."
        }
        Text {
            Layout.alignment: Qt.AlignCenter
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            font { bold: true; pointSize: 32 }
            color: "white"
            text: sensor.humidity.toFixed(2) + " %"
        }
    }
    Button {
        text: "EXIT"
        onClicked: Qt.exit(0)
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
