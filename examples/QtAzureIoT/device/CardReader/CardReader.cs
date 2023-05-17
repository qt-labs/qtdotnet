/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Device.I2c;
using System.Diagnostics;
using Iot.Device.Pn532;
using Iot.Device.Pn532.ListPassive;
using Iot.Device.Pn532.RfConfiguration;
using QtAzureIoT.Utils;

namespace QtAzureIoT.Device
{
    public class CardReader : PropertySet, IDisposable
    {
        public CardReader()
        { }

        public bool CardInReader
        {
            get => propertyCardInReader;
            private set => SetProperty(ref propertyCardInReader, value, nameof(CardInReader));
        }

        public void StartPolling()
        {
            if (Nfc != null)
                return;
            BusConnectionSettings = new I2cConnectionSettings(1, 0x24);
            BusDevice = I2cDevice.Create(BusConnectionSettings);
            Nfc = new Pn532(BusDevice);

            PollingLoop = new CancellationTokenSource();
            Polling = new Task(async () => await PollingLoopAsync(), PollingLoop.Token);
            Polling.Start();
        }

        public void StopPolling()
        {
            if (Nfc == null)
                return;
            PollingLoop.Cancel();
            Polling.Wait();
            Nfc.Dispose();
            Nfc = null;
            BusDevice.Dispose();
            BusDevice = null;
            BusConnectionSettings = null;
            PollingLoop.Dispose();
            PollingLoop = null;
            Polling.Dispose();
            Polling = null;
        }

        #region private
        private I2cConnectionSettings BusConnectionSettings { get; set; }
        private I2cDevice BusDevice { get; set; }
        private Pn532 Nfc { get; set; }
        private CancellationTokenSource PollingLoop { get; set; }
        private Task Polling { get; set; }


        private async Task PollingLoopAsync()
        {
            TargetBaudRate cardType = TargetBaudRate.B106kbpsTypeA;
            while (!PollingLoop.IsCancellationRequested) {
                try {
                    if (Nfc.ListPassiveTarget(MaxTarget.One, cardType) is object) {
                        CardInReader = true;
                        var timeSinceDetected = Stopwatch.StartNew();
                        while (timeSinceDetected.ElapsedMilliseconds < 3000) {
                            if (Nfc.ListPassiveTarget(MaxTarget.One, cardType) is object)
                                timeSinceDetected.Restart();
                            await Task.Delay(200);
                        }
                        CardInReader = false;
                    } else {
                        Nfc.SetRfField(RfFieldMode.None);
                        await Task.Delay(1000);
                        Nfc.SetRfField(RfFieldMode.RF);
                    }
                } catch (Exception e) {
                    Debug.WriteLine($"Exception: {e.GetType().Name}: {e.Message}");
                    Nfc.SetRfField(RfFieldMode.None);
                }
            }
        }

        public void Dispose()
        {
            StopPolling();
        }

        private bool propertyCardInReader = false;
        #endregion
    }
}
