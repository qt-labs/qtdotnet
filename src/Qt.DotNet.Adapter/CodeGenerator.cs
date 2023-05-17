/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Collections.Concurrent;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;

namespace Qt.DotNet
{
    using DelegateIndex = ConcurrentDictionary<(MethodBase, Parameter[]), Type>;
    using ProxyIndex = ConcurrentDictionary<(MethodBase, Parameter[]), MethodInfo>;
    using IIndexer = IEqualityComparer<(MethodBase method, Parameter[] parameters)>;

    public class SafeReturn<T>
    {
        public T Value { get; init; } = default;
        public Exception Exception { get; init; }
    }

    /// <summary>
    /// Functions that generate code at run-time, needed to support native interop.
    /// </summary>
    internal static class CodeGenerator
    {
        public static MethodInfo CreateSafeMethod(MethodInfo unsafeMethod)
        {
#if TESTS || DEBUG
            Debug.Assert(unsafeMethod.DeclaringType != null, "unsafeMethod.DeclaringType is null");
#endif
            var typeGen = ModuleGen.DefineType(
                UniqueName("Safe", unsafeMethod.DeclaringType?.Name, unsafeMethod.Name),
                TypeAttributes.Public, typeof(object));
            var returnType = unsafeMethod.ReturnType;
            var returnTypeIsVoid = returnType == typeof(void);
            var paramTypes = unsafeMethod.GetParameters()
                .Select(x => x.ParameterType)
                .Prepend(typeof(object))
                .ToArray();

            Type safeReturnType = returnTypeIsVoid
                ? typeof(SafeReturn<bool>)
                : typeof(SafeReturn<>).MakeGenericType(returnType);

            var safeReturnCtor = safeReturnType.GetConstructor(Array.Empty<Type>());
            var safeReturnSetValue = safeReturnType.GetMethod("set_Value");
            var safeReturnSetException = safeReturnType.GetMethod("set_Exception");
#if TESTS || DEBUG
            Debug.Assert(safeReturnCtor != null, nameof(safeReturnCtor) + " is null");
            Debug.Assert(safeReturnSetValue != null, nameof(safeReturnSetValue) + " is null");
            Debug.Assert(safeReturnSetException != null,
                nameof(safeReturnSetException) + " is null");
#endif
            var safeMethod = typeGen.DefineMethod("SafeInvoke",
                MethodAttributes.Public | MethodAttributes.HideBySig | MethodAttributes.Static,
                safeReturnType, paramTypes);
            var code = safeMethod.GetILGenerator();

            code.DeclareLocal(safeReturnType);
            code.DeclareLocal(returnTypeIsVoid ? typeof(bool) : returnType);
            code.DeclareLocal(typeof(Exception));

            // {0} = new SafeReturn<T>();
            code.Emit(OpCodes.Newobj, safeReturnCtor);
            code.Emit(OpCodes.Stloc_0);

            //try
            code.BeginExceptionBlock();
            {
                // this = arg0
                if (!unsafeMethod.IsStatic)
                    code.Emit(OpCodes.Ldarg_0);

                // Load arguments into stack
                for (int paramIdx = 1; paramIdx < paramTypes.Length; ++paramIdx) {
                    if (paramIdx == 1)
                        code.Emit(OpCodes.Ldarg_1);
                    else if (paramIdx == 2)
                        code.Emit(OpCodes.Ldarg_2);
                    else if (paramIdx == 3)
                        code.Emit(OpCodes.Ldarg_3);
                    else
                        code.Emit(OpCodes.Ldarg_S, paramIdx + 1);
                }

                // Invoke method
                if (!returnTypeIsVoid) {
                    // {1} = unsafeMethod([...]);
                    code.Emit(OpCodes.Call, unsafeMethod);
                    code.Emit(OpCodes.Stloc_1);
                    // {0}.Value = {1};
                    code.Emit(OpCodes.Ldloc_0);
                    code.Emit(OpCodes.Ldloc_1);
                    code.Emit(OpCodes.Call, safeReturnSetValue);
                } else {
                    // unsafeMethod([...]);
                    code.Emit(OpCodes.Call, unsafeMethod);
                }
            }
            // ... } catch (Exception [0]) { ...
            code.BeginCatchBlock(typeof(Exception));
            {
                // {2} = [0];
                code.Emit(OpCodes.Stloc_2);
                code.Emit(OpCodes.Ldloc_0);
                code.Emit(OpCodes.Ldloc_2);
                code.Emit(OpCodes.Call, safeReturnSetException);
            }
            // ... }
            code.EndExceptionBlock();

            // Return [0]
            code.Emit(OpCodes.Ldloc_0);
            code.Emit(OpCodes.Ret);

            var safeInvokeType = typeGen.CreateType()
                ?? throw new TypeAccessException("Error creating safe invoke");
            var safeInvoke = safeInvokeType.GetMethod("SafeInvoke");
            return safeInvoke;
        }

