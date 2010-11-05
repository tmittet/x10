/************************************************************************/
/* X10 with Arduino .Net test application, v1.0.                        */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet thomas@mittet.nu November 2010.             */
/************************************************************************/

using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Runtime.Serialization.Json;
using System.Security;
using System.Text;
using System.Web;
using X10ExCom.X10;

namespace X10ExCom
{
    public class RestClient
    {
        private readonly Uri _uri;
        private readonly string _base64Auth;
        private readonly int _requestTimeout;
        private readonly Log _log;

        /// <summary>
        /// Creates new REST client.
        /// </summary>
        /// <param name="uri">Uri of Arduino board Ethernet controller.</param>
        /// <param name="userName">Basic Authentication user name.</param>
        /// <param name="password">Basic Authentication password.</param>
        /// <param name="requestTimeout"></param>
        /// <param name="logFilePath"></param>
        public RestClient(Uri uri, string userName, string password, int requestTimeout, string logFilePath = null)
        {
            _uri = uri;
            _base64Auth = Convert.ToBase64String(Encoding.ASCII.GetBytes(userName + ":" + password));
            _requestTimeout = requestTimeout;
            _log = new Log(GetType(), logFilePath);

            SetAllowUnsafeHeaderParsing();
        }

        #region Public Methods (GET)

        /// <summary>
        /// Gets data for specified module.
        /// </summary>
        /// <param name="house">House code.</param>
        /// <param name="unit">Unit code.</param>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns></returns>
        public Message GetModule(House house, Unit unit)
        {
            HttpWebRequest request = GetRequest(house, unit);
            HttpWebResponse response = GetHttpResponse(request);
            return GetX10Message(response);
        }

        /// <summary>
        /// Gets data for all modules or modules using specified house code.
        /// </summary>
        /// <param name="house">House code (optional).</param>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns></returns>
        public IEnumerable<Message> GetModules(House house = House.X)
        {
            HttpWebRequest request = GetRequest(house);
            HttpWebResponse response = GetHttpResponse(request);
            return GetX10Messages(response);
        }

        #endregion

        #region Public Methods (DELETE)

        /// <summary>
        /// Deletes state and info for specified module.
        /// </summary>
        /// <param name="house">House code.</param>
        /// <param name="unit">Unit code.</param>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns>True if delete was successful</returns>
        public bool DeleteModule(House house, Unit unit)
        {
            HttpWebRequest request = GetRequest(house, unit, "DELETE");
            HttpWebResponse response = GetHttpResponse(request);
            StandardMessage module = (StandardMessage)GetX10Message(response);
            return module.ModuleType == ModuleType.Unknown && String.IsNullOrEmpty(module.Name) && !module.On.HasValue;
        }

        /// <summary>
        /// Deletes state and info for all modules or modules using specified house code.
        /// </summary>
        /// <param name="house">House code (optional).</param>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns>True if delete was successful</returns>
        public bool DeleteModules(House house = House.X)
        {
            HttpWebRequest request = GetRequest(house, Unit.X, "DELETE");
            HttpWebResponse response = GetHttpResponse(request);
            return GetX10Messages(response).Count() == 0;
        }

        #endregion

        #region Public Methods (POST)

        /// <summary>
        /// Sets type of specified module.
        /// </summary>
        /// <param name="house">House code.</param>
        /// <param name="unit">Unit code.</param>
        /// <param name="moduleType">Module type to set.</param>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns>Data of module after update.</returns>
        public Message PostModule(House house, Unit unit, ModuleType moduleType)
        {
            NameValueCollection fields = new NameValueCollection { { "type", Convert.ToString((byte)moduleType) } };
            return PostModule(house, unit, fields);
        }

        /// <summary>
        /// Sets name of specified module.
        /// </summary>
        /// <param name="house">House code.</param>
        /// <param name="unit">Unit code.</param>
        /// <param name="name">Module name to set (max. 16 characters).</param>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns>Data of module after update.</returns>
        public Message PostModule(House house, Unit unit, string name)
        {
            NameValueCollection fields = new NameValueCollection(1);
            if(!String.IsNullOrEmpty(name)) fields.Add("name", "\"" + name.Trim().Substring(0, 16) + "\"");
            return PostModule(house, unit, fields);
        }

        /// <summary>
        /// Sets on state of specified module.
        /// </summary>
        /// <param name="house">House code.</param>
        /// <param name="unit">Unit code.</param>
        /// <param name="on">State to set.</param>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns>Data of module after update.</returns>
        public Message PostModule(House house, Unit unit, bool on)
        {
            NameValueCollection fields = new NameValueCollection { { "on", on ? "1" : "0" } };
            return PostModule(house, unit, fields);
        }

        /// <summary>
        /// Sets brightness of specified dimmer module.
        /// </summary>
        /// <param name="house">House code.</param>
        /// <param name="unit">Unit code.</param>
        /// <param name="brightness">Brightness to set (0-100).</param>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns>Data of module after update.</returns>
        public Message PostModule(House house, Unit unit, byte brightness)
        {
            NameValueCollection fields = new NameValueCollection { { "brightness", brightness.ToString() } };
            return PostModule(house, unit, fields);
        }

