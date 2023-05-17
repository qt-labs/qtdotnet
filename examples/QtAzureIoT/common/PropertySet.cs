/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using System.ComponentModel;

namespace QtAzureIoT.Utils
{
    public class PropertySet : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void NotifyPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        protected void SetProperty<T>(ref T currentValue, T newValue, string name)
        {
            if (newValue.Equals(currentValue))
                return;
            currentValue = newValue;
            NotifyPropertyChanged(name);
        }
    }
}