        public static Type CreateInterfaceProxyType(Type interfaceType)
        {
            if (InterfaceProxyTypes.TryGetValue(interfaceType, out var proxyType))
                return proxyType;

            var typeGen = ModuleGen.DefineType(UniqueName("Proxy", interfaceType.Name),
                TypeAttributes.Public, typeof(InterfaceProxy), new[] { interfaceType });

            var interfaceImpl_CleanUp = typeof(InterfaceProxy).GetMethod("CleanUp");
#if TEST || DEBUG
            Debug.Assert(interfaceImpl_CleanUp != null, nameof(interfaceImpl_CleanUp) + " is null");
#endif
            foreach (var method in interfaceType.GetMethods()) {
                var parameterInfos = method.GetParameters();
                var paramTypes = parameterInfos
                    .Select(x => x.ParameterType)
                    .ToArray();
                var callbackParamTypes = paramTypes
                    .Prepend(typeof(ulong))
                    .Prepend(typeof(IntPtr))
                    .ToArray();
                var callbackParameters = callbackParamTypes
                    .Prepend(method.ReturnType)
                    .Select(t => new Parameter(t))
                    .ToArray();

                var delegateGen = typeGen.DefineNestedType(
                    UniqueName(method.Name, "Delegate"),
                    TypeAttributes.Sealed | TypeAttributes.NestedPublic,
                    typeof(MulticastDelegate));
                var callbackInvoke = InitDelegateType(delegateGen, callbackParameters);

                var callbackGen = typeGen.DefineField(
                    UniqueName(method.Name, "Callback"), delegateGen, FieldAttributes.Public);
                var nativeCallbackGen = typeGen.DefineField(
                    $"Native_{callbackGen.Name}", typeof(Delegate), FieldAttributes.Public);
                var cleanUpPtr = typeGen.DefineField(
                    $"CleanUp_{callbackGen.Name}", typeof(IntPtr), FieldAttributes.Public);
                var contextGen = typeGen.DefineField(
                    $"Context_{callbackGen.Name}", typeof(IntPtr), FieldAttributes.Public);
                var countGen = typeGen.DefineField(
                    $"Count_{callbackGen.Name}", typeof(ulong), FieldAttributes.Public);

                var methodGen = typeGen.DefineMethod(method.Name,
                    MethodAttributes.Public
                    | MethodAttributes.Final
                    | MethodAttributes.HideBySig
                    | MethodAttributes.NewSlot
                    | MethodAttributes.Virtual,
                    method.ReturnType, paramTypes);
                var code = methodGen.GetILGenerator();

                // ++Count;
                code.Emit(OpCodes.Ldarg_0);
                code.Emit(OpCodes.Ldarg_0);
                code.Emit(OpCodes.Ldfld, countGen);
                code.Emit(OpCodes.Ldc_I4_1);
                code.Emit(OpCodes.Conv_I8);
                code.Emit(OpCodes.Add);
                code.Emit(OpCodes.Stfld, countGen);

                //  NativeCallback.Invoke(
                code.Emit(OpCodes.Ldarg_0);
                code.Emit(OpCodes.Ldfld, nativeCallbackGen);
                //      context,
                code.Emit(OpCodes.Ldarg_0);
                code.Emit(OpCodes.Ldfld, contextGen);
                //      count,
                code.Emit(OpCodes.Ldarg_0);
                code.Emit(OpCodes.Ldfld, countGen);
                // Load method call arguments into stack
                for (int paramIdx = 0; paramIdx < paramTypes.Length; ++paramIdx) {
                    if (paramIdx == 0)
                        code.Emit(OpCodes.Ldarg_1);
                    else if (paramIdx == 1)
                        code.Emit(OpCodes.Ldarg_2);
                    else if (paramIdx == 2)
                        code.Emit(OpCodes.Ldarg_3);
                    else
                        code.Emit(OpCodes.Ldarg_S, paramIdx + 1);
                }
                //  ); //NativeCallback.Invoke
                code.Emit(OpCodes.Callvirt, callbackInvoke);

                // CleanUp.Invoke(
                code.Emit(OpCodes.Ldarg_0);
                //      cleanUpPtr,
                code.Emit(OpCodes.Ldarg_0);
                code.Emit(OpCodes.Ldfld, cleanUpPtr);
                //      context,
                code.Emit(OpCodes.Ldarg_0);
                code.Emit(OpCodes.Ldfld, contextGen);
                //      count,
                code.Emit(OpCodes.Ldarg_0);
                code.Emit(OpCodes.Ldfld, countGen);
                //  ); //CleanUp.Invoke
                code.Emit(OpCodes.Callvirt, interfaceImpl_CleanUp);

                // return <ret>;
                code.Emit(OpCodes.Ret);

                // Generate nested type
                delegateGen.CreateType();
            }
            proxyType = typeGen.CreateType();
            InterfaceProxyTypes.TryAdd(interfaceType, proxyType);
            return proxyType;
        }

