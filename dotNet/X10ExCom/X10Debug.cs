namespace X10ExCom
{
    public class X10Debug : X10Message
    {
        public string Message { get; private set; }

        public X10Debug(string message)
        {
            Source = X10MessageSource.Debug;
            Message = message;
        }

        public override string ToString()
        {
            return "DEBUG=" + Message;
        }

        public override string ToHumanReadableString()
        {
            return Message;
        }
    }
}
