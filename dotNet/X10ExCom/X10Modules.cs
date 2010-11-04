using System;
using System.Collections.Generic;
using System.Runtime.Serialization;

namespace X10ExCom
{
    [Serializable]
    [DataContract]
    internal class X10Modules
    {
        [DataMember(IsRequired = true, Name = "module")]
        public IEnumerable<X10ExtendedMessage> Module { get; set; }
    }
}
