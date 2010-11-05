using System;
using System.Collections.Generic;
using System.Runtime.Serialization;

namespace X10ExCom.X10
{
    [Serializable]
    [DataContract]
    internal class Modules
    {
        [DataMember(IsRequired = true, Name = "module")]
        public IEnumerable<ExtendedMessage> Module { get; set; }
    }
}
