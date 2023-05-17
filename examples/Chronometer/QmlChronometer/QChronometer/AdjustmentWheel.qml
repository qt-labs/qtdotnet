/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

//////////////////////////////////////////////////////////////////////
// Adjustment wheel
Tumbler {
    id: adjustmentWheel
    model: 100
    property int startX
    property int startY

    x: startX + ((chrono.adjustDayMode || chrono.adjustTimeMode) ? 8 : 0)
    Behavior on x {
        SpringAnimation { spring: 3; damping: 0.5 }
    }
    y: startY; width: 25; height: 70

    enabled: chrono.adjustDayMode || chrono.adjustTimeMode
    onEnabledChanged: {
        if (enabled) {
            chrono.reset();
            if (!chrono.started) {
                laps.reset();
                showLap = currentLap;
            }
        }
    }

    property int lastIndex: 0
    property var lastTime: Date.UTC(0)
    property double turnSpeed: 0.0
    onCurrentIndexChanged: {
        if (currentIndex != lastIndex) {
            var i1 = currentIndex;
            var i0 = lastIndex;
            if (Math.abs(i1 - i0) > 50) {
                if (i1 < i0)
                    i1 += 100;
                else
                    i0 += 100;
            }
            var deltaX = i1 - i0;
            chrono.adjust(deltaX);
            lastIndex = currentIndex;

            var deltaT = Date.now() - lastTime;
            lastTime += deltaT;
            turnSpeed = Math.abs((deltaX * 1000) / deltaT);
        }
    }

    MouseArea {
        anchors.fill: parent
        onWheel: function(wheel) {
            turn(wheel.angleDelta.y > 0 ? 1 : -1);
        }
    }

    function turn(delta) {
        if (enabled) {
            adjustmentWheel.currentIndex = (100 + adjustmentWheel.currentIndex + (delta)) % 100;
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Wheel surface
    background: Rectangle {
        anchors.fill: adjustmentWheel
        color: gray6
        border.color: gray8
        border.width: 2
        radius: 2
    }

    //////////////////////////////////////////////////////////////////////
    // Notches
    delegate: Component {
        Item {
            Rectangle {
                x: 4; y: 0; width: Tumbler.tumbler.width - 8; height: 2
                color: gray3
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Wheel shadow
    Rectangle {
        anchors.centerIn: parent
        width: parent.width; height: parent.height
        gradient: Gradient {
            GradientStop { position: 0.0; color: gray3 }
            GradientStop { position: 0.3; color: "transparent" }
            GradientStop { position: 0.5; color: "transparent" }
            GradientStop { position: 0.7; color: "transparent" }
            GradientStop { position: 1.0; color: gray3 }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Wheel axis
    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: parent.startX - parent.x + 2
        width: 20; height: 20
        z: -1
        gradient: Gradient {
            GradientStop { position: 0.0; color: gray3 }
            GradientStop { position: 0.3; color: gray6 }
            GradientStop { position: 0.5; color: gray6 }
            GradientStop { position: 0.7; color: gray6 }
            GradientStop { position: 1.0; color: gray3 }
        }
        border.color: "transparent"
    }
}
