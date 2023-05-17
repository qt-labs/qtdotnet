/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;

namespace WpfApp
{
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        public HwndHost HwndHost => EmbeddedAppHost;

        public double CameraPositionX => WpfThread(() => SliderCameraPositionX.Value);
        public double CameraPositionY => WpfThread(() => SliderCameraPositionY.Value);
        public double CameraPositionZ => WpfThread(() => SliderCameraPositionZ.Value);
        public double CameraRotationX => WpfThread(() => SliderCameraRotationX.Value);
        public double CameraRotationY => WpfThread(() => SliderCameraRotationY.Value);
        public double CameraRotationZ => WpfThread(() => SliderCameraRotationZ.Value);

        public static double MaxRpm => 100;
        public double Rpm => MaxRpm * Math.Max(1 / MaxRpm, SliderAnimationDuration.Value);
        public double AnimationDuration => WpfThread(() => 60000 / Rpm);

        public double FramesPerSecond
        {
            get => WpfThread(() => FpsValue.Value);
            set
            {
                WpfThread(() =>
                {
                    if (value <= FpsValue.Maximum)
                        FpsValue.Value = value;
                    FpsLabel.Text = $"{value:0.0} fps";
                });
            }
        }

        public string BackgroundColor { get; set; } = "#FFFFFF";

        public event PropertyChangedEventHandler PropertyChanged;
        private void NotifyPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void Slider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (sender is not Slider slider)
                return;
            NotifyPropertyChanged(slider.Name switch
            {
                nameof(SliderCameraPositionX) => nameof(CameraPositionX),
                nameof(SliderCameraPositionY) => nameof(CameraPositionY),
                nameof(SliderCameraPositionZ) => nameof(CameraPositionZ),
                nameof(SliderCameraRotationX) => nameof(CameraRotationX),
                nameof(SliderCameraRotationY) => nameof(CameraRotationY),
                nameof(SliderCameraRotationZ) => nameof(CameraRotationZ),
                nameof(SliderAnimationDuration) => nameof(AnimationDuration),
                _ => throw new NotSupportedException()
            });
            if (slider.Name is nameof(SliderAnimationDuration))
                NotifyPropertyChanged(nameof(Rpm));
        }

        private void Slider_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (sender is not Slider slider)
                return;
            if (slider.Name is nameof(SliderAnimationDuration))
                slider.Value = (slider.Maximum + slider.Minimum) / 10;
            else
                slider.Value = (slider.Maximum + slider.Minimum) / 2;
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
            base.OnRender(drawingContext);
            if (StackPanel.Background is not SolidColorBrush panelBrush)
                return;
            if (BackgroundColor != (BackgroundColor = panelBrush.Color.ToString()))
                NotifyPropertyChanged(nameof(BackgroundColor));
        }

        private static void WpfThread(Action action)
        {
            if (Application.Current?.Dispatcher is { } dispatcher)
                dispatcher.Invoke(action);
        }

        private static T WpfThread<T>(Func<T> func)
        {
            if (Application.Current?.Dispatcher is not { } dispatcher)
                return default;
            return dispatcher.Invoke(func);
        }
    }
}
