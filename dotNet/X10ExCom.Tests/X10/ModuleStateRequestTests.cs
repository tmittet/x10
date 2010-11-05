using System;
using NUnit.Framework;
using X10ExCom.X10;

namespace X10ExCom.Tests.X10
{
    [TestFixture]
    public class X10ModuleStateRequestTests
    {
        [Test]
        public void VerifyThatCorrectlyCreatedMessagesResultInValidToStringOutput()
        {
            string expected = "R**";
            string actualUsingBytes = new ModuleStateRequest('*', 0).ToString();
            string actualUsingEnums = new ModuleStateRequest(House.X, Unit.X).ToString();
            Assert.AreEqual(expected, actualUsingBytes);
            Assert.AreEqual(expected, actualUsingEnums);
            for (var house = 'A'; house <= 'P'; house++ )
            {
                for(byte unit = 0; unit <= 16; unit++)
                {
                    expected = "R" + house + (unit == 0 ? "*" : (unit - 1).ToString("X"));
                    actualUsingBytes = new ModuleStateRequest(house, unit).ToString();
                    actualUsingEnums = new ModuleStateRequest((House)house, (Unit)(unit - 1)).ToString();
                    Assert.AreEqual(expected, actualUsingBytes);
                    Assert.AreEqual(expected, actualUsingEnums);
                }
            }
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidHouseValue()
        {
            new ModuleStateRequest('W', 1);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidUnitValue()
        {
            new ModuleStateRequest('A', 17);
        }
    }
}
