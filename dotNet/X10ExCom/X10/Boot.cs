using System;

namespace X10ExCom.X10
{
    public class Boot : Message
    {
        public override string ToString()
        {
            return "X10";
        }

        public override string ToHumanReadableString()
        {
            return String.Format(
                "{0}Message = {1}",
                base.ToHumanReadableString(),
                "Arduino rebooted. Setup Complete.");
        }
    }
}
