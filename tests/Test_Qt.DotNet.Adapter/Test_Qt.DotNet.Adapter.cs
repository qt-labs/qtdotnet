/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Qt.DotNet.Test
{
    [TestClass]
    public class Test_QDotNetAdapter
    {
        [TestMethod]
        public void BuiltInTest()
        {
#if DEBUG || TESTS
            Assert.IsTrue(Adapter.Test());
#endif
        }
    }
}
