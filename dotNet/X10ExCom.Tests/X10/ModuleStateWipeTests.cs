using System;
using NUnit.Framework;
using X10ExCom.X10;

namespace X10ExCom.Tests.X10
{
    [TestFixture]
    public class X10ModuleStateWipeTests
    {
        [Test]
        public void VerifyThatCorrectlyCreatedMessagesResultInValidToStringOutput()
        {
            string expected = "RW*";
            string actualUsingBytes = new ModuleStateWipe('*').ToString();
            string actualUsingEnums = new ModuleStateWipe(House.X).ToString();
            Assert.AreEqual(expected, actualUsingBytes);
            Assert.AreEqual(expected, actualUsingEnums);
            for (var house = 'A'; house <= 'P'; house++ )
            {
                expected = "RW" + house;
                actualUsingBytes = new ModuleStateWipe(house).ToString();
                actualUsingEnums = new ModuleStateWipe((House)house).ToString();
                Assert.AreEqual(expected, actualUsingBytes);
                Assert.AreEqual(expected, actualUsingEnums);
            }
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidHouseValue()
        {
            new ModuleStateWipe('W');
        }
    }
}
