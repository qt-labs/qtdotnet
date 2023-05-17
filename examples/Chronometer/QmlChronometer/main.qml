/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

import "QChronometer"

Window {
    visible: true
    property int windowWidth: 544 + (showHelp.checked ? 265 : 0)
    Behavior on windowWidth { SmoothedAnimation { duration: 750 } }
    width: windowWidth; minimumWidth: windowWidth; maximumWidth: windowWidth
    height: 500; minimumHeight: height; maximumHeight: height
    title: "QML Chronometer"

    //////////////////////////////////////////////////////////////////
    // Colors
    readonly property var gray1: "#111111"
    readonly property var gray3: "#333333"
    readonly property var gray4: "#444444"
    readonly property var gray6: "#666666"
    readonly property var redF6: "#FF6666"
    readonly property var green86: "#668866"
    readonly property var blue86: "#666688"
    readonly property var gray8: "#888888"

    //////////////////////////////////////////////////////////////////
    // Window background color
    color: gray4

    //////////////////////////////////////////////////////////////////
    // Stopwatch mode
    property int showLap: currentLap
    // 0: showing current lap, or stopwatch not running
    readonly property int currentLap: 0
    // 1: showing last recorded lap
    readonly property int lastLap: 1
    // 2: showing best recorded lap
    readonly property int bestLap: 2

    //////////////////////////////////////////////////////////////////
    // Watch
    Image {
        id: watch
        source: "watchface.png"
    }

    //////////////////////////////////////////////////////////////////
    // Rim
    Rectangle {
        color: "transparent"
        border { color: gray8; width: 3 }
        anchors.centerIn: watch
        width: watch.width; height: watch.height; radius: watch.width / 2
    }

    //////////////////////////////////////////////////////////////////
    // Calendar
    Text {
        enabled: false
        x: 345; y: 295; width: 32; height: 22
        transform: Rotation { origin.x: 0; origin.y: 0; angle: 30 }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font { bold: true; pointSize: 11 }
        text: chrono.day
    }

    //////////////////////////////////////////////////////////////////
    // Inset dial #1 (above-left of center; 30x minutes)
    InsetDial {
        id: insetDial1
        handSource: "/chrono_1_hand.png"
        pinSource: "/chrono_1_center.png"
        centerX: 176; centerY: 208
        rotationAngle:
            (showLap == lastLap) ? (
                /////////////////////////////////////////////////////////////
                // Show minutes of previous lap
                (laps.lastMinutes % 30) * 12
            ) : (showLap == bestLap) ? (
                /////////////////////////////////////////////////////////////
                // Show minutes of best lap
                (laps.bestMinutes % 30) * 12
            ) : (
                /////////////////////////////////////////////////////////////
                // Show minutes of current lap
                (chrono.elapsedMinutes % 30) * 12
            )
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Inset chrono counter #2 (above-right of center; 10x 1/10 second or 10x hours)
    InsetDial {
        id: insetDial2
        handSource: "/chrono_2_hand.png"
        pinSource: "/chrono_2_center.png"
        centerX: 325; centerY: 208
        rotationAngle:
            (showLap == lastLap) ? (
                /////////////////////////////////////////////////////////////
                // Show previous lap
                (laps.lastHours == 0 && laps.lastMinutes < 30) ? (
                    /////////////////////////////////////////////////////////////
                    // 1/10 seconds
                    laps.lastMilliseconds * 360 / 1000
                ) : (
                    /////////////////////////////////////////////////////////////
                    // hours
                    ((laps.lastHours % 10) + (laps.lastMinutes / 60)) * 360 / 10
                )
            ) : (showLap == bestLap) ? (
                /////////////////////////////////////////////////////////////
                // Show best lap
                (laps.bestHours == 0 && laps.bestMinutes < 30) ? (
                    /////////////////////////////////////////////////////////////
                    // 1/10 seconds
                    laps.bestMilliseconds * 360 / 1000
                ) : (
                    /////////////////////////////////////////////////////////////
                    // hours
                    ((laps.bestHours % 10) + (laps.bestMinutes / 60)) * 360 / 10
                )
            ) : (
                /////////////////////////////////////////////////////////////
                // Show current lap
                (chrono.elapsedHours < 1 && chrono.elapsedMinutes < 30) ? (
                    /////////////////////////////////////////////////////////////
                    // 1/10 seconds
                    chrono.elapsedMilliseconds * 360 / 1000
                ) : (
                    /////////////////////////////////////////////////////////////
                    // hours
                    (chrono.elapsedHours % 10) * 360 / 10
                )
            )
    }

    //////////////////////////////////////////////////////////////////////
    // Inset chrono counter #3 (below center; 60x seconds)
    InsetDial {
        id: insetDial3
        handSource: "/chrono_3_needle.png"
        pinSource: "/chrono_3_center.png"
        centerX: 250; centerY: 336
        rotationAngle: 150 + (
            (showLap == lastLap) ? (
                /////////////////////////////////////////////////////////////
                // Show seconds of previous lap
                laps.lastSeconds * 6
            ) : (showLap == bestLap) ? (
                /////////////////////////////////////////////////////////////
                // Show seconds of best lap
                laps.bestSeconds * 6
            ) : (
                /////////////////////////////////////////////////////////////
                // Show seconds of current (wall-clock) time
                chrono.seconds * 6
            ))
    }

    //////////////////////////////////////////////////////////////////////
    // Hours hand for current (wall-clock) time
    Image {
        id: hoursHand;
        source: "hour_hand.png"
        transform: Rotation {
            origin.x: 249; origin.y: 251
            angle: 110 + (chrono.hours % 12) * 30
            Behavior on angle {
                enabled: adjustmentWheel.turnSpeed < 75
                SpringAnimation { spring: 3; damping: 0.5; modulus: 360 }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Minutes hand for current (wall-clock) time
    Image {
        id: minutesHand;
        source: "minute_hand.png"
        transform: Rotation {
            origin.x: 249; origin.y: 251
            angle: -108 + chrono.minutes * 6
            Behavior on angle {
                enabled: adjustmentWheel.turnSpeed < 75
                SpringAnimation { spring: 3; damping: 0.5; modulus: 360 }
            }
        }
    }
    Image {
        source: "center.png"
    }

    //////////////////////////////////////////////////////////////////////
    // Stopwatch seconds hand
    Image {
        id: secondsHand;
        source: "second_hand.png"
        transform: Rotation {
            origin.x: 250; origin.y: 250
            angle: chrono.elapsedSeconds * 6
            Behavior on angle {
                SpringAnimation { spring: 3; damping: 0.5; modulus: 360 }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Adjustment wheel
    AdjustmentWheel {
        id: adjustmentWheel
        startX: 498; startY: 215
    }

    //////////////////////////////////////////////////////////////////////
    // Adjust date
    Switch {
        id: adjustDay
        x: 500; y: 290; padding: 0; spacing: -35
        palette.button: gray8; font { bold: true; pointSize: 9 }
        text: "Date"
        checked: chrono.adjustDayMode
        onToggled: chrono.adjustDayMode = (position == 1)
    }

    //////////////////////////////////////////////////////////////////////
    // Adjust time
    Switch {
        id: adjustTime
        x: 500; y: 310; padding: 0; spacing: -35
        palette.button: gray8; font { bold: true; pointSize: 9 }
        text: "Time"
        checked: chrono.adjustTimeMode
        onToggled: chrono.adjustTimeMode = (position == 1)
    }

    //////////////////////////////////////////////////////////////////////
    // Stopwatch start/stop button
    WatchButton {
        id: buttonStartStop
        x: 425; y: 5
        buttonText: "Start\n\nStop"; split: true
        color: chrono.started ? redF6 : !enabled ? gray3 : gray6
        enabled: !chrono.adjustDayMode && !chrono.adjustTimeMode
        onClicked: chrono.startStop()
    }

    //////////////////////////////////////////////////////////////////////
    // Stopwatch lap/reset button
    WatchButton {
        id: buttonLapReset
        x: 425; y: 425
        buttonText: "Lap\n\nReset"; split: true
        color: !enabled ? gray3 : gray6
        enabled: chrono.started
            || chrono.elapsedHours > 0
            || chrono.elapsedMinutes > 0
            || chrono.elapsedSeconds > 0
            || chrono.elapsedMilliseconds > 0
            || laps.lapCount > 0
        onClicked: {
            chrono.reset();
            if (!chrono.started) {
                laps.reset();
                showLap = currentLap;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Stopwatch last lap button
    WatchButton {
        id: buttonLastLap
        x: 5; y: 425
        buttonText: "Last\nLap"; split: false
        color: (showLap == lastLap) ? blue86 : !enabled ? gray3 : gray6
        enabled: laps.lapCount > 0
        onClicked: {
            showLapTimer.stop();
            if (laps.lapCount > 0) {
                showLap = (showLap != lastLap) ? lastLap : currentLap;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Stopwatch best lap button
    WatchButton {
        id: buttonBestLap
        x: 5; y: 5
        buttonText: "Best\nLap"; split: false
        color: (showLap == bestLap) ? green86 : !enabled ? gray3 : gray6
        enabled: laps.lapCount > 1
        onClicked: {
            showLapTimer.stop();
            if (laps.lapCount > 1) {
                showLap = (showLap != bestLap) ? bestLap : currentLap;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Timer to show last/best lap for 5 secs. after mark
    Timer {
        id: showLapTimer
        interval: 5000
        running: false
        repeat: false
        onTriggered: showLap = currentLap
    }

    //////////////////////////////////////////////////////////////////////
    // Lap events
    Connections {
        target: laps

        //////////////////////////////////////////////////////////////////////
        // Lap counter changed: new lap recorded, or lap counter reset
        function onLapCountChanged() {
            if (laps.lapCount > 0) {
                showLap = lastLap;
                showLapTimer.restart()
            }
        }

        //////////////////////////////////////////////////////////////////////
        // New best lap recorded
        function onNewBestLap() {
            if (laps.lapCount > 1) {
                showLap = bestLap;
                showLapTimer.restart()
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Keyboard events
    Shortcut {
        sequence: " "
        onActivated: buttonStartStop.clicked()
    }
    Shortcut {
        sequences: [ "Return", "Enter" ]
        onActivated: {
            if (chrono.started)
                buttonLapReset.clicked();
        }
    }
    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (chrono.adjustDayMode || chrono.adjustTimeMode)
                chrono.adjustDayMode = chrono.adjustTimeMode = false;
            else if (!chrono.started)
                buttonLapReset.clicked()
            else
                showLap = currentLap;
        }
    }
    Shortcut {
        sequence: "Tab"
        onActivated: buttonLastLap.clicked()
    }
    Shortcut {
        sequence: "Shift+Tab"
        onActivated: buttonBestLap.clicked()
    }
    Shortcut {
        sequence: "Ctrl+D"
        onActivated: {
            adjustDay.toggle();
            adjustDay.onToggled();
        }
    }
    Shortcut {
        sequence: "Ctrl+T"
        onActivated: {
            adjustTime.toggle();
            adjustTime.onToggled();
        }
    }
    Shortcut {
        sequence: "Up"
        onActivated: adjustmentWheel.turn(1)
    }
    Shortcut {
        sequence: "Down"
        onActivated: adjustmentWheel.turn(-1)
    }
    Shortcut {
        sequence: "F1"
        onActivated: showHelp.toggle()
    }

    //////////////////////////////////////////////////////////////////////
    // Usage instructions
    RoundButton {
        id: showHelp
        checkable: true
        x: 524; y: 0; width: 20; height: 40
        palette.button: gray6
        radius: 0
        contentItem: Text {
            font.bold: true
            font.pointSize: 11
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: parent.checked ? "<" : "?"
        }
    }
    Rectangle {
        x: 544; y:0
        width: 265
        height: 500
        color: gray6
        border.width: 0
        Text {
            anchors.fill: parent
            anchors.topMargin: 10
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            textFormat: Text.MarkdownText
            wrapMode: Text.WordWrap
            color: gray1
            text: "### Usage instructions

The **hours and minutes hands** show the current (wall-clock) time. The **seconds hand** shows the
stopwatch elapsed seconds. **Inset dial #1** (above-left of center) shows elapsed minutes of
current/last/best lap. **Inset dial #2** (above-right of center) shows a 1/10th-second counter of
current/last/best lap. **Inset dial #3** (below center) shows seconds of current time, or elapsed
seconds of last/best lap.

Press **Start|Stop** (shortcut: **[Space]**) to begin timing. While the stopwatch is running, press
**Lap|Reset** (shortcut: **[Enter]**) to record a lap. The stopwatch can memorize the last and the
best lap. Press **Last**&nbsp;**Lap** (shortcut: **[Tab]**) or **Best**&nbsp;**Lap** (shortcut:
**[Shift+Tab]**) to view the recorded time of the last or best lap. Press **Start|Stop** (shortcut:
**[Space]**) again to stop timing. Press **Lap|Reset** (shortcut: **[Esc]**) to reset stopwatch
counters and clear lap memory.

Press the **Date** switch (shortcut: **[Ctrl+D]**) or **Time** switch (shortcut: **[Ctrl+T]**) to
enter adjustment mode. Turn the **adjustment wheel** (shortcut: **mouse wheel** or **[Up]** /
**[Down]**) to set the desired date or time. Press the active adjustment switch (or **[Esc]**) to
leave adjustment mode. Note: entering adjustment mode resets the stopwatch."
        }
    }
}
