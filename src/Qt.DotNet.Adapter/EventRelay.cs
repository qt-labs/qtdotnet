/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Diagnostics;
using System.Reflection;

namespace Qt.DotNet
{
    /// <summary>
    /// Notify native subscribers of events raised by a given target object
    /// </summary>
    internal class EventRelay
    {
        public object Target { get; }
        public EventInfo Event { get;}
        public IntPtr Context { get; }

        public Adapter.Delegates.NativeEventHandler Handler { get; set; }

        public EventRelay(
            object target,
            EventInfo eventInfo,
            IntPtr context,
            Adapter.Delegates.NativeEventHandler eventHandler,
            bool enabled = true)
        {
            Target = target;
            Event = eventInfo;
            Handler = eventHandler;
            Context = context;
            RelayEventDelegate = null;
            Enabled = enabled;
#if TESTS || DEBUG
            Debug.Assert(Event.EventHandlerType != null, "Event.EventHandlerType is null");
#endif
        }

        private readonly object eventSync = new();

        public bool Enabled
        {
            get
            {
                lock (eventSync)
                    return RelayEventDelegate != null;
            }
            set
            {
                lock (eventSync) {
                    if (value && RelayEventDelegate == null) {
                        RelayEvent(null, null);
                        Event.AddEventHandler(Target, RelayEventDelegate);
                    } else if (!value && RelayEventDelegate != null) {
                        Event.RemoveEventHandler(Target, RelayEventDelegate);
                        RelayEventDelegate = null;
                    }
                }
            }
        }

        private Delegate RelayEventDelegate { get; set; }
        public void RelayEvent(object sender, EventArgs args)
        {
            if (sender == null && args == null) {
                var info = MethodBase.GetCurrentMethod() as MethodInfo;
#if TESTS || DEBUG
                Debug.Assert(info != null, nameof(info) + " != null");
                Debug.Assert(Event.EventHandlerType != null, "Event.EventHandlerType is null");
#endif
                RelayEventDelegate = Delegate.CreateDelegate(Event.EventHandlerType, this, info);
            } else {
                lock (eventSync)
                    Handler(Context, Event.Name, sender, args);
            }
        }
    }
}
