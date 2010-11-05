using System;

namespace X10ExCom.X10
{
    public class ModuleStateWipe : Message
    {
        public House House { get; set; }

        public ModuleStateWipe(House house)
        {
            Source = MessageSource.Unknown;
            House = house;
        }

        public ModuleStateWipe(char house)
        {
            if ((house < (byte)'A' || house > (byte)'P') && house != '*')
            {
                throw new ArgumentException("House is outside valid range A-P or *.");
            }
            Source = MessageSource.Unknown;
            House = (House)house;
        }

        public override string ToString()
        {
            return "RW" + (House == House.X ? "*" :  House.ToString());
        }

        public override string ToHumanReadableString()
        {
            return String.Format(
                "{0}House = {1}",
                base.ToHumanReadableString(),
                Convert.ToChar(House));
        }
    }
}
