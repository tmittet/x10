using System;

namespace X10ExCom.X10
{
    public class ModuleStateRequest : Message
    {
        public House House { get; set; }
        public Unit Unit { get; set; }

        public ModuleStateRequest(House house, Unit unit)
        {
            Source = MessageSource.Unknown;
            House = house;
            Unit = unit;
        }

        public ModuleStateRequest(char house, byte unit)
        {
            if ((house < (byte)'A' || house > (byte)'P') && house != '*')
            {
                throw new ArgumentException("House is outside valid range A-P or *.");
            }
            if (unit > 16)
            {
                throw new ArgumentException("Unit is outside valid range 0-16.");
            }
            Source = MessageSource.Unknown;
            House = (House)house;
            Unit = (Unit)unit - 1;
        }

        public override string ToString()
        {
            return
                "R" +
                (House == House.X ? "*" : House.ToString()) +
                (Unit == Unit.X ? "*" : Convert.ToByte(Unit).ToString("X"));
        }

        public override string ToHumanReadableString()
        {
            return String.Format(
                "{0}Module = {1}{2}",
                base.ToHumanReadableString(),
                Convert.ToChar(House),
                UnitToString(Unit, "*"));
        }
    }
}
