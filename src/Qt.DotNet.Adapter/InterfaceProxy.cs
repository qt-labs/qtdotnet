/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Qt.DotNet
{
    public class InterfaceProxy
    {
        public delegate void CleanUpDelegate(IntPtr context, ulong count);
        public CleanUpDelegate CleanUpCallback;

        public void CleanUp(IntPtr callback, IntPtr context, ulong count)
        {
            CleanUpCallback(context, count);
        }
    }

    public partial class Adapter
    {
        public static InterfaceProxy AddInterfaceProxy(string interfaceName)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.AddInterfaceProxy(AddInterfaceProxy);
#endif
            var interfaceType = Type.GetType(interfaceName)
                ?? throw new ArgumentException(
                    $"Interface '{interfaceName}' not found", nameof(interfaceName));
            var proxyType = CodeGenerator.CreateInterfaceProxyType(interfaceType);
            var ctor = proxyType.GetConstructor(Array.Empty<Type>());
#if DEBUG
            Debug.Assert(ctor != null, nameof(ctor) + " is null");
#endif
            var obj = ctor.Invoke(null);
            return obj as InterfaceProxy;
        }

        public static void SetInterfaceMethod(
            InterfaceProxy proxy,
            string methodName,
            int parameterCount,
            Parameter[] parameters,
            IntPtr callbackPtr,
            IntPtr cleanUpPtr,
            IntPtr context)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.SetInterfaceMethod(SetInterfaceMethod);
#endif
            var type = proxy.GetType();
            var parameterTypes = parameters
                .Skip(3)
                .Select((x, i) => x.GetParameterType()
                    ?? throw new ArgumentException($"Type not found [{i}]", nameof(parameters)))
                .ToArray();

            var method = type.GetMethod(methodName, parameterTypes)
                ?? throw new ArgumentException(
                    $"Method '{methodName}' not found", nameof(methodName));

            var delegateType = CodeGenerator.CreateDelegateType(method.Name, parameters);
            var delegateTypeInvoke = delegateType.GetMethod("Invoke");
#if DEBUG
            Debug.Assert(delegateTypeInvoke != null, nameof(delegateTypeInvoke) + " != null");
#endif
            var paramTypes = delegateTypeInvoke.GetParameters()
                .Select(p => p.ParameterType);

            var fieldDelegate = proxy.GetType().GetFields()
                .FirstOrDefault(f => f.FieldType.IsAssignableTo(typeof(Delegate))
                    && f.FieldType.GetMethod("Invoke") is MethodInfo invoke
                    && invoke.GetParameters()
                        .Zip(paramTypes)
                        .All(x => x.First.ParameterType == x.Second)
                    && invoke.ReturnType.IsAssignableTo(delegateTypeInvoke.ReturnType))
                    ?? throw new ArgumentException("Signature mismatch", nameof(parameters));

            var fieldNative = proxy.GetType().GetField($"Native_{fieldDelegate.Name}");
            var callbackDelegate = Marshal.GetDelegateForFunctionPointer(callbackPtr, delegateType);
#if DEBUG
            Debug.Assert(fieldNative != null, nameof(fieldNative) + " is null");
#endif
            fieldNative.SetValue(proxy, callbackDelegate);

            var fieldContext = proxy.GetType().GetField($"Context_{fieldDelegate.Name}");
#if DEBUG
            Debug.Assert(fieldContext != null, nameof(fieldContext) + " is null");
#endif
            fieldContext.SetValue(proxy, context);

            proxy.CleanUpCallback = Marshal.GetDelegateForFunctionPointer(
                cleanUpPtr, typeof(InterfaceProxy.CleanUpDelegate))
                as InterfaceProxy.CleanUpDelegate;
        }
    }
}
