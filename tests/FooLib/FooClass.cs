/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FooLib
{
    public interface IBarTransformation
    {
        string Transform(string bar);
    }

    public class BarIdentity : IBarTransformation
    {
        public string Transform(string bar) => bar;
    }

    public class Foo : INotifyPropertyChanged
    {
        public Foo(IBarTransformation barTransformation)
        {
            BarTransformation = barTransformation ?? new BarIdentity();
        }

        public Foo() : this(null)
        { }

        private IBarTransformation BarTransformation { get; }

        public event PropertyChangedEventHandler PropertyChanged;

        private void NotifyPropertyChanged([CallerMemberName] string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private string bar;
        public string Bar
        {
            get => bar;
            set
            {
                bar = BarTransformation?.Transform(value) ?? value;
                NotifyPropertyChanged();
            }
        }

        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.LPWStr)]
        public delegate string FormatNumberDelegate(
            [In, MarshalAs(UnmanagedType.LPWStr)] string format, int number);

        public static string FormatNumber(string format, int number)
        {
            return string.Format(format, number);
        }

        public static int EntryPoint(IntPtr arg, int argLength)
        {
            return Convert.ToInt32(Marshal.PtrToStringUni(arg, argLength));
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public class Date
        {
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Year = "";
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Month = "";
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Day = "";
        }

        [return: MarshalAs(UnmanagedType.LPWStr)]
        public delegate string FormatDateDelegate(
            [In, MarshalAs(UnmanagedType.LPWStr)] string format, [In] Date date);

        public static string FormatDate(string format, Date date)
        {
            return string.Format(format, date.Year, date.Month, date.Day);
        }
    }
}
