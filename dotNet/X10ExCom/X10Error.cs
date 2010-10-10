using System;

namespace X10ExCom
{
    public class X10Error : X10Message
    {
        public string Code { get; private set; }
        public string Message { get; private set; }

        public X10Error(string code)
        {
            Source = X10MessageSource.Unknown;
            Code = code;
            switch (Code.ToUpper())
            {
                case "TIMOUT":
                    Message = "Complete 3 or 9 character message not received within timeout.";
                    break;
                case "SYNTAX":
                    Message = "Syntax error, in serial message sent to controller.";
                    break;
                case "BUFFER":
                    Message = "X10 message buffer is full. Please wait a few seconds and try again.";
                    break;
                default:
                    Message = "Unknown exception type.";
                    break;
            }
        }

        public X10Error(X10MessageSource source, string code, string message)
        {
            Source = source;
            Code = code;
            Message = message;
        }

        public override string ToString()
        {
            return "_Ex" + Code;
        }

        public override string ToHumanReadableString()
        {
            return String.Format(
                "Type = {0}, Code = {1}, Message = {2}",
                "Error",
                Code,
                Message);
        }
    }
}
