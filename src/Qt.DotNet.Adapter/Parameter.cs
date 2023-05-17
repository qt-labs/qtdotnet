/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Qt.DotNet
{
    public interface IAdapterCustomMarshaler
    {
        static Type NativeType { get; }
    }

    /// <summary>
    /// Information describing a parameter or the return type of a function.
    /// Used to generate dynamic delegate types for methods requested through the Adapter.
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct Parameter
    {
        [MarshalAs(UnmanagedType.LPWStr)]
        public string TypeName = string.Empty;
        public readonly ulong ParamInfo = 0;

        // Format of parameter info field
        // 63......................31...........15............0
        // |------ ARRAYLENGTH -----|--- FLAGS --|--- TYPE ---|

        const int TYPE_OFFSET = 0;
        const int TYPE_SIZE = 16;

        const int FLAGS_OFFSET = TYPE_OFFSET + TYPE_SIZE;
        const int FLAGS_SIZE = 16;
        const int FLAGS_IN_BIT = 0;
        const int FLAGS_OUT_BIT = 1;
        const int FLAGS_ARRAY_BIT = 2;
        const int FLAGS_FIXEDLENGTH_BIT = 3;
        const int FLAGS_WEAKREF_BIT = 4;

        const int ARRAYLENGTH_OFFSET = FLAGS_OFFSET + FLAGS_SIZE;
        const int ARRAYLENGTH_SIZE = 32;

        private static ulong MASK(ulong value, int size) => value & ((((ulong)1) << size) - 1);
        private static bool FLAG(ulong value, int offset) => ((value >> offset) & 1) == 1;

        public Type GetParameterType()
        {
            if (IsVoid)
                return typeof(void);
            if (MarshalAs == UnmanagedType.CustomMarshaler) {
                Type customMarshalerType = Type.GetType(TypeName);
#if TEST || DEBUG
                Debug.Assert(customMarshalerType != null, nameof(customMarshalerType) + " is null");
#endif
                if (customMarshalerType.IsAssignableTo(typeof(IAdapterCustomMarshaler))) {
                    var nativeType = customMarshalerType.GetProperty(
                        "NativeType", BindingFlags.Public | BindingFlags.Static);
                    if (nativeType != null && nativeType.GetValue(null) is Type customType)
                        return customType;
                }
                return typeof(object);
            }
            if (!string.IsNullOrEmpty(TypeName))
                return Type.GetType(TypeName);
            string typeName = $"{nameof(System)}."
                + MarshalAs switch
                {
                    UnmanagedType.Bool => nameof(Boolean),
                    UnmanagedType.I1 => nameof(SByte),
                    UnmanagedType.U1 => nameof(Byte),
                    UnmanagedType.I2 => nameof(Int16),
                    UnmanagedType.U2 => nameof(UInt16),
                    UnmanagedType.I4 => nameof(Int32),
                    UnmanagedType.U4 => nameof(UInt32),
                    UnmanagedType.I8 => nameof(Int64),
                    UnmanagedType.U8 => nameof(UInt64),
                    UnmanagedType.R4 => nameof(Single),
                    UnmanagedType.R8 => nameof(Double),
                    UnmanagedType.SysInt => nameof(IntPtr),
                    UnmanagedType.SysUInt => nameof(UIntPtr),
                    UnmanagedType.FunctionPtr => nameof(Delegate),
                    UnmanagedType.LPStr => nameof(String),
                    UnmanagedType.LPWStr => nameof(String),
                    UnmanagedType.LPTStr => nameof(String),
                    UnmanagedType.ByValTStr => nameof(String),
                    _ => nameof(Object)
                }
                + (IsArray ? "[]" : "");

            var type = Type.GetType(typeName);
            if (type != null)
                TypeName = typeName;
            return type;
        }

        public Parameter(Type type, ulong paramInfo = 0)
        {
            TypeName = type.FullName;
            ParamInfo = paramInfo;
        }

        public Parameter(string typeName, ulong paramInfo = 0)
        {
            TypeName = typeName;
            ParamInfo = paramInfo;
        }

        public Parameter(UnmanagedType type)
        {
            ParamInfo = MASK((ulong)type, TYPE_SIZE) << TYPE_OFFSET;
        }

        public static UnmanagedType ObjectRef => (UnmanagedType)MASK(ulong.MaxValue, TYPE_SIZE);
        public UnmanagedType MarshalAs => (UnmanagedType)MASK(ParamInfo >> TYPE_OFFSET, TYPE_SIZE);
        public bool IsIn => FLAG(ParamInfo >> FLAGS_OFFSET, FLAGS_IN_BIT);
        public bool IsOut => FLAG(ParamInfo >> FLAGS_OFFSET, FLAGS_OUT_BIT);
        public bool IsArray => FLAG(ParamInfo >> FLAGS_OFFSET, FLAGS_ARRAY_BIT);
        public bool IsFixedLength => FLAG(ParamInfo >> FLAGS_OFFSET, FLAGS_FIXEDLENGTH_BIT);
        public bool IsWeakRef => FLAG(ParamInfo >> FLAGS_OFFSET, FLAGS_WEAKREF_BIT);
        public int ArrayLength => (int)MASK(ParamInfo >> ARRAYLENGTH_OFFSET, ARRAYLENGTH_SIZE);
        public bool IsVoid => string.IsNullOrEmpty(TypeName) && MarshalAs == 0;
        public static Parameter Void { get; } = new();
    }
}
