char* StatusToString(KernelStatus Status)
{
    switch (Status)
    {
    case KernelStatusSuccess:
        return AllocateStringArray("Success");
    case KernelStatusNotFound:
        return AllocateStringArray("Not Found");
    case KernelStatusDisaster:
        return AllocateStringArray("Unknown Dangerous Exception");
    case KernelStatusInfiniteLoopTimeouted:
        return AllocateStringArray("TimeOut");
    case KernelStatusNoBudget:
        return AllocateStringArray("Out Of Resources");
    case KernelStatusMemoryRot:
        return AllocateStringArray("Damaged Memory");
    case KernelStatusDiskServicesDiskErr:
        return AllocateStringArray("Disk Error");
    case KernelStatusInvalidParam:
        return AllocateStringArray("Invalid Parameter");
    case KernelStatusThingVerySmall:
        return AllocateStringArray("Buffer Too Small");
    case KernelStatusDeviceBusy:
        return AllocateStringArray("Device Busy");
    default:
        return AllocateStringArray("huh?");
    }
}