        public static Type CreateDelegateType(string methodName, Parameter[] parameters)
        {
            // Generate dynamic Delegate sub-type
            var typeGen = ModuleGen.DefineType(
                UniqueName(methodName, "Delegate"),
                TypeAttributes.Sealed | TypeAttributes.Public,
                typeof(MulticastDelegate));

            InitDelegateType(typeGen, parameters);

            // Get generated type
            var delegateType = typeGen.CreateType()
                ?? throw new TypeAccessException("Error creating dynamic delegate type");

            return delegateType;
        }

        /// <summary>
        /// Generate a delegate type that matches the signature of a given method.
        /// </summary>
        /// <remarks>
        /// This is used to configure marshaling for incoming native interop calls.
        /// </remarks>
        /// <param name="method">Information on the managed method being called</param>
        /// <param name="parameters">Marshaling configuration of each parameter</param>
        /// <returns>Generated delegate type</returns>
        /// <exception cref="TypeAccessException"/>
        public static Type CreateDelegateTypeForMethod(MethodInfo method, Parameter[] parameters)
        {
#if TESTS || DEBUG
            Debug.Assert(method.GetParameters().Length == parameters.Length - 1);
            Debug.Assert(method.DeclaringType != null, "method.DeclaringType is null");
            Debug.Assert(method.ReturnType.IsAssignableTo(parameters[0].GetParameterType())
                || method.ReturnType.IsAssignableFrom(parameters[0].GetParameterType()));
            Debug.Assert(method.GetParameters().Zip(parameters.Skip(1))
                .All(x => x.First.ParameterType.IsAssignableTo(x.Second.GetParameterType())
                    || x.First.ParameterType.IsAssignableFrom(x.Second.GetParameterType())));
#endif
            // Check if already in cache
            if (DelegateTypes.TryGetValue((method, parameters), out Type delegateType))
                return delegateType;

            // Generate dynamic Delegate sub-type
            var typeGen = ModuleGen.DefineType(
                UniqueName(method.DeclaringType.Name, method.Name),
                TypeAttributes.Sealed | TypeAttributes.Public,
                typeof(MulticastDelegate));

            // Generate constructor for Delegate sub-type
            var ctorGen = typeGen.DefineConstructor(
                MethodAttributes.HideBySig | MethodAttributes.Public,
                CallingConventions.Standard,
                new[] { typeof(object), typeof(IntPtr) });
            ctorGen.SetImplementationFlags(MethodImplAttributes.CodeTypeMask);

            // Delegate matches return and param types
            Type returnType = method.ReturnType;
            var paramInfos = method.GetParameters();
            var paramTypes = paramInfos
                .Select(p => p.ParameterType)
                .ToArray();

            // Generate Invoke() method
            var invokeGen = typeGen.DefineMethod("Invoke",
                MethodAttributes.HideBySig | MethodAttributes.Virtual | MethodAttributes.Public,
                returnType, paramTypes);
            invokeGen.SetImplementationFlags(MethodImplAttributes.CodeTypeMask);

            // Generate return parameter (if return type is not void)
            var returnParam = parameters[0];
            if (!returnParam.IsVoid) {
                var returnGen = invokeGen.DefineParameter(0, ParameterAttributes.Retval, "");
                SetMarshalAs(returnGen, returnParam);
            }

            // Generate method parameters
            for (int i = 0; i < parameters.Length - 1; i++) {
                var param = parameters[i + 1];
                var paramInfo = paramInfos[i];
                var paramGen = invokeGen.DefineParameter(
                    i + 1, ParameterAttributes.None, paramInfo.Name);
                if (param.MarshalAs != 0)
                    SetMarshalAs(paramGen, param);
                if (param.IsIn)
                    SetIn(paramGen);
                if (param.IsOut)
                    SetOut(paramGen);
            }

            // Get generated type
            delegateType = typeGen.CreateType()
                ?? throw new TypeAccessException("Error creating dynamic delegate type");

            // Add to cache and return
            DelegateTypes.TryAdd((method, parameters), delegateType);
            return delegateType;
        }

