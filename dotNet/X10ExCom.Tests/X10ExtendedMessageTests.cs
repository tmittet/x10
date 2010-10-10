using System;
using NUnit.Framework;

namespace X10ExCom.Tests
{
    [TestFixture]
    public class X10ExtendedMessageTests
    {
        [Test]
        public void VerifyThatCorrectlyCreatedMessagesResultInValidToStringOutput()
        {
            Assert.AreEqual(
                "A07x00xFF",
                new X10ExtendedMessage(X10House.A, X10Unit.U01, X10Command.ExtendedCode, 0x00, 0xFF).ToString());
            Assert.AreEqual(
                "D37xFFx00",
                new X10ExtendedMessage(X10House.D, X10Unit.U04, X10Command.ExtendedCode, 0xFF, 0x00).ToString());
            Assert.AreEqual(
                "G67x01xAB",
                new X10ExtendedMessage(X10House.G, X10Unit.U07, X10Command.ExtendedCode, 0x01, 0xAB).ToString());
            Assert.AreEqual(
                "J9Cx40x10",
                new X10ExtendedMessage(X10House.J, X10Unit.U10, X10Command.ExtendedData, 0x40, 0x10).ToString());
            Assert.AreEqual(
                "MCCx00x3F",
                new X10ExtendedMessage(X10House.M, X10Unit.U13, X10Command.ExtendedData, 0x00, 0x3F).ToString());
            Assert.AreEqual(
                "PFCx7Dx00",
                new X10ExtendedMessage(X10House.P, X10Unit.U16, X10Command.ExtendedData, 0x7D, 0x00).ToString());
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithUnitX()
        {
            new X10ExtendedMessage(X10House.A, X10Unit.X, X10Command.ExtendedCode, 0x31, 0x1F);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithNonExtendedCommands()
        {
            new X10ExtendedMessage(X10House.A, X10Unit.U01, X10Command.On, 0x31, 0x1F);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeCreatedWithBothExtendedCommandAndDataSetToZero()
        {
            new X10ExtendedMessage(X10House.A, X10Unit.U01, X10Command.ExtendedCode, 0x00, 0x00);
        }
    }
}
