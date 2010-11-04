using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace X10ExCom.Tests
{
    [TestFixture]
    public class X10ErrorTests
    {
        [Test]
        public void VerifyThatMessageTypesAndCodesMapToErrorMessage()
        {
            X10Error unknownError = new X10Error(X10MessageSource.Unknown, null);
            List<string> errorCodes = new List<string>(new[] { "Buffer", "Syntax", "TimOut", "NoAuth", "Method" });
            foreach (X10MessageSource source in Enum.GetNames(typeof (X10MessageSource)).Select(name => (X10MessageSource) Enum.Parse(typeof (X10MessageSource), name)))
            {
                foreach (X10Error error in errorCodes.Select(code => new X10Error(source, code)))
                {
                    Assert.IsNotNullOrEmpty(error.Message);
                    Assert.AreNotEqual(unknownError.Message, error.Message);
                }
            }
        }
    }
}
