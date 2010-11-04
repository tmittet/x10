using System;
using NUnit.Framework;

namespace X10ExCom.Tests
{
    [TestFixture]
    public class X10MessageTests
    {
        [Test]
        public void VerifyThatWellFormedMessagesAreParsedToCorrectType()
        {
            Assert.IsInstanceOf(typeof(X10StandardMessage), X10Message.Parse("A__"));
            Assert.IsInstanceOf(typeof(X10StandardMessage), X10Message.Parse("A22"));
            Assert.IsInstanceOf(typeof(X10StandardMessage), X10Message.Parse("PFF"));
            Assert.IsInstanceOf(typeof(X10ExtendedMessage), X10Message.Parse("AB7x31x2E"));
            Assert.IsInstanceOf(typeof(X10ExtendedMessage), X10Message.Parse("A2Cx00x01"));
            Assert.IsInstanceOf(typeof(X10ExtendedMessage), X10Message.Parse("A07xFFx00"));
            Assert.IsInstanceOf(typeof(X10ScenarioExecute), X10Message.Parse("S00"));
            Assert.IsInstanceOf(typeof(X10ScenarioExecute), X10Message.Parse("SF0"));
            Assert.IsInstanceOf(typeof(X10ScenarioExecute), X10Message.Parse("SFF"));
            Assert.IsInstanceOf(typeof(X10ModuleStateRequest), X10Message.Parse("R**"));
            Assert.IsInstanceOf(typeof(X10ModuleStateRequest), X10Message.Parse("RC*"));
            Assert.IsInstanceOf(typeof(X10ModuleStateRequest), X10Message.Parse("RC9"));
            Assert.IsInstanceOf(typeof(X10ModuleStateWipe), X10Message.Parse("RW*"));
            Assert.IsInstanceOf(typeof(X10ModuleStateWipe), X10Message.Parse("RWA"));
            Assert.IsInstanceOf(typeof(X10ModuleStateWipe), X10Message.Parse("RWP"));
            Assert.IsInstanceOf(typeof(X10Error), X10Message.Parse("_ExTester"));
            Assert.IsInstanceOf(typeof(X10Error), X10Message.Parse("_Ex123"));
            Assert.IsInstanceOf(typeof(X10Error), X10Message.Parse("_ExLongErrorCodeTest"));
        }

        [Test]
        public void VerifyThatWellFormedMessagesSourceIsParsedCorrectly()
        {
            Assert.AreEqual(X10MessageSource.Unknown, X10Message.Parse("__:A00").Source);
            Assert.AreEqual(X10MessageSource.Parser, X10Message.Parse("XP:A00").Source);
            Assert.AreEqual(X10MessageSource.Serial, X10Message.Parse("SD:A00").Source);
            Assert.AreEqual(X10MessageSource.ModuleState, X10Message.Parse("MS:A00").Source);
            Assert.AreEqual(X10MessageSource.PowerLine, X10Message.Parse("PL:A00").Source);
            Assert.AreEqual(X10MessageSource.Radio, X10Message.Parse("RF:A00").Source);
            Assert.AreEqual(X10MessageSource.Infrared, X10Message.Parse("IR:A00").Source);
            Assert.AreEqual(X10MessageSource.Ethernet, X10Message.Parse("ER:A00").Source);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputIsNull()
        {
            X10Message.Parse(null);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs1()
        {
            X10Message.Parse("A");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs2()
        {
            X10Message.Parse("A0");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs4()
        {
            X10Message.Parse("A07x");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs5()
        {
            X10Message.Parse("A07x3");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs6()
        {
            X10Message.Parse("A07x31");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs7()
        {
            X10Message.Parse("A07x31x");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs8()
        {
            X10Message.Parse("A07x31x2");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIsTooLong()
        {
            X10Message.Parse("A07x31x2Ex");
        }
    }
}
