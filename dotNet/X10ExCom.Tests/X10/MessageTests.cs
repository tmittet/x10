using System;
using NUnit.Framework;
using X10ExCom.X10;

namespace X10ExCom.Tests.X10
{
    [TestFixture]
    public class X10MessageTests
    {
        [Test]
        public void VerifyThatWellFormedMessagesAreParsedToCorrectType()
        {
            Assert.IsInstanceOf(typeof(StandardMessage), Message.Parse("A__"));
            Assert.IsInstanceOf(typeof(StandardMessage), Message.Parse("A22"));
            Assert.IsInstanceOf(typeof(StandardMessage), Message.Parse("PFF"));
            Assert.IsInstanceOf(typeof(ExtendedMessage), Message.Parse("AB7x31x2E"));
            Assert.IsInstanceOf(typeof(ExtendedMessage), Message.Parse("A2Cx00x01"));
            Assert.IsInstanceOf(typeof(ExtendedMessage), Message.Parse("A07xFFx00"));
            Assert.IsInstanceOf(typeof(ScenarioExecute), Message.Parse("S00"));
            Assert.IsInstanceOf(typeof(ScenarioExecute), Message.Parse("SF0"));
            Assert.IsInstanceOf(typeof(ScenarioExecute), Message.Parse("SFF"));
            Assert.IsInstanceOf(typeof(ModuleStateRequest), Message.Parse("R**"));
            Assert.IsInstanceOf(typeof(ModuleStateRequest), Message.Parse("RC*"));
            Assert.IsInstanceOf(typeof(ModuleStateRequest), Message.Parse("RC9"));
            Assert.IsInstanceOf(typeof(ModuleStateWipe), Message.Parse("RW*"));
            Assert.IsInstanceOf(typeof(ModuleStateWipe), Message.Parse("RWA"));
            Assert.IsInstanceOf(typeof(ModuleStateWipe), Message.Parse("RWP"));
            Assert.IsInstanceOf(typeof(MessageError), Message.Parse("_ExTester"));
            Assert.IsInstanceOf(typeof(MessageError), Message.Parse("_Ex123"));
            Assert.IsInstanceOf(typeof(MessageError), Message.Parse("_ExLongErrorCodeTest"));
        }

        [Test]
        public void VerifyThatWellFormedMessagesSourceIsParsedCorrectly()
        {
            Assert.AreEqual(MessageSource.Unknown, Message.Parse("__:A00").Source);
            Assert.AreEqual(MessageSource.Parser, Message.Parse("XP:A00").Source);
            Assert.AreEqual(MessageSource.Serial, Message.Parse("SD:A00").Source);
            Assert.AreEqual(MessageSource.ModuleState, Message.Parse("MS:A00").Source);
            Assert.AreEqual(MessageSource.PowerLine, Message.Parse("PL:A00").Source);
            Assert.AreEqual(MessageSource.Radio, Message.Parse("RF:A00").Source);
            Assert.AreEqual(MessageSource.Infrared, Message.Parse("IR:A00").Source);
            Assert.AreEqual(MessageSource.Ethernet, Message.Parse("ER:A00").Source);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputIsNull()
        {
            Message.Parse(null);
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs1()
        {
            Message.Parse("A");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs2()
        {
            Message.Parse("A0");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs4()
        {
            Message.Parse("A07x");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs5()
        {
            Message.Parse("A07x3");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs6()
        {
            Message.Parse("A07x31");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs7()
        {
            Message.Parse("A07x31x");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIs8()
        {
            Message.Parse("A07x31x2");
        }

        [Test, ExpectedException(typeof(ArgumentException))]
        public void VerifyThatMessageCannotBeParsedWhenInputLengthIsTooLong()
        {
            Message.Parse("A07x31x2Ex");
        }
    }
}
