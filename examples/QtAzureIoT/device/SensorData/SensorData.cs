/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.Diagnostics;
using System.Device.I2c;
using Iot.Device.Bmxx80;
using Iot.Device.Bmxx80.PowerMode;
using QtAzureIoT.Utils;

namespace QtAzureIoT.Device
{
    public class SensorData : PropertySet, IDisposable
    {
        public SensorData()
        { }

        public double Temperature
        {
            get => propertyTemperature;
            private set => SetProperty(ref propertyTemperature, value, nameof(Temperature));
        }

        public double Pressure
        {
            get => propertyPressure;
            private set => SetProperty(ref propertyPressure, value, nameof(Pressure));
        }

        public double Humidity
        {
            get => propertyHumidity;
            private set => SetProperty(ref propertyHumidity, value, nameof(Humidity));
        }

        public void StartPolling()
        {
            if (Sensor != null)
                return;
            BusConnectionSettings = new I2cConnectionSettings(1, Bme280.DefaultI2cAddress);
            BusDevice = I2cDevice.Create(BusConnectionSettings);
            Sensor = new Bme280(BusDevice);
            MesasuramentDelay = Sensor.GetMeasurementDuration();

            PollingLoop = new CancellationTokenSource();
            Polling = new Task(async () => await PollingLoopAsync(), PollingLoop.Token);
            Polling.Start();
        }

        public void StopPolling()
        {
            if (Sensor == null)
                return;
            PollingLoop.Cancel();
            Polling.Wait();
            Sensor.Dispose();
            Sensor = null;
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
        private Bme280 Sensor { get; set; }
        int MesasuramentDelay { get; set; }
        private CancellationTokenSource PollingLoop { get; set; }
        private Task Polling { get; set; }


        private async Task PollingLoopAsync()
        {
            while (!PollingLoop.IsCancellationRequested) {
                try {
                    Sensor.SetPowerMode(Bmx280PowerMode.Forced);
                    await Task.Delay(MesasuramentDelay);

                    if (Sensor.TryReadTemperature(out var tempValue))
                        Temperature = tempValue.DegreesCelsius;
                    if (Sensor.TryReadPressure(out var preValue))
                        Pressure = preValue.Hectopascals;
                    if (Sensor.TryReadHumidity(out var humValue))
                        Humidity = humValue.Percent;
                } catch (Exception e) {
                    Debug.WriteLine($"Exception: {e.GetType().Name}: {e.Message}");
                }
                await Task.Delay(1000);
            }
        }

        public void Dispose()
        {
            StopPolling();
        }

        double propertyTemperature;
        double propertyPressure;
        double propertyHumidity;
        #endregion
    }
}
