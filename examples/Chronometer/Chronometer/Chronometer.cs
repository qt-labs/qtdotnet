/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.ComponentModel;
using System.Diagnostics;

namespace WatchModels
{
    public interface ILapRecorder
    {
        void Mark(int hours, int minutes, int seconds, int milliseconds);
    }

    public class Chronometer : INotifyPropertyChanged
    {
        public double Hours
        {
            get => hours;
            private set => SetProperty(ref hours, value, nameof(Hours));
        }
        public double Minutes
        {
            get => minutes;
            private set => SetProperty(ref minutes, value, nameof(Minutes));
        }
        public double Seconds
        {
            get => seconds;
            private set => SetProperty(ref seconds, value, nameof(Seconds));
        }

        public int Day
        {
            get => day;
            private set => SetProperty(ref day, value, nameof(Day));
        }

        public double ElapsedHours
        {
            get => elapsedHours;
            private set => SetProperty(ref elapsedHours, value, nameof(ElapsedHours));
        }

        public double ElapsedMinutes
        {
            get => elapsedMinutes;
            private set => SetProperty(ref elapsedMinutes, value, nameof(ElapsedMinutes));
        }

        public double ElapsedSeconds
        {
            get => elapsedSeconds;
            private set => SetProperty(ref elapsedSeconds, value, nameof(ElapsedSeconds));
        }

        public double ElapsedMilliseconds
        {
            get => elapsedMilliseconds;
            private set => SetProperty(ref elapsedMilliseconds, value, nameof(ElapsedMilliseconds));
        }

        public bool Started
        {
            get => Stopwatch.IsRunning;
            private set
            {
                if (value == Stopwatch.IsRunning)
                    return;
                if (value)
                    Stopwatch.Start();
                else
                    Stopwatch.Stop();
                NotifyPropertyChanged(nameof(Started));
            }
        }

        public bool AdjustDayMode
        {
            get => adjustDayMode;
            set
            {
                if (value == adjustDayMode)
                    return;
                adjustDayMode = value;
                if (adjustDayMode) {
                    if (adjustTimeMode) {
                        adjustTimeMode = false;
                        NotifyPropertyChanged(nameof(AdjustTimeMode));
                    }
                    Started = false;
                    Reset();
                    Time.Stop();
                    baseTime = baseTime.Add(Time.Elapsed);
                    Time.Reset();
                } else if (!adjustTimeMode) {
                    Time.Start();
                }
                NotifyPropertyChanged(nameof(AdjustDayMode));
            }
        }

        public bool AdjustTimeMode
        {
            get => adjustTimeMode;
            set
            {
                if (value == adjustTimeMode)
                    return;
                adjustTimeMode = value;
                if (adjustTimeMode) {
                    if (adjustDayMode) {
                        adjustDayMode = false;
                        NotifyPropertyChanged(nameof(AdjustDayMode));
                    }
                    Started = false;
                    Reset();
                    Time.Stop();
                    baseTime = baseTime.Add(Time.Elapsed);
                    Time.Reset();
                } else if (!adjustDayMode) {
                    Time.Start();
                }
                NotifyPropertyChanged(nameof(AdjustTimeMode));
            }
        }

        public ILapRecorder LapRecorder { get; set; }

        public Chronometer()
        {
            Mechanism = new Task(async () => await MechanismLoopAsync(), MechanismLoop.Token);
            Mechanism.Start();
            baseTime = DateTime.Now;
            Time.Start();
        }

        public void StartStop()
        {
            if (AdjustTimeMode || AdjustDayMode)
                return;
            Started = !Started;
        }

        public void Reset()
        {
            ElapsedHours = ElapsedMinutes = ElapsedSeconds = ElapsedMilliseconds = 0;
            if (!Stopwatch.IsRunning) {
                Stopwatch.Reset();
            } else {
                LapRecorder?.Mark(
                    Stopwatch.Elapsed.Hours,
                    Stopwatch.Elapsed.Minutes,
                    Stopwatch.Elapsed.Seconds,
                    Stopwatch.Elapsed.Milliseconds);
                Stopwatch.Restart();
            }
        }

        public void Adjust(int delta)
        {
            if (AdjustDayMode)
                baseTime = baseTime.AddDays(delta);
            else if (AdjustTimeMode)
                baseTime = baseTime.AddSeconds(delta * 60);
            Refresh();
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void NotifyPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void Refresh()
        {
            DateTime now = baseTime.Add(Time.Elapsed);
            Day = now.Day;

            TimeSpan time = now.TimeOfDay;
            Hours = time.TotalHours;
            Minutes = time.TotalMinutes
                - (time.Hours * 60);
            Seconds = time.TotalSeconds
                - (time.Hours * 3600) - (time.Minutes * 60);

            TimeSpan elapsed = Stopwatch.Elapsed;
            ElapsedHours = elapsed.TotalHours;
            ElapsedMinutes = elapsed.TotalMinutes
                - (elapsed.Hours * 60);
            ElapsedSeconds = elapsed.TotalSeconds
                - (elapsed.Hours * 3600) - (elapsed.Minutes * 60);
            ElapsedMilliseconds = elapsed.TotalMilliseconds
                - (elapsed.Hours * 3600000) - (elapsed.Minutes * 60000) - (elapsed.Seconds * 1000);
        }

        private async Task MechanismLoopAsync()
        {
            while (!MechanismLoop.IsCancellationRequested) {
                await Task.Delay(5);
                Refresh();
            }
        }

        private void SetProperty<T>(ref T currentValue, T newValue, string name)
        {
            if (newValue.Equals(currentValue))
                return;
            currentValue = newValue;
            NotifyPropertyChanged(name);
        }

        private double hours;
        private double minutes;
        private double seconds;
        private int day;
        private double elapsedHours;
        private double elapsedMinutes;
        private double elapsedSeconds;
        private double elapsedMilliseconds;
        private bool adjustDayMode;
        private bool adjustTimeMode;

        private DateTime baseTime;
        private Stopwatch Time { get; } = new();
        private Stopwatch Stopwatch { get; } = new();

        private CancellationTokenSource MechanismLoop { get; } = new();
        private Task Mechanism { get; }
    }
}
