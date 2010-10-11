namespace X10ExCom
{
    public enum X10MessageSource
    {
        Unknown,
        Parser,
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

    public enum X10ExtendedCategory : byte // nibble
    {
        Shutter = 0x00,
        Sensor = 0x10,
        Security = 0x20,
        Module = 0x30,
        SecureAddress = 0x40,
        SecureGroup = 0x50,
    }

    public enum X10ExtendedFunction : byte // nibble
    {
        // Shutter
        OpenEnableSunProtect = X10ExtendedCategory.Shutter | 0x1,
        LimitEnableSunProtect = X10ExtendedCategory.Shutter | 0x2,
        OpenDisableSunProtect = X10ExtendedCategory.Shutter | 0x3,
        OpenHouseDisableSunProtect = X10ExtendedCategory.Shutter | 0x4,
        OpenAllDisableSunProtect = X10ExtendedCategory.Shutter | 0x5,
        IncludeInLifestyle = X10ExtendedCategory.Shutter | 0x7,
        BeginLifestyleMode = X10ExtendedCategory.Shutter | 0x8,
        RemoveFromLifestyle = X10ExtendedCategory.Shutter | 0x9,
        RemoveFromAllLifestyles = X10ExtendedCategory.Shutter | 0xA,
        CloseHouseEnableSunProtect = X10ExtendedCategory.Shutter | 0xB,
        CloseAllEnableSunProtect = X10ExtendedCategory.Shutter | 0xC,
        SelfTestHouseUnitUp1S = X10ExtendedCategory.Shutter | 0xE,
        SelftTestEaromUp1SDn1S = X10ExtendedCategory.Shutter | 0xF,
        
        // Sensor
        RequestAverageLight = X10ExtendedCategory.Sensor | 0x1,
        RequestInstantTemp = X10ExtendedCategory.Sensor | 0x2,
        RequestStatus = X10ExtendedCategory.Sensor | 0x3,
        RequestInstantLight = X10ExtendedCategory.Sensor | 0x4,
        Request16MinAverageTemp = X10ExtendedCategory.Sensor | 0x5,
        RequestAmbientLight = X10ExtendedCategory.Sensor | 0xB,
        RequestTemperature = X10ExtendedCategory.Sensor | 0xC,
        RequestStatusBitMapped = X10ExtendedCategory.Sensor | 0xD,
        
        // Module (Dimmer and Appliances)
        IncludeInGroupAtCurrentSetting = X10ExtendedCategory.Module | 0x0,
        PreSetDim = X10ExtendedCategory.Module | 0x1,
        IncludeInGroup = X10ExtendedCategory.Module | 0x2,
        AllUnitsInHouseOn = X10ExtendedCategory.Module | 0x3,
        AllUnitsInHouseOff = X10ExtendedCategory.Module | 0x4,
        RemoveFromGroup = X10ExtendedCategory.Module | 0x5,
        ExecuteGroupFunction = X10ExtendedCategory.Module | 0x6,
        RequestOutputStatus = X10ExtendedCategory.Module | 0x7,
        OutputStatusAck = X10ExtendedCategory.Module | 0x8,
        GroupStatusAck = X10ExtendedCategory.Module | 0x9,
        GroupStatusAckNotInGroup = X10ExtendedCategory.Module | 0xA,
        ConfigureAutoAck = X10ExtendedCategory.Module | 0xB,
        GroupBrightDim = X10ExtendedCategory.Module | 0xC,
        
        // SecureAddress
        HouseUnitAddressMatch = X10ExtendedCategory.SecureAddress | 0x0,
        HouseAddressMatch = X10ExtendedCategory.SecureAddress | 0x1,
        HouseAddressMatchExecuteStandard = X10ExtendedCategory.SecureAddress | 0x2,
        HouseAddressMatchExecuteExtended = X10ExtendedCategory.SecureAddress | 0x3,
        HouseUnitAddressMatchExecuteOn = X10ExtendedCategory.SecureAddress | 0x4,
        HouseUnitAddressMatchExecuteOff = X10ExtendedCategory.SecureAddress | 0x5,
        
        // SecureGroup
        ExecuteGroup0 = X10ExtendedCategory.SecureGroup | 0x0,
        ExecuteGroup1 = X10ExtendedCategory.SecureGroup | 0x1,
        ExecuteGroup2 = X10ExtendedCategory.SecureGroup | 0x2,
        ExecuteGroup3 = X10ExtendedCategory.SecureGroup | 0x3,
        ExecuteGroup0Off = X10ExtendedCategory.SecureGroup | 0x4,
        ExecuteGroup1Off = X10ExtendedCategory.SecureGroup | 0x5,
        ExecuteGroup2Off = X10ExtendedCategory.SecureGroup | 0x6,
        ExecuteGroup3Off = X10ExtendedCategory.SecureGroup | 0x7,
        ExecuteGroup0Bright = X10ExtendedCategory.SecureGroup | 0x8,
        ExecuteGroup1Bright = X10ExtendedCategory.SecureGroup | 0x9,
        ExecuteGroup2Bright = X10ExtendedCategory.SecureGroup | 0xA,
        ExecuteGroup3Bright = X10ExtendedCategory.SecureGroup | 0xB,
        ExecuteGroup0Dim = X10ExtendedCategory.SecureGroup | 0xC,
        ExecuteGroup1Dim = X10ExtendedCategory.SecureGroup | 0xD,
        ExecuteGroup2Dim = X10ExtendedCategory.SecureGroup | 0xE,
        ExecuteGroup3Dim = X10ExtendedCategory.SecureGroup | 0xF,
    }
}
