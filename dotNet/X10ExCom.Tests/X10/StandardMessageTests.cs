using System;
using NUnit.Framework;
using X10ExCom.X10;

namespace X10ExCom.Tests.X10
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
                        string actualUsingBytes = new StandardMessage(house, unit, command).ToString();
                        string actualUsingEnums = new StandardMessage((House)house, (Unit)(unit - 1), (Command)(command - 1)).ToString();
                        Assert.AreEqual(expected, actualUsingBytes);
                        Assert.AreEqual(expected, actualUsingEnums);
                    }
                }
            }
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidHouseEnumValue()
        {
            new StandardMessage(House.X, Unit.U01, Command.On);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidHouseValue()
        {
            new StandardMessage('W', 1, 2);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidUnitValue()
        {
            new StandardMessage('A', 17, 2);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithInvalidCommandValue()
        {
            new StandardMessage('A', 1, 17);
        }
    }
}
