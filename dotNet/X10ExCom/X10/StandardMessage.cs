using System;
using System.Runtime.Serialization;

namespace X10ExCom.X10
{
    [Serializable]
    [DataContract]
    public class StandardMessage : Message
    {
        public House House { get; set; }

        [DataMember(Name = "house", IsRequired = true, Order = 1)]
        public string HouseString
        {
            get { return ((char)House).ToString(); }
            set { House = (House)value[0]; }
        }

        public Unit Unit { get; set; }

        [DataMember(Name = "unit", IsRequired = true, Order = 2)]
        public byte UnitNumber
        {
            get { return (byte)((byte)Unit + 1); }
            set { Unit = (Unit)(byte)(value - 1); }
        }

        [DataMember(Name = "url", IsRequired = true, Order = 3)]
        public string Url { get; set; }

        public Command Command { get; set; }

        [DataMember(Name = "type", IsRequired = false, Order = 4)]
        public ModuleType ModuleType { get; set; }

        [DataMember(Name = "name", IsRequired = false, Order = 5)]
        public string Name { get; set; }

        [DataMember(Name = "on", IsRequired = false, Order = 6)]
        public bool? On
        {
            get
            {
                if (
                    Command != Command.On &&
                    Command != Command.Off &&
                    Command != Command.StatusOn &&
                    Command != Command.StatusOff &&
                    Command != Command.Bright &&
                    Command != Command.Dim)
                {
                    return null;
                }
                return Command == Command.On || Command ==  Command.StatusOn || Command == Command.Bright || Command == Command.Dim;
            }
            set
            {
                if (value.HasValue)
                {
                    Command = value.Value ? Command.StatusOn : Command.StatusOff;
                }
                
            }
        }

        public StandardMessage()
        {
            Source = MessageSource.Ethernet;
            House = House.X;
            Unit = Unit.X;
            Url = "/";
            Command = Command.X;
            ModuleType = ModuleType.Unknown;
        }

        /// <summary>
        /// Creates new X10 standard message.
        /// </summary>
        /// <param name="house">Valid range is A-P.</param>
        /// <param name="unit">Valid units are 01-16 and X for no unit.</param>
        /// <param name="command">All commands are valid, use X for no command.</param>
        public StandardMessage(House house, Unit unit, Command command)
        {
            Source = MessageSource.Unknown;
            if (house == House.X)
            {
                throw new ArgumentException(
                    "House is outside valid range A-P. " +
                    "X (all house codes) only allowed when requesting module state or parsing response from module state wipe.");
            }
            House = house;
            Unit = unit;
            Url = "/" + House + "/" + ((byte)Unit + 1) + "/";
            Command = command;
            ModuleType = ModuleType.Unknown;
        }

        /// <summary>
        /// Creates new X10 standard message.
        /// </summary>
        /// <param name="house">Valid characters are A-P or *.</param>
        /// <param name="unit">Units range from 1-16, 0 is treated as no unit.</param>
        /// <param name="command">Commands range from 1-16, 0 is treated as no command.</param>
        public StandardMessage(char house, byte unit, byte command)
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
            Source = MessageSource.Unknown;
            House = (House)house;
            Unit = (Unit)unit - 1;
            Url = "/" + House + "/" + ((byte)Unit + 1) + "/";
            Command = (Command)command - 1;
            ModuleType = ModuleType.Unknown;
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
                ModuleType,
                String.IsNullOrEmpty(Name) ? "" : " (" + Name + ")",
                Command);
        }
    }
}