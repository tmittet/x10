using System;
using NUnit.Framework;

namespace X10ExCom.Tests
{
    [TestFixture]
    public class X10ModuleStateWipeTests
    {
        [Test]
        public void VerifyThatCorrectlyCreatedMessagesResultInValidToStringOutput()
        {
            string expected = "RW*";
            string actualUsingBytes = new X10ModuleStateWipe('*').ToString();
            string actualUsingEnums = new X10ModuleStateWipe(X10House.X).ToString();
            Assert.AreEqual(expected, actualUsingBytes);
            Assert.AreEqual(expected, actualUsingEnums);
            for (var house = 'A'; house <= 'P'; house++ )
            {
                expected = "RW" + house;
                actualUsingBytes = new X10ModuleStateWipe(house).ToString();
                actualUsingEnums = new X10ModuleStateWipe((X10House)house).ToString();
                Assert.AreEqual(expected, actualUsingBytes);
                Assert.AreEqual(expected, actualUsingEnums);
            }
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidHouseValue()
        {
            new X10ModuleStateWipe('W');
        }
    }
}
