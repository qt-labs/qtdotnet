/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Reflection;
using System.Runtime.InteropServices;

namespace Qt.DotNet
{
    public partial class Adapter
    {
        internal static IntPtr GetRefPtrToObject(object obj, bool weakRef = false)
        {
            var objHandle = GCHandle.Alloc(obj, weakRef ? GCHandleType.Weak : GCHandleType.Normal);
            var objRefPtr = GCHandle.ToIntPtr(objHandle);
            ObjectRefs.TryAdd(objRefPtr, new ObjectRef(objHandle));
            return objRefPtr;
        }

        internal static ObjectRef GetObjectRefFromPtr(IntPtr objRefPtr)
        {
            if (objRefPtr == IntPtr.Zero)
                throw new ArgumentNullException(nameof(objRefPtr));
            if (!ObjectRefs.TryGetValue(objRefPtr, out var objRef))
                return null;
            if (!objRef.IsValid)
                throw new ObjectDisposedException(nameof(objRefPtr));
            return objRef;
        }

        /// <summary>
        /// Create a new reference (GCHandle) to the given object
        /// </summary>
        /// <param name="objRefPtr">Native reference to target object.</param>
        /// <param name="weakRef">'true' to create a weak ref.; 'false' otherwise (default)</param>
        /// <returns>Native object reference</returns>
        /// <exception cref="ArgumentException"></exception>
        public static IntPtr AddObjectRef(IntPtr objRefPtr, bool weakRef = false)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.AddObjectRef(AddObjectRef);
#endif
            var objRef = GetObjectRefFromPtr(objRefPtr);
            if (objRef == null)
                throw new ArgumentException("Invalid object reference", nameof(objRefPtr));
            var obj = objRef.Target;
            var objHandle = GCHandle.Alloc(
                obj, weakRef ? GCHandleType.Weak : GCHandleType.Normal);
            var newObjRefPtr = GCHandle.ToIntPtr(objHandle);
            ObjectRefs.TryAdd(newObjRefPtr, new ObjectRef(objHandle));
            return newObjRefPtr;
        }

        /// <summary>
        /// Release object reference, as well as any associated instance method and event references
        /// </summary>
        /// <param name="objRefPtr">Native reference to target object.</param>
        /// <returns>'true' if object ref. was released successfully; 'false' otherwise</returns>
        /// <exception cref="ArgumentException"></exception>
        public static void FreeObjectRef(IntPtr objRefPtr)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.FreeObjectRef(FreeObjectRef);
#endif
            if (!ObjectRefs.TryRemove(objRefPtr, out var objRef))
                throw new ArgumentException("Invalid object reference", nameof(objRefPtr));
            RemoveAllEventHandlers(objRef);

            var liveObjects = ObjectRefs.Values.Select(x => x.Target).ToList();
            var isLive = liveObjects.Any(x => x.Equals(objRef.Target));
            if (!isLive) {
                var deadMethods = DelegatesByMethod
                    .Where(x => x.Key.Target.Equals(objRef.Target))
                    .Select(x => x.Value.FuncPtr)
                    .ToList();
                deadMethods.ForEach(FreeDelegateRef);
            }

            objRef.Handle.Free();
        }

        /// <summary>
        /// Release reference to a type (i.e. instance of System.Type)
        /// </summary>
        /// <param name="typeName">Fully qualified name of type</param>
        /// <exception cref="ArgumentException"></exception>
        public static void FreeTypeRef(string typeName)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.FreeTypeRef(FreeTypeRef);
#endif
            var type = Type.GetType(typeName)
                ?? throw new ArgumentException($"Type '{typeName}' not found", nameof(typeName));

            var typeRefs = ObjectRefs
                .Where(x => x.Value.Target.Equals(type))
                .ToList();
            foreach (var typeRef in typeRefs)
                FreeObjectRef(typeRef.Key);

            var deadMethods = DelegatesByMethod
                .Where(x => x.Key.Target.Equals(type))
                .Select(x => x.Value.FuncPtr)
                .ToList();
            deadMethods.ForEach(FreeDelegateRef);
        }

        public static void FreeDelegateRef(IntPtr delRefPtr)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.FreeDelegateRef(FreeDelegateRef);
#endif
            if (!DelegateRefs.TryRemove(delRefPtr, out var delegateRef))
                return;
            DelegatesByMethod.TryRemove((delegateRef.Target, delegateRef.Method), out _);
            delegateRef.Ref.Handle.Free();
        }

        public static IntPtr GetObject(IntPtr objRefPtr, string path)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.GetObject(GetObject);
#endif
            var memberNames = path.Split('.',
                StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);

            var objRef = GetObjectRefFromPtr(objRefPtr);
            if (objRef == null)
                throw new ArgumentException("Invalid object reference", nameof(objRefPtr));

            var obj = objRef.Target;
            foreach (var memberName in memberNames) {
                if (obj?.GetType()?.GetMember(memberName) is not MemberInfo[] members)
                    throw new ArgumentException("Invalid object reference", memberName);
                foreach (var member in members) {
                    if (member is PropertyInfo prop) {
                        obj = prop.GetValue(obj);
                        break;
                    } else if (member is FieldInfo field) {
                        obj = field.GetValue(obj);
                        break;
                    }
                    throw new ArgumentException("Invalid object reference", memberName);
                }
            }
            return GetRefPtrToObject(obj);
        }
    }
}
