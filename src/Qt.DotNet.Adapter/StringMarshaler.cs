/**************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Text.RegularExpressions;
using System.Runtime.InteropServices;

namespace Qt.DotNet
{
    internal class StringMarshaler : ICustomMarshaler, IAdapterCustomMarshaler
    {
        public static Type NativeType => typeof(string);

        public static ICustomMarshaler GetInstance(string options) => new StringMarshaler
        {
            Options = options
        };

        private string Options { get; init; }
        private bool CleanUp =>
            Regex.IsMatch(Options, @"\bCleanUp\s*=\s*true\b", RegexOptions.IgnoreCase);

        public int GetNativeDataSize()
        {
            return Marshal.SizeOf(typeof(IntPtr));
        }

        public IntPtr MarshalManagedToNative(object objStr)
        {
            return Marshal.StringToHGlobalUni(objStr as string);
        }

        public object MarshalNativeToManaged(IntPtr ptrStr)
        {
            return Marshal.PtrToStringUni(ptrStr);
        }

        public void CleanUpNativeData(IntPtr ptrStr)
        {
            if (CleanUp)
                Marshal.FreeHGlobal(ptrStr);
        }
        public void CleanUpManagedData(object objStr)
        { }
    }
}
