/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Client.Samples;
using Microsoft.Azure.Devices.Provisioning.Client;
using Microsoft.Azure.Devices.Provisioning.Client.PlugAndPlay;
using Microsoft.Azure.Devices.Provisioning.Client.Transport;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Extensions.Logging;

namespace QtAzureIoT.Device
{
    public class Backoffice
    {
        public Backoffice()
        {
            DeviceClient = DeviceClient.CreateFromConnectionString(
                "HostName=QtDotNetDemo-Hub.azure-devices.net;DeviceId=QtDotNetDemoDevice;SharedAccessKey=YkZmsSOZf8lvQb5HDthosRHP4XV1hYSuDEoExe/2Fj8=",
                TransportType.Mqtt,
                new ClientOptions
                {
                    ModelId = "dtmi:com:example:TemperatureController;2"
                });
            BackofficeInterface = new TemperatureControllerSample(DeviceClient);
        }

        public void SetTelemetry(string name, double value)
        {
            BackofficeInterface.SetTelemetry(name, value);
        }

        public void SetTelemetry(string name, bool value)
        {
            BackofficeInterface.SetTelemetry(name, value);
        }

        public void StartPolling()
        {
            PollingLoop = new CancellationTokenSource();
            Polling = new Task(async () => await PollingLoopAsync(), PollingLoop.Token);
            Polling.Start();
        }

        public void StopPolling()
        {
            PollingLoop.Cancel();
        }

        #region private
        private CancellationTokenSource PollingLoop { get; set; }
        private Task Polling { get; set; }
        private DeviceClient DeviceClient { get; }
        private TemperatureControllerSample BackofficeInterface { get; }

        private async Task PollingLoopAsync()
        {
            await BackofficeInterface.InitOperationsAsync(PollingLoop.Token);
            while (!PollingLoop.IsCancellationRequested) {
                await BackofficeInterface.PerformOperationsAsync(PollingLoop.Token);
            }
        }
        #endregion
    }
}
