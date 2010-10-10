using System;

namespace X10ExCom
{
    public class X10ModuleStateWipe : X10Message
    {
        public X10House House { get; set; }

        public X10ModuleStateWipe(X10House house)
        {
            Source = X10MessageSource.Unknown;
            House = house;
        }

        public X10ModuleStateWipe(char house)
        {
            if ((house < (byte)'A' || house > (byte)'P') && house != '*')
            {
                throw new ArgumentException("House is outside valid range A-P or *.");
            }
            Source = X10MessageSource.Unknown;
            House = (X10House)house;
        }

        public override string ToString()
        {
            return "RW" + (House == X10House.X ? "*" :  House.ToString());
        }

        public override string ToHumanReadableString()
        {
            return String.Format(
                "Type = {0}, House = {1}",
                "ModuleStateWipe",
                Convert.ToChar(House));
        }
    }
}
