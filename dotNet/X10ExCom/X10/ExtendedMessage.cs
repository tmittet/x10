using System;
using System.Runtime.Serialization;

namespace X10ExCom.X10
{
    [Serializable]
    [DataContract]
    public class ExtendedMessage : StandardMessage
    {
        public byte ExtendedCommand { get; set; }
        public byte ExtendedData { get; set; }

        /// <summary>
        /// ExtendedCategoryValue is the first nibble (4 bits) of the ExtendedCommand.
        /// Note: Only used with ExtendedCode commands.
        /// </summary>
        public byte ExtendedCategoryValue
        {
            get { return Convert.ToByte(Command == Command.ExtendedCode ? ExtendedCommand >> 4 : 0); }
        }

        /// <summary>
        /// ExtendedFunctionValue is the last nibble (4 bits) of the ExtendedCommand.
        /// Note: Only used with ExtendedCode commands.
        /// </summary>
        public byte ExtendedFunctionValue
        {
            get { return Convert.ToByte(Command == Command.ExtendedCode ? ExtendedCommand & 0xF : 0); }
        }

        /// <summary>
        /// ExtendedCategoryName is parsed by looking up ExtendedCategoryValue in ExtendedType enum.
        /// Note: Only used with ExtendedCode commands.
        /// </summary>
        public string ExtendedCategoryName
        {
            get
            {
                if (Command != Command.ExtendedCode) return String.Empty;
                ExtendedCategory result;
                return
                    Enum.TryParse(((ExtendedCategory)(ExtendedCommand & 0xF0)).ToString(), false, out result) ?
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
                if (Command != Command.ExtendedCode) return String.Empty;
                ExtendedFunction result;
                return
                    Enum.TryParse(((ExtendedFunction)(ExtendedCommand)).ToString(), false, out result) ?
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
                        Command == Command.StatusOn ||
                        Command == Command.StatusOff ||
                        (Command == Command.ExtendedCode && ExtendedCommand == (byte)ExtendedFunction.PreSetDim) ?
                        Math.Round(100D * (ExtendedData & 0x3F) / 62) :
                        0);
            }
            set
            {
                if (value > 0 && Command == Command.StatusOn || Command == Command.StatusOff)
                {
                    ExtendedCommand = (byte)ExtendedCategory.Module | (byte)ExtendedFunction.PreSetDim;
                    ExtendedData = (byte)Math.Round(62 * (value >= 100 ? 1 : value / 100D));
                }
            }
        }

        public ExtendedMessage()
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
        public ExtendedMessage(House house, Unit unit, Command command, byte extendedCommand, byte extendedData)
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
        public ExtendedMessage(char house, byte unit, byte command, byte extendedCommand, byte extendedData)
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
            switch (Command)
            {
                case Command.ExtendedCode:
                    return String.Format(
                        "{0}, Category = {1} (0x{2}), Function = {3} (0x{4}), Data = 0x{5}",
                        base.ToHumanReadableString(),
                        ExtendedCategoryName,
                        ExtendedCategoryValue.ToString("X"),
                        ExtendedFunctionName,
                        ExtendedFunctionValue.ToString("X"),
                        ExtendedData.ToString("X").PadLeft(2, '0'));
                case Command.StatusOn:
                case Command.StatusOff:
                    return String.Format(
                        "{0}, Brightness = {1}%",
                        base.ToHumanReadableString(),
                        ExtendedBrightness);
                default:
                    return String.Format(
                        "{0}, ExtCommand = 0x{1}, Data = 0x{2}",
                        base.ToHumanReadableString(),
                        ExtendedCommand,
                        ExtendedData);
            }
        }

        private void Validate(byte extendedCommand, byte extendedData)
        {
            if (House == House.X)
            {
                throw new ArgumentException("House X (all house codes) is invalid when sending extended messages.");
            }
            if (Unit == Unit.X)
            {
                throw new ArgumentException("Unit X (no unit) is invalid when sending extended messages.");
            }
            if (
                Command != Command.ExtendedCode && Command != Command.ExtendedData &&
                // Allow status on and staus of because of module state request response data
                Command != Command.StatusOn && Command != Command.StatusOff)
            {
                throw new ArgumentException(String.Format(
                    "\"{0}\" command is invalid when sending extended messages. " +
                    "Command must either be set to {1} ({2}) or {3} ({4}).",
                    Command,
                    Command.ExtendedCode,
                    (byte)Command.ExtendedCode,
                    Command.ExtendedData,
                    (byte)Command.ExtendedData));
            }
            if(extendedCommand == 0 && extendedData == 0)
            {
                throw new ArgumentException(
                    "Invalid extended message. Both extendedCommand and extendedData are set to 0.");
            }
        }
    }
}
