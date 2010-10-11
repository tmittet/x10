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
/* Written by Thomas Mittet thomas@mittet.nu October 2010.              */
/************************************************************************/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO.Ports;
using System.Text;
using System.Threading;

namespace X10ExCom
{
    public delegate void MessageReceivedHandler(object source, X10Message message);

    public class Serial : IDisposable
    {
        private readonly StringBuilder _serialBuffer;
        private readonly Log _log;
        private readonly ISynchronizeInvoke _syncObject;

        #region Public Properties and Events

        public SerialPort SerialPort { get; private set; }

        /// <summary>
        /// Is invoked when X10ex message is received from Arduino controller.
        /// </summary>
        public event MessageReceivedHandler MessageReceived;

        #endregion

        #region Constructor and Public Methods

        /// <summary>
        /// Opens serial connection with and inits Arduino running X10ex.
        /// Serial class is IDisposable, encolse in using section or call dispose to close connection.
        /// </summary>
        /// <param name="baudRate">Baudrate: 9600, 115200 e.g.</param>
        /// <param name="logFilePath">Full log file path, "C:\X10ExSerial.log" e.g. When set to null or empty log messages will be written to console.</param>
        public Serial(int baudRate, string logFilePath)
            : this(null, baudRate, logFilePath, null)
        { }

        /// <summary>
        /// Opens serial connection with and inits Arduino running X10ex.
        /// Serial class is IDisposable, encolse in using section or call dispose to close connection.
        /// </summary>
        /// <param name="portHint">Tells library what port to scan first, speeds up discovery process.</param>
        /// <param name="baudRate">Baudrate: 9600, 115200 e.g.</param>
        /// <param name="logFilePath">Full log file path, "C:\X10ExSerial.log" e.g. When set to null or empty log messages will be written to console.</param>
        public Serial(string portHint, int baudRate, string logFilePath)
            : this(portHint, baudRate, logFilePath, null)
        { }

        /// <summary>
        /// Opens serial connection with and inits Arduino running X10ex.
        /// Serial class is IDisposable, encolse in using section or call dispose to close connection.
        /// </summary>
        /// <param name="baudRate">Baudrate: 9600, 115200 e.g.</param>
        /// <param name="logFilePath">Full log file path, "C:\X10ExSerial.log" e.g. When set to null or empty log messages will be written to console.</param>
        /// <param name="syncObject">Synchronization object. Use this to make events fire on UI thread e.g.</param>
        public Serial(int baudRate, string logFilePath, ISynchronizeInvoke syncObject)
            : this(null, baudRate, logFilePath, syncObject)
        { }