        /// <summary>
        /// Generate static method that encapsulates a call to a given constructor.
        /// </summary>
        /// <remarks>
        /// This is used to configure marshaling for incoming native interop calls to constructors.
        /// </remarks>
        /// <param name="ctor">Constructor information</param>
        /// <param name="parameters">Marshaling configuration of each parameter</param>
        /// <returns>Generated method information</returns>
        /// <exception cref="TypeAccessException"></exception>
        public static MethodInfo CreateProxyMethodForCtor(
            ConstructorInfo ctor, Parameter[] parameters)
        {
#if TESTS || DEBUG
            Debug.Assert(ctor.GetParameters().Length == parameters.Length - 1);
            Debug.Assert(ctor.DeclaringType != null, "ctor.DeclaringType is null");
            Debug.Assert(ctor.DeclaringType.IsAssignableTo(parameters[0].GetParameterType()));
            Debug.Assert(ctor.GetParameters().Zip(parameters.Skip(1))
                .All(x => x.First.ParameterType.IsAssignableTo(x.Second.GetParameterType())));
#endif
            // Check if already in cache
            if (Proxies.TryGetValue((ctor, parameters), out MethodInfo proxy))
                return proxy;

            // Proxy matches return and param types
            Type returnType = ctor.DeclaringType;
            var paramInfos = ctor.GetParameters();
            var paramTypes = paramInfos
                .Select(p => p.ParameterType)
                .ToArray();

            // Generate placeholder type for proxy method
            var typeGen = ModuleGen.DefineType(
                UniqueName(returnType.Name, "ctor"),
                TypeAttributes.Sealed | TypeAttributes.Public,
                typeof(object));

            // Generate proxy method Create()
            var proxyGen = typeGen.DefineMethod("Create",
                MethodAttributes.Public | MethodAttributes.HideBySig | MethodAttributes.Static,
                returnType, paramTypes);

            // Get code generator for proxy method
            var code = proxyGen.GetILGenerator();

            // Load arguments into stack
            for (int paramIdx = 0; paramIdx < paramTypes.Length; ++paramIdx) {
                if (paramIdx == 0)
                    code.Emit(OpCodes.Ldarg_0);
                else if (paramIdx == 1)
                    code.Emit(OpCodes.Ldarg_1);
                else if (paramIdx == 2)
                    code.Emit(OpCodes.Ldarg_2);
                else if (paramIdx == 3)
                    code.Emit(OpCodes.Ldarg_3);
                else
                    code.Emit(OpCodes.Ldarg_S, paramIdx);
            }

            // Invoke encapsulated constructor
            code.Emit(OpCodes.Newobj, ctor);

            // Return newly created object
            code.Emit(OpCodes.Ret);

            // Get generated type
            var proxyType = typeGen.CreateType()
                ?? throw new TypeAccessException("Error creating dynamic ctor proxy");

            // Get generated method
            proxy = proxyType.GetMethod("Create");

            // Add to cache and return
            Proxies.TryAdd((ctor, parameters), proxy);
            return proxy;
        }

