namespace X10ExCom
{
    public enum X10MessageSource
    {
        Unknown,
        Parser,
        Debug,
        Serial,
        ModuleState,
        PowerLine,
        Radio,
        Infrared,
    }

    public enum X10House : byte
    {
        A = (byte)'A',
        B = (byte)'B',
        C = (byte)'C',
        D = (byte)'D',
        E = (byte)'E',
        F = (byte)'F',
        G = (byte)'G',
        H = (byte)'H',
        I = (byte)'I',
        J = (byte)'J',
        K = (byte)'K',
        L = (byte)'L',
        M = (byte)'M',
        N = (byte)'N',
        O = (byte)'O',
        P = (byte)'P',

        X = (byte)'*',
    }

    public enum X10Unit : byte
    {
        U01 = 0x0,
        U02 = 0x1,
        U03 = 0x2,
        U04 = 0x3,
        U05 = 0x4,
        U06 = 0x5,
        U07 = 0x6,
        U08 = 0x7,
        U09 = 0x8,
        U10 = 0x9,
        U11 = 0xA,
        U12 = 0xB,
        U13 = 0xC,
        U14 = 0xD,
        U15 = 0xE,
        U16 = 0xF,

        X = 0xFF,
    }

    public enum X10Command : byte
    {
        AllUnitsOff = 0x0,
        AllLightsOn = 0x1,
        On = 0x2,
        Off = 0x3,
        Dim = 0x4,
        Bright = 0x5,
        AllLightsOff = 0x6,
        ExtendedCode = 0x7,
        HailRequest = 0x8,
        HailAcknowledge = 0x9,
        PreSetDim0 = 0xA,
        PreSetDim1 = 0xB,
        ExtendedData = 0xC,
        StatusOn = 0xD,
        StatusOff = 0xE,
        StatusRequest = 0xF,

        X = 0xFF,
    }
}
