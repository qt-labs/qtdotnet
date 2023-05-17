/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

//////////////////////////////////////////////////////////////////
// Inset dial
Item {
    id: insetDial
    property string handSource
    property string pinSource
    property int centerX
    property int centerY
    property double rotationAngle
    //////////////////////////////////////////////////////////////////
    // Hand
    Image {
        source: insetDial.handSource
        transform: Rotation {
            origin.x: insetDial.centerX; origin.y: insetDial.centerY
            Behavior on angle {
                SpringAnimation { spring: 3; damping: 0.5; modulus: 360 }
            }
            angle: insetDial.rotationAngle
        }
    }
    //////////////////////////////////////////////////////////////////
    // Highlight
    Shape {
        id: insetDialHighlight
        anchors.fill: insetDial
        opacity: 0.5
        property var centerX: insetDial.centerX
        property var centerY: insetDial.centerY
        property var color:
            showLap == lastLap ? blue86 : showLap == bestLap ? green86 : "transparent"
        ShapePath {
            startX: insetDialHighlight.centerX; startY: insetDialHighlight.centerY
            strokeColor: "transparent"
            PathAngleArc {
                centerX: insetDialHighlight.centerX; centerY: insetDialHighlight.centerY
                radiusX: 55; radiusY: 55; startAngle: 0; sweepAngle: 360
            }
            fillGradient: RadialGradient {
                centerX: insetDialHighlight.centerX; centerY: insetDialHighlight.centerY
                centerRadius: 55;
                focalX: centerX; focalY: centerY
                GradientStop { position: 0; color: "transparent" }
                GradientStop { position: 0.6; color: "transparent" }
                GradientStop { position: 1; color: insetDialHighlight.color }
            }
        }
    }
    //////////////////////////////////////////////////////////////////
    // Center pin
    Image {
        source: insetDial.pinSource
    }
}
