using System;

namespace X10ExCom
{
    public abstract class X10Message
    {
        public X10MessageSource Source { get; internal set; }
        public string SourceString { get; private set; }

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
            if(message.Substring(2, 1) == ":")
            {
                source = message.Substring(0, 2).ToUpper();
                message = message.Substring(3);
            }
            X10Message x10Msg;
            if (message.Length >= 3 && message.Substring(0, 3).ToUpper() == "_EX")
            {
                x10Msg = new X10Error(message.Substring(3));
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
                    if (message.Length == 3)
                    {
                        x10Msg = new X10StandardMessage(message[0], HexToNibble(message[1]), HexToNibble(message[2]));
                    }
                    else
                    {
                        x10Msg = new X10ExtendedMessage(
                            message[0], HexToNibble(message[1]), HexToNibble(message[2]),
                            Convert.ToByte(message.Substring(4, 2), 16),
                            Convert.ToByte(message.Substring(7, 2), 16));
                    }
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
                        x10Msg = new X10ScenarioExecute(Convert.ToByte(message.Substring(1, 2), 16));
                    }
                    // Module State
                    else if (message[0] == 'R')
                    {
                        // Module State Request
                        if (message[1] != 'W')
                        {
                            x10Msg = new X10ModuleStateRequest(message[1], HexToNibble(message[2]));
                        }
                            // Module State Wipe
                        else
                        {
                            x10Msg = new X10ModuleStateWipe(message[2]);
                        }
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
            switch (source)
            {
                case "XP": x10Msg.Source = X10MessageSource.Parser; break;
                case "SD": x10Msg.Source = X10MessageSource.Serial; break;
                case "MS": x10Msg.Source = X10MessageSource.ModuleState; break;
                case "PL": x10Msg.Source = X10MessageSource.PowerLine; break;
                case "RF": x10Msg.Source = X10MessageSource.Radio; break;
                case "IR": x10Msg.Source = X10MessageSource.Infrared; break;
                default: x10Msg.Source = X10MessageSource.Unknown; break;
            }
            return x10Msg;
        }

        public abstract override string ToString();

        public abstract string ToHumanReadableString();

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
    }
}
