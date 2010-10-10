using System;
using NUnit.Framework;

namespace X10ExCom.Tests
{
    [TestFixture]
    public class X10ModuleStateRequestTests
    {
        [Test]
        public void VerifyThatCorrectlyCreatedMessagesResultInValidToStringOutput()
        {
            string expected = "R**";
            string actualUsingBytes = new X10ModuleStateRequest('*', 0).ToString();
            string actualUsingEnums = new X10ModuleStateRequest(X10House.X, X10Unit.X).ToString();
            Assert.AreEqual(expected, actualUsingBytes);
            Assert.AreEqual(expected, actualUsingEnums);
            for (var house = 'A'; house <= 'P'; house++ )
            {
                for(byte unit = 0; unit <= 16; unit++)
                {
                    expected = "R" + house + (unit == 0 ? "*" : (unit - 1).ToString("X"));
                    actualUsingBytes = new X10ModuleStateRequest(house, unit).ToString();
                    actualUsingEnums = new X10ModuleStateRequest((X10House)house, (X10Unit)(unit - 1)).ToString();
                    Assert.AreEqual(expected, actualUsingBytes);
                    Assert.AreEqual(expected, actualUsingEnums);
                }
            }
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidHouseValue()
        {
            new X10ModuleStateRequest('W', 1);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidUnitValue()
        {
            new X10ModuleStateRequest('A', 17);
        }
    }
}
