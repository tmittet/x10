using System;
using NUnit.Framework;
using X10ExCom.X10;

namespace X10ExCom.Tests.X10
{
    [TestFixture]
    public class X10ExtendedMessageTests
    {
        [Test]
        public void VerifyThatCorrectlyCreatedMessagesResultInValidToStringOutput()
        {
            Assert.AreEqual(
                "A07x00xFF",
                new ExtendedMessage(House.A, Unit.U01, Command.ExtendedCode, 0x00, 0xFF).ToString());
            Assert.AreEqual(
                "D37xFFx00",
                new ExtendedMessage(House.D, Unit.U04, Command.ExtendedCode, 0xFF, 0x00).ToString());
            Assert.AreEqual(
                "G67x01xAB",
                new ExtendedMessage(House.G, Unit.U07, Command.ExtendedCode, 0x01, 0xAB).ToString());
            Assert.AreEqual(
                "J9Cx40x10",
                new ExtendedMessage(House.J, Unit.U10, Command.ExtendedData, 0x40, 0x10).ToString());
            Assert.AreEqual(
                "MCCx00x3F",
                new ExtendedMessage(House.M, Unit.U13, Command.ExtendedData, 0x00, 0x3F).ToString());
            Assert.AreEqual(
                "PFCx7Dx00",
                new ExtendedMessage(House.P, Unit.U16, Command.ExtendedData, 0x7D, 0x00).ToString());
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithUnitX()
        {
            new ExtendedMessage(House.A, Unit.X, Command.ExtendedCode, 0x31, 0x1F);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithNonExtendedCommands()
        {
            new ExtendedMessage(House.A, Unit.U01, Command.On, 0x31, 0x1F);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithBothExtendedCommandAndDataSetToZero()
        {
            new ExtendedMessage(House.A, Unit.U01, Command.ExtendedCode, 0x00, 0x00);
        }
    }
}
