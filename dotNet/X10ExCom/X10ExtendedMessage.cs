using System;
using System.Runtime.Serialization;

namespace X10ExCom
{
    [Serializable]
    [DataContract]
    public class X10ExtendedMessage : X10StandardMessage
    {
        public byte ExtendedCommand { get; set; }
        public byte ExtendedData { get; set; }

        /// <summary>
        /// ExtendedCategoryValue is the first nibble (4 bits) of the ExtendedCommand.
        /// Note: Only used with ExtendedCode commands.
        /// </summary>
        public byte ExtendedCategoryValue
        {
            get { return Convert.ToByte(Command == X10Command.ExtendedCode ? ExtendedCommand >> 4 : 0); }
        }

        /// <summary>
        /// ExtendedFunctionValue is the last nibble (4 bits) of the ExtendedCommand.
        /// Note: Only used with ExtendedCode commands.
        /// </summary>
        public byte ExtendedFunctionValue
        {
            get { return Convert.ToByte(Command == X10Command.ExtendedCode ? ExtendedCommand & 0xF : 0); }
        }

        /// <summary>
        /// ExtendedCategoryName is parsed by looking up ExtendedCategoryValue in ExtendedType enum.
        /// Note: Only used with ExtendedCode commands.
        /// </summary>
        public string ExtendedCategoryName
        {
            get
            {
                if (Command != X10Command.ExtendedCode) return String.Empty;
                X10ExtendedCategory result;
                return
                    Enum.TryParse(((X10ExtendedCategory)(ExtendedCommand & 0xF0)).ToString(), false, out result) ?
                    result.ToString() :
                    "Unknown";
            }
        }

        /// <summary>
        /// ExtendedFunctionName is parsed by looking up ExtendedFunctionValue in ExtendedType enum.
        /// Note: Only used with ExtendedCode commands.
        /// </summary>
        public string ExtendedFunctionName
        {
            get
            {
                if (Command != X10Command.ExtendedCode) return String.Empty;
                X10ExtendedFunction result;
                return
                    Enum.TryParse(((X10ExtendedFunction)(ExtendedCommand)).ToString(), false, out result) ?
                    result.ToString() :
                    "Unknown";
            }
        }

        /// <summary>
        /// ExtendedBrightness is calculated from ExtendedData when command is StatusOn, StatusOff
        /// or command is ExtendedCode and ExtendedCommand is PreSetDim.
        /// </summary>
        [DataMember(Name = "brightness", IsRequired = false, Order = 7)]
        public byte ExtendedBrightness
        {
            get
            {
                return
                    Convert.ToByte(
                        Command == X10Command.StatusOn ||
                        Command == X10Command.StatusOff ||
                        (Command == X10Command.ExtendedCode && ExtendedCommand == (byte)X10ExtendedFunction.PreSetDim) ?
                        Math.Round(100D * (ExtendedData & 0x3F) / 62) :
                        0);
            }
            set
            {
                if (value > 0 && Command == X10Command.StatusOn || Command == X10Command.StatusOff)
                {
                    ExtendedCommand = (byte)X10ExtendedCategory.Module | (byte)X10ExtendedFunction.PreSetDim;
                    ExtendedData = (byte)Math.Round(62 * (value >= 100 ? 1 : value / 100D));
                }
            }
        }

        public X10ExtendedMessage()
        {
            ExtendedCommand = 0;
            ExtendedData = 0;
        }

        /// <summary>
        /// Creates new X10 extended message.
        /// </summary>
        /// <param name="house">Valid range is A-P.</param>
        /// <param name="unit">Valid units are 01-16.</param>
        /// <param name="command">Only commands ExtendedCode and ExtendedData are valid for extended messages.</param>
        /// <param name="extendedCommand">Byte 0-255. Make sure that either extended command or data is above 0.</param>
        /// <param name="extendedData">Byte 0-255. Make sure that either extended command or data is above 0.</param>
        public X10ExtendedMessage(X10House house, X10Unit unit, X10Command command, byte extendedCommand, byte extendedData)
            : base(house, unit, command)
        {
            Validate(extendedCommand, extendedData);
            ExtendedCommand = extendedCommand;
            ExtendedData = extendedData;
        }

        /// <summary>
        /// Creates new X10 extended message.
        /// </summary>
        /// <param name="house">Valid characters are A-P.</param>
        /// <param name="unit">Units range from 1-16.</param>
        /// <param name="command">Only commands 7 (ExtendedCode) and 12 (ExtendedData) are valid for extended messages.</param>
        /// <param name="extendedCommand">Byte 0-255. Make sure that either extended command or data is above 0.</param>
        /// <param name="extendedData">Byte 0-255. Make sure that either extended command or data is above 0.</param>
        public X10ExtendedMessage(char house, byte unit, byte command, byte extendedCommand, byte extendedData)
            : base(house, unit, command)
        {
            Validate(extendedCommand, extendedData);
            ExtendedCommand = extendedCommand;
            ExtendedData = extendedData;
        }

        public override string ToString()
        {
            return
                Convert.ToChar(House) +
                NibbleToHex((byte)Unit) +
                NibbleToHex((byte)Command) +
                "x" +
                ExtendedCommand.ToString("X").PadLeft(2, '0') +
                "x" +
                ExtendedData.ToString("X").PadLeft(2, '0');
        }

        public override string ToHumanReadableString()
        {
            string standardMessage = String.Format(
                "Type = {0}, Module = {1}{2}, Command = {3}",
                "ExtendedMessage",
                Convert.ToChar(House),
                NibbleToDecimal((byte) Unit, "_"),
                Command);
            switch (Command)
            {
                case X10Command.ExtendedCode:
                    return String.Format(
                        "{0}, Category = {1} (0x{2}), Function = {3} (0x{4}), Data = 0x{5}",
                        standardMessage,
                        ExtendedCategoryName,
                        ExtendedCategoryValue.ToString("X"),
                        ExtendedFunctionName,
                        ExtendedFunctionValue.ToString("X"),
                        ExtendedData.ToString("X").PadLeft(2, '0'));
                case X10Command.StatusOn:
                case X10Command.StatusOff:
                    return String.Format(
                        "{0}, Brightness = {1}%",
                        standardMessage,
                        ExtendedBrightness);
                default:
                    return String.Format(
                        "{0}, ExtCommand = 0x{1}, Data = 0x{2}",
                        standardMessage,
                        ExtendedCommand,
                        ExtendedData);
            }
        }

        private void Validate(byte extendedCommand, byte extendedData)
        {
            if (House == X10House.X)
            {
                throw new ArgumentException("House X (all house codes) is invalid when sending extended messages.");
            }
            if (Unit == X10Unit.X)
            {
                throw new ArgumentException("Unit X (no unit) is invalid when sending extended messages.");
            }
            if (
                Command != X10Command.ExtendedCode && Command != X10Command.ExtendedData &&
                // Allow status on and staus of because of module state request response data
                Command != X10Command.StatusOn && Command != X10Command.StatusOff)
            {
                throw new ArgumentException(String.Format(
                    "\"{0}\" command is invalid when sending extended messages. " +
                    "Command must either be set to {1} ({2}) or {3} ({4}).",
                    Command,
                    X10Command.ExtendedCode,
                    (byte)X10Command.ExtendedCode,
                    X10Command.ExtendedData,
                    (byte)X10Command.ExtendedData));
            }
            if(extendedCommand == 0 && extendedData == 0)
            {
                throw new ArgumentException(
                    "Invalid extended message. Both extendedCommand and extendedData are set to 0.");
            }
        }
    }
}
