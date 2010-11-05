using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using X10ExCom.X10;

namespace X10ExCom.Tests.X10
{
    [TestFixture]
    public class X10MessageErrorTests
    {
        [Test]
        public void VerifyThatMessageTypesAndCodesMapToErrorMessage()
        {
            MessageError unknownMessageError = new MessageError(MessageSource.Unknown, null);
            List<string> errorCodes = new List<string>(new[] { "Buffer", "Syntax", "TimOut", "NoAuth", "Method" });
            foreach (MessageSource source in Enum.GetNames(typeof (MessageSource)).Select(name => (MessageSource) Enum.Parse(typeof (MessageSource), name)))
            {
                foreach (MessageError error in errorCodes.Select(code => new MessageError(source, code)))
                {
                    Assert.IsNotNullOrEmpty(error.Message);
                    Assert.AreNotEqual(unknownMessageError.Message, error.Message);
                }
            }
        }
    }
}
