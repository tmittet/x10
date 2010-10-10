using System;

namespace X10ExCom
{
    public class X10StandardMessage : X10Message
    {
        public X10House House { get; set; }
        public X10Unit Unit { get; set; }
        public X10Command Command { get; set; }

        /// <summary>
        /// Creates new X10 standard message.
        /// </summary>
        /// <param name="house">Valid range is A-P.</param>
        /// <param name="unit">Valid units are 01-16 and X for no unit.</param>
        /// <param name="command">All commands are valid, use X for no command.</param>
        public X10StandardMessage(X10House house, X10Unit unit, X10Command command)
        {
            Source = X10MessageSource.Unknown;
            if (house == X10House.X)
            {
                throw new ArgumentException(
                    "House is outside valid range A-P. " +
                    "X (all house codes) only allowed when requesting module state or parsing response from module state wipe.");
            }
            House = house;
            Unit = unit;
            Command = command;
        }

        /// <summary>
        /// Creates new X10 standard message.
        /// </summary>
        /// <param name="house">Valid characters are A-P.</param>
        /// <param name="unit">Units range from 1-16, 0 is treated as no unit.</param>
        /// <param name="command">Commands range from 1-16, 0 is treated as no command.</param>
        public X10StandardMessage(char house, byte unit, byte command)
        {
            if ((house < (byte)'A' || house > (byte)'P') && house != '*')
            {
                throw new ArgumentException("House is outside valid range A-P.");
            }
            if (unit > 16)
            {
                throw new ArgumentException("Unit is outside valid range 0-16.");
            }
            if (command > 16)
            {
                throw new ArgumentException("Command is outside valid range 0-16.");
            }
            Source = X10MessageSource.Unknown;
            House = (X10House)house;
            Unit = (X10Unit)unit - 1;
            Command = (X10Command)command - 1;
        }

        public override string ToString()
        {
            string value =
                Convert.ToChar(House) +
                NibbleToHex((byte)Unit) +
                NibbleToHex((byte)Command);
            return value;
        }

        public override string ToHumanReadableString()
        {
            return String.Format(
                "Type = {0}, Module = {1}{2}, Command = {3}",
                "StandardMessage",
                Convert.ToChar(House),
                NibbleToDecimal((byte)Unit, "_"),
                Command);
        }
    }
}