using System;
using NUnit.Framework;

namespace X10ExCom.Tests
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
                string actual = new X10ScenarioExecute((byte)i).ToString();
                Assert.AreEqual(expected, actual);
            }
        }
    }
}
