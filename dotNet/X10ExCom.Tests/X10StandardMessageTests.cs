using System;
using NUnit.Framework;

namespace X10ExCom.Tests
{
    [TestFixture]
    public class X10StandardMessageTests
    {
        [Test]
        public void VerifyThatCorrectlyCreatedMessagesResultInValidToStringOutput()
        {
            for (var house = 'A'; house <= 'P'; house++ )
            {
                for(byte unit = 0; unit <= 16; unit++)
                {
                    for(byte command = 0; command <= 16; command++)
                    {
                        string expected =
                            house + (unit == 0 ? "_" : (unit - 1).ToString("X")) +
                            (command == 0 ? "_" : (command - 1).ToString("X"));
                        string actualUsingBytes = new X10StandardMessage(house, unit, command).ToString();
                        string actualUsingEnums = new X10StandardMessage((X10House)house, (X10Unit)(unit - 1), (X10Command)(command - 1)).ToString();
                        Assert.AreEqual(expected, actualUsingBytes);
                        Assert.AreEqual(expected, actualUsingEnums);
                    }
                }
            }
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidHouseEnumValue()
        {
            new X10StandardMessage(X10House.X, X10Unit.U01, X10Command.On);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidHouseValue()
        {
            new X10StandardMessage('W', 1, 2);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidUnitValue()
        {
            new X10StandardMessage('A', 17, 2);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidCommandValue()
        {
            new X10StandardMessage('A', 1, 17);
        }
    }
}
