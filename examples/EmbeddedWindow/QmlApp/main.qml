/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

import QtQuick
import QtQuick3D

Rectangle {
    width: mainWindow.hostWidth
    height: mainWindow.hostHeight
    gradient: Gradient {
        GradientStop { position: 0.0; color: mainWindow.backgroundColor }
        GradientStop { position: 0.40; color: "#E6ECED" }
        GradientStop { position: 0.50; color: "#CCD9DB" }
        GradientStop { position: 0.60; color: "#B3C6C9" }
        GradientStop { position: 0.70; color: "#99B3B7" }
        GradientStop { position: 0.75; color: "#80A0A5" }
        GradientStop { position: 0.80; color: "#668D92" }
        GradientStop { position: 0.85; color: "#4D7A80" }
        GradientStop { position: 0.90; color: "#33676E" }
        GradientStop { position: 0.95; color: "#19545C" }
        GradientStop { position: 1.0; color: "#00414A" }
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 5
        anchors.rightMargin: 10
        font.pointSize: 20
        font.weight: Font.Bold
        color: "#001012"
        text: "QML"
    }

    View3D {
        id: view
        anchors.fill: parent

        PerspectiveCamera {
            position: Qt.vector3d(
                mainWindow.cameraPositionX,
                mainWindow.cameraPositionY + 200,
                mainWindow.cameraPositionZ + 300)
            eulerRotation.x: (mainWindow.cameraRotationX - 30) % 360
            eulerRotation.y: mainWindow.cameraRotationY
            eulerRotation.z: mainWindow.cameraRotationZ
        }

        DirectionalLight {
            eulerRotation.x: (mainWindow.cameraRotationX - 30) % 360
            eulerRotation.y: mainWindow.cameraRotationY
            eulerRotation.z: mainWindow.cameraRotationZ
        }

        Model {
            id: cube
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    sourceItem: Item {
                        id: qt_logo
                        width: 230
                        height: 230
                        visible: false
                        layer.enabled: true
                        Rectangle {
                            anchors.fill: parent
                            color: "black"
                            Image {
                                anchors.fill: parent
                                source: "qt_logo.png"
                            }
                            Text {
                                anchors.top: parent.top
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "white"
                                font.pixelSize: 17
                                text: "The Future is Written with Qt"
                            }
                            Text {
                                anchors.bottom: parent.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "white"
                                font.pixelSize: 17
                                text: "The Future is Written with Qt"
                            }
                        }
                    }
                }
            }
            property var rotation: Qt.vector3d(0, 90, 0)

            eulerRotation.x: rotation.x % 360
            eulerRotation.y: rotation.y % 360
            eulerRotation.z: rotation.z % 360

            Vector3dAnimation on rotation {
                property var delta: Qt.vector3d(0, 0, 0)
                id: cubeAnimation
                loops: Animation.Infinite
                duration: mainWindow.animationDuration
                from: Qt.vector3d(0, 0, 0).plus(delta)
                to: Qt.vector3d(360, 0, 360).plus(delta)
                onDurationChanged: {
                    delta = cube.eulerRotation;
                    restart();
                }
            }
        }
    }

    property var t0: 0
    property var n: 0

    Component.onCompleted: {
        window.afterFrameEnd.connect(
            function() {
                var t = Date.now();
                if (t0 == 0) {
                    t0 = t;
                    n = 1;
                } else {
                    var dt = t - t0;
                    if (dt >= 1000) {
                        mainWindow.framesPerSecond = (1000 * n) / dt;
                        n = 0;
                        t0 = t;
                    } else {
                        n++;
                    }
                }
            });
    }
}
