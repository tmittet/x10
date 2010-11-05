namespace X10ExCom.X10
{
    public enum MessageSource
    {
        Unknown,
        Parser,
        Serial,
        ModuleState,
        PowerLine,
        Radio,
        Infrared,
        Ethernet,
    }

    public enum ModuleType : byte
    {
        Unknown = 0x0,
        Appliance = 0x1,
        Dimmer = 0x2,
        Sensor = 0x3,
    }

    public enum House : byte
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

    public enum Unit : byte
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

    public enum Command : byte
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

    public enum ExtendedCategory : byte // nibble
    {
        Shutter = 0x00,
        Sensor = 0x10,
        Security = 0x20,
        Module = 0x30,
        SecureAddress = 0x40,
        SecureGroup = 0x50,
    }

    public enum ExtendedFunction : byte // nibble
    {
        // Shutter
        OpenEnableSunProtect = ExtendedCategory.Shutter | 0x1,
        LimitEnableSunProtect = ExtendedCategory.Shutter | 0x2,
        OpenDisableSunProtect = ExtendedCategory.Shutter | 0x3,
        OpenHouseDisableSunProtect = ExtendedCategory.Shutter | 0x4,
        OpenAllDisableSunProtect = ExtendedCategory.Shutter | 0x5,
        IncludeInLifestyle = ExtendedCategory.Shutter | 0x7,
        BeginLifestyleMode = ExtendedCategory.Shutter | 0x8,
        RemoveFromLifestyle = ExtendedCategory.Shutter | 0x9,
        RemoveFromAllLifestyles = ExtendedCategory.Shutter | 0xA,
        CloseHouseEnableSunProtect = ExtendedCategory.Shutter | 0xB,
        CloseAllEnableSunProtect = ExtendedCategory.Shutter | 0xC,
        SelfTestHouseUnitUp1S = ExtendedCategory.Shutter | 0xE,
        SelftTestEaromUp1SDn1S = ExtendedCategory.Shutter | 0xF,
        
        // Sensor
        RequestAverageLight = ExtendedCategory.Sensor | 0x1,
        RequestInstantTemp = ExtendedCategory.Sensor | 0x2,
        RequestStatus = ExtendedCategory.Sensor | 0x3,
        RequestInstantLight = ExtendedCategory.Sensor | 0x4,
        Request16MinAverageTemp = ExtendedCategory.Sensor | 0x5,
        RequestAmbientLight = ExtendedCategory.Sensor | 0xB,
        RequestTemperature = ExtendedCategory.Sensor | 0xC,
        RequestStatusBitMapped = ExtendedCategory.Sensor | 0xD,
        
        // Module (Dimmer and Appliances)
        IncludeInGroupAtCurrentSetting = ExtendedCategory.Module | 0x0,
        PreSetDim = ExtendedCategory.Module | 0x1,
        IncludeInGroup = ExtendedCategory.Module | 0x2,
        AllUnitsInHouseOn = ExtendedCategory.Module | 0x3,
        AllUnitsInHouseOff = ExtendedCategory.Module | 0x4,
        RemoveFromGroup = ExtendedCategory.Module | 0x5,
        ExecuteGroupFunction = ExtendedCategory.Module | 0x6,
        RequestOutputStatus = ExtendedCategory.Module | 0x7,
        OutputStatusAck = ExtendedCategory.Module | 0x8,
        GroupStatusAck = ExtendedCategory.Module | 0x9,
        GroupStatusAckNotInGroup = ExtendedCategory.Module | 0xA,
        ConfigureAutoAck = ExtendedCategory.Module | 0xB,
        GroupBrightDim = ExtendedCategory.Module | 0xC,
        
        // SecureAddress
        HouseUnitAddressMatch = ExtendedCategory.SecureAddress | 0x0,
        HouseAddressMatch = ExtendedCategory.SecureAddress | 0x1,
        HouseAddressMatchExecuteStandard = ExtendedCategory.SecureAddress | 0x2,
        HouseAddressMatchExecuteExtended = ExtendedCategory.SecureAddress | 0x3,
        HouseUnitAddressMatchExecuteOn = ExtendedCategory.SecureAddress | 0x4,
        HouseUnitAddressMatchExecuteOff = ExtendedCategory.SecureAddress | 0x5,
        
        // SecureGroup
        ExecuteGroup0 = ExtendedCategory.SecureGroup | 0x0,
        ExecuteGroup1 = ExtendedCategory.SecureGroup | 0x1,
        ExecuteGroup2 = ExtendedCategory.SecureGroup | 0x2,
        ExecuteGroup3 = ExtendedCategory.SecureGroup | 0x3,
        ExecuteGroup0Off = ExtendedCategory.SecureGroup | 0x4,
        ExecuteGroup1Off = ExtendedCategory.SecureGroup | 0x5,
        ExecuteGroup2Off = ExtendedCategory.SecureGroup | 0x6,
        ExecuteGroup3Off = ExtendedCategory.SecureGroup | 0x7,
        ExecuteGroup0Bright = ExtendedCategory.SecureGroup | 0x8,
        ExecuteGroup1Bright = ExtendedCategory.SecureGroup | 0x9,
        ExecuteGroup2Bright = ExtendedCategory.SecureGroup | 0xA,
        ExecuteGroup3Bright = ExtendedCategory.SecureGroup | 0xB,
        ExecuteGroup0Dim = ExtendedCategory.SecureGroup | 0xC,
        ExecuteGroup1Dim = ExtendedCategory.SecureGroup | 0xD,
        ExecuteGroup2Dim = ExtendedCategory.SecureGroup | 0xE,
        ExecuteGroup3Dim = ExtendedCategory.SecureGroup | 0xF,
    }
}
