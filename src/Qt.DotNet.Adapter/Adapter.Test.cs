/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Qt.DotNet
{
    public partial class Adapter
    {
#if DEBUG || TESTS
        /// <summary>
        /// Get current ref counters. For debug/test purposes.
        /// </summary>
        /// <param name="refCount">Object ref. count</param>
        /// <param name="staticCount">Static method ref. count</param>
        /// <param name="eventCount">Event ref. count</param>
        public static void Stats(out int refCount, out int staticCount, out int eventCount)
        {
            refCount = ObjectRefs.Count;
            staticCount = DelegateRefs.Count;
            eventCount = Events.Count;
        }

        /// <summary>
        /// Built-in test
        /// </summary>
        /// <returns></returns>
        public static bool Test()
        {
            // TO-DO: revise this

            LoadAssembly("FooLib.dll");

            var ctorPtr = ResolveConstructor(1, new[] { new Parameter("FooLib.Foo, FooLib") });
            var ctorPtr2 = ResolveConstructor(1, new[] { new Parameter("FooLib.Foo, FooLib") });

            var ctor = GetMethod(ctorPtr) as ConstructorInfo;
            Debug.Assert(ctor != null, nameof(ctor) + " is null");
            var objRef = GetRefPtrToObject(ctor.Invoke(Array.Empty<object>()));

            var getTypePtr = ResolveInstanceMethod(
                objRef, "GetType", 1, new[] { new Parameter("System.Type") });
            var getBarPtr = ResolveInstanceMethod(
                objRef, "get_Bar", 1, new[] { new Parameter(UnmanagedType.LPWStr) });
            var setBarPtr = ResolveInstanceMethod(
                objRef, "set_Bar", 1, new[] { new(), new Parameter(UnmanagedType.LPWStr) });

            AddEventHandler(
                objRef,
                "PropertyChanged",
                new IntPtr(42),
                TestNativeEventHandler);

            for (int i = 0; i < 1000; ++i) {
                var str = GetMethod(getBarPtr)
                    .Invoke(GetObjectRefFromPtr(objRef).Target, Array.Empty<object>()) as string;
                str += "hello";
                GetMethod(setBarPtr)
                    .Invoke(GetObjectRefFromPtr(objRef).Target, new object[] { str });
            }

            RemoveAllEventHandlers(objRef);
            FreeObjectRef(objRef);
            FreeTypeRef("FooLib.Foo, FooLib");

            bool ok = Events.IsEmpty;
            ok = ok && ObjectRefs.IsEmpty;
            ok = ok && DelegateRefs.IsEmpty;
            return ok;
        }

        private static void TestNativeEventHandler(
                    IntPtr context,
                    string eventName,
                    object senderObj,
                    object argsObj)
        {
            var senderRef = GetRefPtrToObject(senderObj);
            var argsRef = GetRefPtrToObject(argsObj);

            var getTypePtr = ResolveInstanceMethod(
                argsRef, "GetType", 1, new[] { new Parameter("System.Type") });
            if (eventName == "PropertyChanged") {
                var typeObj = GetMethod(getTypePtr)
                    .Invoke(GetObjectRefFromPtr(argsRef).Target, Array.Empty<object>());
                var typeRef = GetRefPtrToObject(typeObj);

                var getFullNamePtr = ResolveInstanceMethod(
                    typeRef, "get_FullName", 1, new[] { new Parameter(UnmanagedType.LPWStr) });
                var argsTypeName = GetMethod(getFullNamePtr)
                    .Invoke(GetObjectRefFromPtr(typeRef).Target, Array.Empty<object>())
                    as string;

                if (argsTypeName == "System.ComponentModel.PropertyChangedEventArgs") {

                    var propChangeRef = AddObjectRef(argsRef);

                    var getPropertyNamePtr = ResolveInstanceMethod(
                        propChangeRef, "get_PropertyName", 1,
                        new[]
                        {
                            new Parameter(UnmanagedType.LPWStr)
                        });
                    var propName = GetMethod(getPropertyNamePtr)
                        .Invoke(GetObjectRefFromPtr(propChangeRef).Target, Array.Empty<object>())
                        as string;

                    if (propName == "Bar") {
                        var getBarPtr = ResolveInstanceMethod(
                            senderRef, "get_Bar", 1, new[] { new Parameter(UnmanagedType.LPWStr) });
                        var str = GetMethod(getBarPtr)
                            .Invoke(GetObjectRefFromPtr(senderRef).Target, Array.Empty<object>())
                            as string;
                        Debug.Assert(str != null, nameof(str) + " is null");
                        Console.WriteLine($"BAR CHANGED!!! [{str.Length / "hello".Length}x hello]");
                    }
                    FreeObjectRef(typeRef);
                    FreeObjectRef(propChangeRef);
                }
            }
            FreeObjectRef(argsRef);
            FreeObjectRef(senderRef);
        }

        private static MethodBase GetMethod(IntPtr funcPtr)
        {
            var methods = DelegateRefs
                .Where(x => x.Value.Ref.FuncPtr == funcPtr)
                .Select(x => x.Value.Method);
            return methods.First();

        }
#endif
    }
}