        /// <summary>
        /// Generic set method for the specified module.
        /// </summary>
        /// <param name="house">House code.</param>
        /// <param name="unit">Unit code.</param>
        /// <param name="fields">The fields to set (Please note: on and brightness can not be set at the same time).</param>
        /// <exception cref="ArgumentException">Thrown when on and brightness are used at the same time.</exception>
        /// <exception cref="ProtocolViolationException"></exception>
        /// <exception cref="WebException"></exception>
        /// <returns>Data of module after update.</returns>
        public Message PostModule(House house, Unit unit, NameValueCollection fields)
        {
            string onValue = fields.Get("on");
            string brightnessValue = fields.Get("brightness");
            // Check that fields don't contain both on and brightness
            if (!String.IsNullOrEmpty(onValue) && !String.IsNullOrEmpty(brightnessValue))
            {
                throw new ArgumentException("Fields on and brightness can not be set at the same time.");
            }
            // Make sure on or brightness is the last field in the request
            if (!String.IsNullOrEmpty(onValue))
            {
                fields.Remove("on");
                fields.Add("on", onValue);
            }
            if (!String.IsNullOrEmpty(brightnessValue))
            {
                fields.Remove("brightness");
                fields.Add("brightness", brightnessValue);
            }
            // Get HTTP request
            HttpWebRequest request = GetRequest(house, unit, "POST", fields);
            HttpWebResponse response = GetHttpResponse(request);
            return GetX10Message(response);
        }

        #endregion

        #region Private Methods

        private static void SetAllowUnsafeHeaderParsing()
        {
            Assembly assembly = Assembly.GetAssembly(typeof(System.Net.Configuration.SettingsSection));
            if (assembly != null)
            {
                Type settingsType = assembly.GetType("System.Net.Configuration.SettingsSectionInternal");
                if (settingsType != null)
                {
                    object invokeMember = settingsType.InvokeMember(
                        "Section",
                        BindingFlags.Static | BindingFlags.GetProperty | BindingFlags.NonPublic,
                        null, null, new object[] { });
                    if (invokeMember != null)
                    {
                        FieldInfo field = settingsType.GetField("useUnsafeHeaderParsing", BindingFlags.NonPublic | BindingFlags.Instance);
                        if (field != null)
                        {
                            field.SetValue(invokeMember, true);
                        }
                    }
                }
            }
        }

        private HttpWebRequest GetRequest(House house, Unit unit = Unit.X, string method = "GET", NameValueCollection postFields = null)
        {
            string address = _uri.AbsoluteUri.TrimEnd(new[] { ' ', '/' });
            if (house != House.X) address += "/" + house;
            if (unit != Unit.X) address += "/" + unit;
            HttpWebRequest request = null;
            try
            {
                request = (HttpWebRequest) WebRequest.Create(address + "/");
                request.Method = method.ToUpper();
                request.Headers.Add("Authorization", "Basic " + _base64Auth);
                request.PreAuthenticate = true;
                request.Timeout = _requestTimeout;
                if (postFields != null && postFields.Count > 0)
                {
                    string postData = "";
                    for (int i = 0; i < postFields.Count; i++)
                    {
                        postData += postFields.GetKey(i) + "=" + HttpUtility.UrlEncode(postFields[i]) + "&";
                    }
                    UTF8Encoding encoding = new UTF8Encoding();
                    byte[] bytes = encoding.GetBytes(postData.TrimEnd(new[] { '&' }));
                    request.ContentLength = bytes.Length;
                    using (Stream writeStream = request.GetRequestStream())
                    {
                        writeStream.Write(bytes, 0, bytes.Length);
                    }
                }
            }
            catch (ArgumentNullException e)
            {
                _log.Error(e.ToString());
            }
            catch (SecurityException e)
            {
                _log.Error(e.ToString());
            }
            catch (UriFormatException e)
            {
                _log.Error(e.ToString());
            }
            return request;
        }

        private HttpWebResponse GetHttpResponse(WebRequest request)
        {
            if (request != null)
            {
                try
                {
                    HttpWebResponse response = (HttpWebResponse)request.GetResponse();
                    if (response != null)
                    {
                        if (response.StatusCode == HttpStatusCode.OK)
                        {
                            return response;
                        }
                        _log.Error("Invalid response status: " + response.StatusCode);
                    }
                }
                catch (NotSupportedException e)
                {
                    _log.Error(e.ToString());
                }
            }
            return null;
        }

        private static Message GetX10Message(WebResponse response)
        {
            using (Stream stream = response != null ? response.GetResponseStream() : null)
            {
                if (stream != null)
                {
                    DataContractJsonSerializer jsSerializer = new DataContractJsonSerializer(typeof(ExtendedMessage));
                    ExtendedMessage module = jsSerializer.ReadObject(stream) as ExtendedMessage;
                    if (module != null)
                    {
                        if (module.ExtendedCommand == 0 && module.ExtendedData == 0)
                        {
                            return new StandardMessage(module.House, module.Unit, module.Command)
                            {
                                ModuleType = module.ModuleType,
                                Name = module.Name,
                                On = module.On,
                            };
                        }
                        return module;
                    }
                }
            }
            return null;
        }

        private static IEnumerable<Message> GetX10Messages(WebResponse response)
        {
            using (Stream stream = response != null ? response.GetResponseStream() : null)
            {
                if (stream != null)
                {
                    DataContractJsonSerializer jsSerializer = new DataContractJsonSerializer(typeof(Modules));
                    Modules modules = jsSerializer.ReadObject(stream) as Modules;
                    if (modules != null)
                    {
                        foreach (var module in modules.Module)
                        {
                            if (module.ExtendedCommand == 0 && module.ExtendedData == 0)
                            {
                                yield return new StandardMessage(module.House, module.Unit, module.Command)
                                {
                                    ModuleType = module.ModuleType,
                                    Name = module.Name,
                                    On = module.On,
                                };
                            }
                            else
                            {
                                yield return module;
                            }
                        }
                    }
                }
            }
        }

        #endregion
    }
}
