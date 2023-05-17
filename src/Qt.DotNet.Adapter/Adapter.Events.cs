/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

namespace Qt.DotNet
{
    public partial class Adapter
    {
        /// <summary>
        /// Subscribe to notifications of given event from given object
        /// </summary>
        /// <param name="objRefPtr">Native reference to target object.</param>
        /// <param name="eventName">Name of event</param>
        /// <param name="context">Opaque pointer to context data</param>
        /// <param name="eventHandler">Pointer to callback function</param>
        /// <exception cref="ArgumentException"></exception>
        public static void AddEventHandler(
            IntPtr objRefPtr,
            string eventName,
            IntPtr context,
            Delegates.NativeEventHandler eventHandler)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.AddEventHandler(AddEventHandler);
#endif
            var objRef = GetObjectRefFromPtr(objRefPtr);
            if (objRef == null)
                throw new ArgumentException("Invalid object reference", nameof(objRefPtr));
            var eventInfo = objRef.Target.GetType().GetEvent(eventName)
                ?? throw new ArgumentException($"Event '{eventName}' not found", nameof(eventName));

            if (Events.TryGetValue((objRef, eventName, context), out var eventRelay)) {
                eventRelay.Handler = eventHandler;
            } else {
                eventRelay = new EventRelay(objRef.Target, eventInfo, context, eventHandler);
                Events.TryAdd((objRef, eventName, context), eventRelay);
            }
        }

        /// <summary>
        /// Unsubscribe to notifications of given event from given object
        /// </summary>
        /// <param name="objRefPtr">Native reference to target object.</param>
        /// <param name="eventName">Name of event</param>
        /// <param name="context">Opaque pointer to context data</param>
        /// <exception cref="ArgumentException"></exception>
        public static void RemoveEventHandler(
            IntPtr objRefPtr,
            string eventName,
            IntPtr context)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.RemoveEventHandler(RemoveEventHandler);
#endif
            var objRef = GetObjectRefFromPtr(objRefPtr);
            if (objRef == null)
                throw new ArgumentException("Invalid object reference", nameof(objRefPtr));
            RemoveEventHandler(objRef, eventName, context);
        }

        private static void RemoveEventHandler(
            ObjectRef objRef,
            string eventName,
            IntPtr context)
        {
            if (Events.TryRemove((objRef, eventName, context), out var evRelay))
                evRelay.Enabled = false;
        }

        /// <summary>
        /// Unsubscribe to notifications of all events from given object
        /// </summary>
        /// <param name="objRefPtr">Native reference to target object.</param>
        /// <exception cref="ArgumentException"></exception>
        public static void RemoveAllEventHandlers(IntPtr objRefPtr)
        {
#if DEBUG
            // Compile-time signature check of delegate vs. method
            _ = new Delegates.RemoveAllEventHandlers(RemoveAllEventHandlers);
#endif
            var objRef = GetObjectRefFromPtr(objRefPtr);
            if (objRef == null)
                throw new ArgumentException("Invalid object reference", nameof(objRefPtr));
            RemoveAllEventHandlers(objRef);
        }

        private static void RemoveAllEventHandlers(ObjectRef objRef)
        {
            var evHandlers = Events.Where(x => x.Key.Source == objRef);
            foreach (var evHandler in evHandlers)
                RemoveEventHandler(objRef, evHandler.Key.Name, evHandler.Key.Context);
        }
    }
}
