using System;
using System.Runtime.Serialization;

namespace X10ExCom
{
    [Serializable]
    [DataContract]
    public class X10StandardMessage : X10Message
    {
        public X10House House { get; set; }

        [DataMember(Name = "house", IsRequired = true, Order = 1)]
        public string HouseString
        {
            get { return ((char)House).ToString(); }
            set { House = (X10House)value[0]; }
        }

        public X10Unit Unit { get; set; }

        [DataMember(Name = "unit", IsRequired = true, Order = 2)]
        public byte UnitNumber
        {
            get { return (byte)((byte)Unit + 1); }
            set { Unit = (X10Unit)(byte)(value - 1); }
        }

        [DataMember(Name = "url", IsRequired = true, Order = 3)]
        public string Url { get; set; }

        public X10Command Command { get; set; }

        [DataMember(Name = "type", IsRequired = false, Order = 4)]
        public X10Type Type { get; set; }

        [DataMember(Name = "name", IsRequired = false, Order = 5)]
        public string Name { get; set; }

        [DataMember(Name = "on", IsRequired = false, Order = 6)]
        public bool? On
        {
            get
            {
                if (
                    Command != X10Command.On &&
                    Command != X10Command.Off &&
                    Command != X10Command.StatusOn &&
                    Command != X10Command.StatusOff &&
                    Command != X10Command.Bright &&
                    Command != X10Command.Dim)
                {
                    return null;
                }
                return Command == X10Command.On || Command ==  X10Command.StatusOn || Command == X10Command.Bright || Command == X10Command.Dim;
            }
            set
            {
                if (value.HasValue)
                {
                    Command = value.Value ? X10Command.StatusOn : X10Command.StatusOff;
                }
                
            }
        }

        public X10StandardMessage()
        {
            Source = X10MessageSource.Ethernet;
            House = X10House.X;
            Unit = X10Unit.X;
            Url = "/";
            Command = X10Command.X;
            Type = X10Type.Unknown;
        }

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
            Url = "/" + House + "/" + ((byte)Unit + 1) + "/";
            Command = command;
            Type = X10Type.Unknown;
        }

        /// <summary>
        /// Creates new X10 standard message.
        /// </summary>
        /// <param name="house">Valid characters are A-P or *.</param>
        /// <param name="unit">Units range from 1-16, 0 is treated as no unit.</param>
        /// <param name="command">Commands range from 1-16, 0 is treated as no command.</param>
        public X10StandardMessage(char house, byte unit, byte command)
        {
            if ((house < (byte)'A' || house > (byte)'P') && house != '*')
            {
                throw new ArgumentException("House is outside valid range A-P or *.");
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
            Url = "/" + House + "/" + ((byte)Unit + 1) + "/";
            Command = (X10Command)command - 1;
            Type = X10Type.Unknown;
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
                "{0}Module = {1}{2}, Type = {3}{4}, Command = {5}",
                base.ToHumanReadableString(),
                Convert.ToChar(House),
                UnitToString(Unit, "_"),
                Type,
                String.IsNullOrEmpty(Name) ? "" : " (" + Name + ")",
                Command);
        }
    }
}