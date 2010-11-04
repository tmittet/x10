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
using System.Runtime.Serialization;

namespace X10ExCom
{
    [Serializable]
    [DataContract]
    public abstract class X10Message
    {
        public X10MessageSource Source { get; internal set; }
        public string SourceString { get; private set; }

        #region Public Methods

        public static X10Message Parse(string message)
        {
            if (String.IsNullOrEmpty(message))
            {
                throw new ArgumentException("Message can not be null or empty.");
            }
            if (message.Length < 3)
            {
                throw new ArgumentException("Message must be at least 3 characters long.");
            }
            message = message.Trim();
            string source = "";
            if (message.Substring(2, 1) == ":")
            {
                source = message.Substring(0, 2).ToUpper();
                message = message.Substring(3);
            }
            X10Message x10Msg;
            if (message.Length == 3 && message.ToUpper() == "X10")
            {
                x10Msg = new X10Boot();
            }
            else if (message.Length >= 3 && message.Substring(0, 3).ToUpper() == "_EX")
            {
                x10Msg = new X10Error(ParseSource(source), message.Substring(3));
            }
            else
            {
                message = message.ToUpper();
                if (message.Length != 3 && message.Length != 9)
                {
                    throw new ArgumentException(
                        message.Length + " characters is an invalid message length. " +
                        "Valid messages are 3 or 9 characters long.");
                }
                // Standard or Extended
                if ((message[0] >= 'A' && message[0] <= 'P') || message[0] == '*')
                {
                    x10Msg = ParseStandardAndExtended(message);
                }
                else
                {
                    if (message.Length != 3)
                    {
                        throw new ArgumentException(
                            "9 characters is an invalid message length for this type. " +
                            "Only extended code messages starting with house code A-P can be this length.");
                    }
                    // Scenario Execute
                    if (message[0] == 'S')
                    {
                        x10Msg = ParseScenario(message);
                    }
                    // Module State
                    else if (message[0] == 'R')
                    {
                        x10Msg = ParseModuleState(message);
                    }
                    else
                    {
                        throw new ArgumentException(
                            message[0] + " is an invalid house/type character. " +
                            "Valid characters are A-P, S or R.");
                    }
                }
            }
            x10Msg.SourceString = source;
            x10Msg.Source = ParseSource(source);
            return x10Msg;
        }

        internal static X10MessageSource ParseSource(string source)
        {
            source = source ?? "";
            switch (source.TrimStart().TrimEnd(new [] { ' ', ':' }).ToUpper())
            {
                case "XP": return X10MessageSource.Parser;
                case "SD": return X10MessageSource.Serial;
                case "MS": return X10MessageSource.ModuleState;
                case "PL": return X10MessageSource.PowerLine;
                case "RF": return X10MessageSource.Radio;
                case "IR": return X10MessageSource.Infrared;
                case "ER": return X10MessageSource.Ethernet;
                default: return X10MessageSource.Unknown;
            }
        }

        public abstract override string ToString();

        public abstract string ToHumanReadableString();

        #endregion

        #region Private and Internal Methods

        private static X10Message ParseStandardAndExtended(string message)
        {
            if (message.Length == 3)
            {
                return new X10StandardMessage(message[0], HexToNibble(message[1]), HexToNibble(message[2]));
            }
            else
            {
                return new X10ExtendedMessage(
                    message[0], HexToNibble(message[1]), HexToNibble(message[2]),
                    Convert.ToByte(message.Substring(4, 2), 16),
                    Convert.ToByte(message.Substring(7, 2), 16));
            }
        }

        private static X10Message ParseScenario(string message)
        {
            return new X10ScenarioExecute(Convert.ToByte(message.Substring(1, 2), 16));
        }

        private static X10Message ParseModuleState(string message)
        {
            // Module State Request
            if (message[1] != 'W')
            {
                return new X10ModuleStateRequest(message[1], HexToNibble(message[2]));
            }
            // Module State Wipe
            else
            {
                return new X10ModuleStateWipe(message[2]);
            }
        }

        internal static string NibbleToHex(byte number)
        {
            return number < 16 ? number.ToString("X") : "_";
        }

        internal static string NibbleToDecimal(byte number, string unknownReplacement)
        {
            return number < 16 ? (number + 1).ToString() : unknownReplacement;
        }

        internal static byte HexToNibble(char number)
        {
            if (number == '_' || number == '*')
            {
                return 0;
            }
            return (byte)(Convert.ToByte(number.ToString(), 16) + 1);
        }

        #endregion
    }
}