        /// <summary>
        /// Opens serial connection with and inits Arduino running X10ex.
        /// Serial class is IDisposable, encolse in using section or call dispose to close connection.
        /// </summary>
        /// <param name="portHint">Tells library what port to scan first, speeds up discovery process.</param>
        /// <param name="baudRate">Baudrate: 9600, 115200 e.g.</param>
        /// <param name="logFilePath">Full log file path, "C:\X10ExSerial.log" e.g. When set to null or empty log messages will be written to console.</param>
        /// <param name="syncObject">Synchronization object. Use this to make events fire on UI thread e.g.</param>
        public Serial(string portHint, int baudRate, string logFilePath, ISynchronizeInvoke syncObject)
        {
            _serialBuffer = new StringBuilder();
            _log = new Log(GetType(), logFilePath);
            _syncObject = syncObject;
            // Get list of available serial port names and sort alphabetically
            List<string> portNames = new List<string>();
            portNames.AddRange(SerialPort.GetPortNames());
            portNames.Sort();
            // If porthint is specified, make sure it's the first entry in the list of ports
            if (!String.IsNullOrEmpty(portHint))
            {
                portHint = portHint.ToUpper();
                portNames.Remove(portHint);
                portNames.Insert(0, portHint);
            }
            _log.Info("Scanning serial ports for Arduino running X10ex.");
            foreach (string port in portNames)
            {
                SerialPort = new SerialPort(port, baudRate)
                {
                    DtrEnable = true,
                    Encoding = Encoding.ASCII
                };
                try
                {
                    SerialPort.Open();
                    int timeoutMs = 2000;
                    while (timeoutMs > 0 && !SerialPort.IsOpen)
                    {
                        Thread.Sleep(100);
                        timeoutMs -= 100;
                    }
                    while (timeoutMs > 0 && !_serialBuffer.ToString().Contains("X10"))
                    {
                        Thread.Sleep(100);
                        timeoutMs -= 100;
                        _serialBuffer.Append(SerialPort.ReadExisting());
                    }
                    if (_serialBuffer.ToString().Contains("X10"))
                    {
                        break;
                    }
                    CloseConnection();
                }
                catch (System.IO.IOException ex)
                {
                    CloseConnection();
                    _log.Debug(ex.Message);
                }
                catch (UnauthorizedAccessException ex)
                {
                    CloseConnection();
                    _log.Debug("Serial port \"" + port + "\" is already in use. Error message: " + ex.Message);
                }
                _serialBuffer.Clear();
            }
            if (SerialPort == null)
            {
                throw new Exception("Could not find Arduino running X10ex on any available serial port.");
            }
            SerialPort.DataReceived += DataReceived;
            SerialPort.ErrorReceived += ErrorReceived;
            SerialPort.DiscardInBuffer();
            _serialBuffer.Clear();
            _log.Info("Serial connection with Arduino running X10ex established on port \"" + SerialPort.PortName + "\".");
        }

        public void SendMessage(X10Message message)
        {
            byte[] bytes = Encoding.ASCII.GetBytes(message.ToString());
            SerialPort.Write(bytes, 0, bytes.Length);
        }

        /// <summary>
        /// Close serial connection and dispose Serial class.
        /// </summary>
        public void Dispose()
        {
            try
            {
                _log.Info("Closing serial connection and disposing Serial class.");
                CloseConnection();
            }
            catch (Exception ex)
            {
                _log.Error("An error occurred when disposing Serial class. Error message: " + ex.Message);
            }
        }

        #endregion

        #region Private Methods

        private void DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            _serialBuffer.Append(SerialPort.ReadExisting());
            string messageData = _serialBuffer.ToString();
            int lastMessageEnd = messageData.LastIndexOf('\n');
            if (lastMessageEnd > -1)
            {
                messageData = messageData.Substring(0, lastMessageEnd);
                _serialBuffer.Remove(0, lastMessageEnd);
                string[] messages = messageData.Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
                foreach (string message in messages)
                {
                    string messageTrimmed = message.Trim(new[] {' ', '\r', '\n'});
                    X10Message x10Message;
                    try
                    {
                        x10Message = X10Message.Parse(messageTrimmed);
                        _log.Debug(x10Message.GetType().Name + " parsed successfully: " + messageTrimmed);
                    }
                    catch (Exception ex)
                    {
                        string errorMessage = ex.GetType().Name + " thrown when parsing message \"" + messageTrimmed + "\". " + ex.Message;
                        _log.Warn(errorMessage);
                        x10Message = new X10Error(X10MessageSource.Parser, "_ExParser", errorMessage);
                    }
                    InvokeMessageReceived(x10Message);
                }
            }
        }

        private void ErrorReceived(object sender, SerialErrorReceivedEventArgs e)
        {
            _log.Error("Serial Port Error Received. Type: " + e.EventType);
        }

        private void InvokeMessageReceived(X10Message message)
        {
            if (MessageReceived != null)
            {
                _log.Debug("Invoking MessageReceived Event.");
                if (_syncObject != null)
                {
                    _syncObject.Invoke(MessageReceived, new object[] { this, message });
                }
                else
                {
                    MessageReceived.Invoke(this, message);
                }
            }
        }

        private void CloseConnection()
        {
            if (SerialPort != null)
            {
                if (SerialPort.IsOpen)
                {
                    SerialPort.Close();
                }
                SerialPort = null;
            }
        }

        #endregion
    }
}