        private static MethodBuilder InitDelegateType(TypeBuilder typeGen, Parameter[] parameters)
        {
            // Generate constructor for Delegate sub-type
            var ctorGen = typeGen.DefineConstructor(
                MethodAttributes.HideBySig | MethodAttributes.Public,
                CallingConventions.Standard,
                new[] { typeof(object), typeof(IntPtr) });
            ctorGen.SetImplementationFlags(MethodImplAttributes.CodeTypeMask);

            // Delegate matches return and param types
            Type returnType = parameters[0].GetParameterType();
            var paramTypes = parameters
                .Skip(1)
                .Select(p => p.GetParameterType())
                .ToArray();

            // Generate Invoke() method
            var invokeGen = typeGen.DefineMethod("Invoke",
                MethodAttributes.HideBySig | MethodAttributes.Virtual | MethodAttributes.Public,
                returnType, paramTypes);
            invokeGen.SetImplementationFlags(MethodImplAttributes.CodeTypeMask);

            // Generate return parameter (if return type is not void)
            var returnParam = parameters[0];
            if (!returnParam.IsVoid) {
                var returnGen = invokeGen.DefineParameter(0, ParameterAttributes.Retval, "");
                SetMarshalAs(returnGen, returnParam);
            }

            // Generate method parameters
            for (int i = 0; i < parameters.Length - 1; i++) {
                var param = parameters[i + 1];
                var paramGen = invokeGen.DefineParameter(
                    i + 1, ParameterAttributes.None, null);
                if (param.MarshalAs != 0)
                    SetMarshalAs(paramGen, param);
                if (param.IsIn)
                    SetIn(paramGen);
                if (param.IsOut)
                    SetOut(paramGen);
            }

            return invokeGen;
        }

        /// <summary>
        /// [MarshalAs] attribute type
        /// </summary>
        private static Type MarshalAs { get; } = typeof(MarshalAsAttribute);

        /// <summary>
        /// [MarshalAs] attribute constructor
        /// </summary>
        private static ConstructorInfo MarshalAsCtor { get; }
            = MarshalAs.GetConstructor(new[] { typeof(UnmanagedType) });

        // Set parameter [MarshalAs] attribute
        /// <summary>
        /// Set the [MarshalAs] attribute for a generated parameter.
        /// </summary>
        /// <param name="paramGen">Generated parameter</param>
        /// <param name="parameter">Marshaling configuration of the parameter</param>
        private static void SetMarshalAs(ParameterBuilder paramGen, Parameter parameter)
        {
            CustomAttributeBuilder paramAttribute;
            if (parameter.MarshalAs == Parameter.ObjectRef) {
                paramAttribute = new CustomAttributeBuilder(
                    MarshalAsCtor, new object[] { UnmanagedType.CustomMarshaler },
                    new[]
                    {
                        MarshalAs.GetField("MarshalTypeRef"),
                        MarshalAs.GetField("MarshalCookie")
                    },
                    new object[]
                    {
                        typeof(ObjectMarshaler),
                        parameter.IsWeakRef ? "weak" : "normal"
                    });
            } else if (parameter.MarshalAs == UnmanagedType.CustomMarshaler) {
                paramAttribute = new CustomAttributeBuilder(
                    MarshalAsCtor, new object[] { UnmanagedType.CustomMarshaler },
                    new[]
                    {
                        MarshalAs.GetField("MarshalTypeRef"),
                        MarshalAs.GetField("MarshalCookie")
                    },
                    new object[]
                    {
                        Type.GetType(parameter.TypeName),
                        parameter.TypeName
                    });
            } else if (parameter.MarshalAs != 0) {
                if (parameter.IsArray) {
                    paramAttribute = new CustomAttributeBuilder(
                        MarshalAsCtor, new object[] { UnmanagedType.LPArray },
                        new[]
                        {
                            MarshalAs.GetField("ArraySubType"),
                            parameter.IsFixedLength
                                ? MarshalAs.GetField("SizeConst")
                                : MarshalAs.GetField("SizeParamIndex")
                        },
                        new object[]
                        {
                            parameter.MarshalAs,
                            parameter.ArrayLength
                        });
                } else {
                    paramAttribute = new CustomAttributeBuilder(
                        MarshalAsCtor, new object[] { parameter.MarshalAs });
                }
            } else {
                return;
            }
            paramGen.SetCustomAttribute(paramAttribute);
        }

