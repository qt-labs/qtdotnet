/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System;
using System.Windows;

namespace WpfApp
{
    public static class Program
    {
        [STAThread]
        public static int Main()
        {
            var application = new Application();
            (MainWindow ??= new MainWindow()).InitializeComponent();
            MainWindow.Show();
            return application.Run();
        }
        public static MainWindow MainWindow { get; set; }
    }
}
