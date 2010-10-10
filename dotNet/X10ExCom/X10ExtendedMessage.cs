using System;

namespace X10ExCom
{
    public class X10ExtendedMessage : X10StandardMessage
    {
        public byte ExtendedCommand { get; set; }
        public byte ExtendedData { get; set; }

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
            return String.Format(
                "Type = {0}, Module = {1}{2}, Command = {3}, ExtCommand = 0x{4}, ExtData = 0x{5}",
                "ExtendedMessage",
                Convert.ToChar(House),
                NibbleToDecimal((byte)Unit, "_"),
                Command,
                ExtendedCommand.ToString("X").PadLeft(2, '0'),
                ExtendedData.ToString("X").PadLeft(2, '0'));
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
