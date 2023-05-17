/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Collections.Concurrent;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Qt.DotNet
{
    /// <summary>
    /// Provides access to managed types, allowing native code to obtain references to, and call
    /// constructors, static methods and instance methods.
    /// </summary>
    public partial class Adapter
    {
        /// <summary>
        /// Loads a .NET assembly into memory
        /// </summary>
        /// <param name="assemblyName">Name of the assembly or path to the assembly DLL.</param>
        /// <returns>'true' if load was successful; 'false' otherwise</returns>
        public static bool LoadAssembly(string assemblyName)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.LoadAssembly(LoadAssembly);
#endif
            try {
                Assembly.Load(assemblyName);
                return true;
            } catch (Exception) {
            }

            if (File.Exists(Path.GetFullPath(assemblyName))) {
                try {
                    Assembly.LoadFile(Path.GetFullPath(assemblyName));
                    return true;
                } catch (Exception) {
                }
            }

            if (File.Exists(Path.GetFullPath($"{assemblyName}.dll"))) {
                try {
                    Assembly.LoadFile(Path.GetFullPath($"{assemblyName}.dll"));
                    return true;
                } catch (Exception) {
                }
            }
            return false;
        }

        internal class DelegateRef
        {
            public GCHandle Handle { get; }
            public bool IsValid => Handle.IsAllocated;
            public Delegate Target => Handle.Target as Delegate;
            public IntPtr FuncPtr { get; }
            public DelegateRef(GCHandle handle, IntPtr funcPtr)
            {
                Handle = handle;
                FuncPtr = funcPtr;
            }
        }

        internal class ObjectRef
        {
            public GCHandle Handle { get; }
            public object Target => Handle.Target;
            public bool IsValid => Handle.IsAllocated && Handle.Target != null;
            public ObjectRef(GCHandle handle)
            {
                Handle = handle;
            }
        }

        private static ConcurrentDictionary
            <IntPtr, ObjectRef> ObjectRefs
        { get; } = new();

        private static ConcurrentDictionary
            <IntPtr, (object Target, MethodBase Method, DelegateRef Ref)> DelegateRefs
        { get; } = new();

        private static ConcurrentDictionary
            <(object Target, MethodBase Method), DelegateRef> DelegatesByMethod
        { get; } = new();

        private static ConcurrentDictionary
            <(ObjectRef Source, string Name, IntPtr Context), EventRelay> Events
        { get; } = new();
    }
}
