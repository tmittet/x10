using System;

namespace X10ExCom
{
    public class X10ScenarioExecute : X10Message
    {
        public byte Scenario { get; set; }

        public X10ScenarioExecute(byte scenario)
        {
            Source = X10MessageSource.Unknown;
            Scenario = scenario;
        }

        public override string ToString()
        {
            return "S" + Scenario.ToString("X").PadLeft(2, '0');
        }

        public override string ToHumanReadableString()
        {
            return String.Format(
                "{0}Scenario = 0x{1} ({2})",
                base.ToHumanReadableString(),
                Scenario.ToString("X").PadLeft(2, '0'),
                Scenario);
        }
    }
}
