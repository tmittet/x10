using System;

namespace X10ExCom
{
    public class X10ModuleStateRequest : X10Message
    {
        public X10House House { get; set; }
        public X10Unit Unit { get; set; }

        public X10ModuleStateRequest(X10House house, X10Unit unit)
        {
            Source = X10MessageSource.Unknown;
            House = house;
            Unit = unit;
        }

        public X10ModuleStateRequest(char house, byte unit)
        {
            if ((house < (byte)'A' || house > (byte)'P') && house != '*')
            {
                throw new ArgumentException("House is outside valid range A-P or *.");
            }
            if (unit > 16)
            {
                throw new ArgumentException("Unit is outside valid range 0-16.");
            }
            Source = X10MessageSource.Unknown;
            House = (X10House)house;
            Unit = (X10Unit)unit - 1;
        }

        public override string ToString()
        {
            return
                "R" +
                (House == X10House.X ? "*" : House.ToString()) +
                (Unit == X10Unit.X ? "*" : Convert.ToByte(Unit).ToString("X"));
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
