/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

RoundButton {
    id: watchButton
    property string buttonText
    property bool split
    property string color
    width: 70; height: 70; radius: 25
    palette.button: color
    Text {
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        font.bold: true
        text: watchButton.buttonText
    }
    Rectangle {
        visible: watchButton.split
        color: "black"
        width: 55; height: 1
        anchors.centerIn: parent
    }
}
