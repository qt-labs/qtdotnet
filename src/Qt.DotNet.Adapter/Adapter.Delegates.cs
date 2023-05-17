/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Runtime.InteropServices;

namespace Qt.DotNet
{
    public partial class Adapter
    {
        /// <summary>
        /// Delegate types for Adapter public functions.
        /// </summary>
        public static class Delegates
        {
            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate bool LoadAssembly(
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string assemblyName);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate IntPtr ResolveStaticMethod(
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string targetType,
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string methodName,
                [In] int parameterCount,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)]
                [In] Parameter[] parameters);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate IntPtr ResolveConstructor(
                [In] int parameterCount,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 0)]
                [In] Parameter[] parameters);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate IntPtr ResolveInstanceMethod(
                [In] IntPtr objRefPtr,
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string methodName,
                [In] int parameterCount,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)]
                [In] Parameter[] parameters);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate IntPtr ResolveSafeMethod(
                [In] IntPtr funcPtr,
                [In] int parameterCount,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
                [In] Parameter[] parameters);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void NativeEventHandler(
                [In] IntPtr context,
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string eventName,
                [MarshalAs(UnmanagedType.CustomMarshaler,
                    MarshalTypeRef = typeof(ObjectMarshaler))]
                [In] object sender,
                [MarshalAs(UnmanagedType.CustomMarshaler,
                    MarshalTypeRef = typeof(ObjectMarshaler))]
                [In] object args);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void AddEventHandler(
                [In] IntPtr objRefPtr,
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string eventName,
                [In] IntPtr context,
                [In] NativeEventHandler eventHandler);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void RemoveEventHandler(
                [In] IntPtr objRefPtr,
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string eventName,
                [In] IntPtr context);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void RemoveAllEventHandlers(
                [In] IntPtr objRefPtr);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate IntPtr AddObjectRef(
                [In] IntPtr objRefPtr,
                [In] bool weakRef);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void FreeDelegateRef(
                [In] IntPtr delRefPtr);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void FreeObjectRef(
                [In] IntPtr objRefPtr);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void FreeTypeRef(
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string typeName);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            [return: MarshalAs(UnmanagedType.CustomMarshaler,
                MarshalTypeRef = typeof(ObjectMarshaler))]
            public delegate InterfaceProxy AddInterfaceProxy(
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string interfaceName);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void SetInterfaceMethod(
                [MarshalAs(UnmanagedType.CustomMarshaler,
                    MarshalTypeRef = typeof(ObjectMarshaler))]
                [In] InterfaceProxy proxy,
                [MarshalAs(UnmanagedType.LPWStr)]
                [In] string methodName,
                [In] int parameterCount,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)]
                [In] Parameter[] parameters,
                [In] IntPtr callbackPtr,
                [In] IntPtr cleanUpPtr,
                [In] IntPtr context);

            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate IntPtr GetObject
                ([In] IntPtr objRefPtr, [MarshalAs(UnmanagedType.LPWStr)][In] string path);

#if DEBUG || TESTS
            [UnmanagedFunctionPointer(CallingConvention.Winapi)]
            public delegate void Stats(
                [Out] out int refCount,
                [Out] out int staticCount,
                [Out] out int eventCount);
#endif
        }
    }
}