        /// <summary>
        /// [In] attribute type
        /// </summary>
        private static Type In { get; } = typeof(InAttribute);

        /// <summary>
        /// [In] attribute constructor
        /// </summary>
        private static ConstructorInfo InCtor { get; }
            = In.GetConstructor(Array.Empty<Type>());

        /// <summary>
        /// Set parameter [In] attribute.
        /// </summary>
        /// <param name="paramGen">Marshaling configuration of the parameter</param>
        private static void SetIn(ParameterBuilder paramGen)
        {
            var inAttribute = new CustomAttributeBuilder(InCtor, Array.Empty<object>());
            paramGen.SetCustomAttribute(inAttribute);
        }

        /// <summary>
        /// [Out] attribute type
        /// </summary>
        private static Type Out { get; } = typeof(OutAttribute);

        /// <summary>
        /// [Out] attribute constructor
        /// </summary>
        private static ConstructorInfo OutCtor { get; }
            = Out.GetConstructor(Array.Empty<Type>());

        /// <summary>
        /// Set parameter [Out] attribute.
        /// </summary>
        /// <param name="paramGen">Marshaling configuration of the parameter</param>
        private static void SetOut(ParameterBuilder paramGen)
        {
            var outAttribute = new CustomAttributeBuilder(OutCtor, Array.Empty<object>());
            paramGen.SetCustomAttribute(outAttribute);
        }

        /// <summary>
        /// Generated assembly unique name.
        /// </summary>
        private static string UniqueAssemblyName { get; } = UniqueName("InteropDelegates");

        /// <summary>
        /// Dynamic assembly generator.
        /// </summary>
        private static AssemblyBuilder AssemblyGen { get; }
            = AssemblyBuilder.DefineDynamicAssembly(
                new AssemblyName(UniqueAssemblyName), AssemblyBuilderAccess.Run);

        /// <summary>
        /// Dynamic module generator.
        /// </summary>
        private static ModuleBuilder ModuleGen { get; }
            = AssemblyGen.DefineDynamicModule(UniqueAssemblyName);

        /// <summary>
        /// Delegate type cache.
        /// </summary>
        private static DelegateIndex DelegateTypes { get; } = new(new Indexer());

        /// <summary>
        /// Proxy method cache.
        /// </summary>
        private static ProxyIndex Proxies { get; } = new(new Indexer());

        /// <summary>
        /// Interface proxy type cache.
        /// </summary>
        private static ConcurrentDictionary<Type, Type> InterfaceProxyTypes { get; } = new();

        /// <summary>
        /// Cache custom indexer.
        /// </summary>
        private class Indexer : IIndexer
        {
            public bool Equals(
                (MethodBase method, Parameter[] parameters) x,
                (MethodBase method, Parameter[] parameters) y)
            {
                if (x.method != y.method)
                    return false;
                if (x.parameters.Length != y.parameters.Length)
                    return false;
                var xyParameters = x.parameters.Zip(y.parameters).ToList();
                if (xyParameters.Any(xy => xy.First.ParamInfo != xy.Second.ParamInfo))
                    return false;
                return xyParameters.All(xy => xy.First.TypeName == xy.Second.TypeName);
            }

            public int GetHashCode([DisallowNull] (MethodBase method, Parameter[] parameters) obj)
            {
                int hashCode = obj.method.GetHashCode();
                foreach (var parameter in obj.parameters)
                    hashCode = HashCode.Combine(hashCode, parameter.GetHashCode());
                return hashCode;
            }
        }

        /// <summary>
        /// Get a unique name, based on a concatenation of several parts and a random string.
        /// </summary>
        /// <remarks>
        /// Utility function.
        /// </remarks>
        /// <param name="nameParts">List of strings to concatenate</param>
        /// <returns>Unique name</returns>
        private static string UniqueName(params string[] nameParts)
        {
            return $"{string.Join("_", nameParts)}_{Path.GetRandomFileName().Replace(".", "")}";
        }
    }
}
