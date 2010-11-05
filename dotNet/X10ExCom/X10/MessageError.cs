using System;

namespace X10ExCom.X10
{
    public class MessageError : Message
    {
        public string Code { get; private set; }
        public string Message { get; private set; }

        public MessageError(MessageSource source, string code)
        {
            Source = source;
            Code = code ?? "";
            switch (Code.ToUpper())
            {
                case "BUFFER":
                    Message = "X10 message buffer is full. Please wait a few seconds and try again.";
                    break;
                case "SYNTAX":
                    Message = "Syntax error in message sent to controller.";
                    break;
                case "TIMOUT":
                    Message =
                        Source != MessageSource.Ethernet ?
                        "Complete 3 or 9 character message not received within timeout." :
                        "Response Timeout. Client failed to send response to 100 Continue challenge within threshold.";
                    break;
                case "NOAUTH":
                    Message = "Unauthorized. Client tried to log in with invalid user name or password.";
                    break;
                case "METHOD":
                    Message = "Not Implemented. Client made a request using an unsupported HTTP method.";
                    break;
                default:
                    Message = "Unknown exception type.";
                    break;
            }
        }

        public MessageError(MessageSource source, string code, string message)
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
                "{0}Code = {1}, Message = {2}",
                base.ToHumanReadableString(),
                Code,
                Message);
        }
    }
}
