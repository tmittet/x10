using System;

namespace X10ExCom.X10
{
    public class ScenarioExecute : Message
    {
        public byte Scenario { get; set; }

        public ScenarioExecute(byte scenario)
        {
            Source = MessageSource.Unknown;
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
