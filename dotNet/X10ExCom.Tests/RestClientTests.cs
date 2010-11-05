using System;
using System.Collections.Specialized;
using NUnit.Framework;
using X10ExCom.X10;

namespace X10ExCom.Tests
{
    [TestFixture]
    public class RestClientTests
    {
        private const string URI = "http://arduino_ip/";
        private const string USERNAME = "test";
        private const string PASSWORD = "test";

        [Test]
        [Ignore]
        public void GetOneModule()
        {
            RestClient restClient = new RestClient(new Uri(URI), USERNAME, PASSWORD, 6000);
            const House house = House.P;
            const Unit unit = Unit.U16;
            StandardMessage module = (StandardMessage)restClient.GetModule(house, unit);
            Assert.AreEqual(house, module.House);
            Assert.AreEqual(unit, module.Unit);
        }

        [Test]
        [Ignore]
        public void GetAllModules()
        {
            RestClient restClient = new RestClient(new Uri(URI), USERNAME, PASSWORD, 6000);
            foreach (var module in restClient.GetModules())
            {
                Console.WriteLine(module.ToHumanReadableString());
            }
        }

        [Test]
        [Ignore]
        public void GetHouseModules()
        {
            RestClient restClient = new RestClient(new Uri(URI), USERNAME, PASSWORD, 6000);
            foreach (var module in restClient.GetModules(House.A))
            {
                Console.WriteLine(module.ToHumanReadableString());
            }
        }

        [Test]
        [Ignore]
        public void CreateNewModuleP16AndThenDeleteIt()
        {
            RestClient restClient = new RestClient(new Uri(URI), USERNAME, PASSWORD, 6000);
            const House house = House.P;
            const Unit unit = Unit.U16;
            // Create
            const ModuleType expectedType = ModuleType.Dimmer;
            const string expectedName = "Test";
            const bool expectedOnState = true;
            NameValueCollection fields = new NameValueCollection
            {
                {"on", expectedOnState ? "1" : "0"},
                {"type", Convert.ToString((byte) expectedType)},
                {"name", "\"Test\""}
            };
            StandardMessage x10Message = (StandardMessage)restClient.PostModule(house, unit, fields);
            Assert.AreEqual(house, x10Message.House);
            Assert.AreEqual(unit, x10Message.Unit);
            Assert.AreEqual(expectedType, x10Message.ModuleType);
            Assert.AreEqual(expectedName, x10Message.Name);
            Assert.AreEqual(expectedOnState, x10Message.On);
            // Delete
            Assert.IsTrue(restClient.DeleteModule(house, unit));
        }
    }
}
