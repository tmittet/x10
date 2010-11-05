using System;
using NUnit.Framework;
using X10ExCom.X10;

namespace X10ExCom.Tests.X10
{
    [TestFixture]
    public class X10ScenarioExecuteTests
    {
        [Test]
        public void VerifyThatCorrectlyCreatedMessagesResultInValidToStringOutput()
        {
            for (int i = 0; i <= 255; i++)
            {
                string expected = "S" + i.ToString("X").PadLeft(2, '0');
                string actual = new ScenarioExecute((byte)i).ToString();
                Assert.AreEqual(expected, actual);
            }
        }
    }
}
