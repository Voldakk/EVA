#pragma once

namespace EVA
{
    const uint32_t TempUniformBinding = 15;

    enum class Access
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    enum class Usage
    {
        AppModifiedOnceDeviceUsedOnce,
        AppModifiedOnceDeviceUsedRepeatedly,
        AppModifiedRepeatedlyDeviceUsedRepeatedly,

        DeviceModifiedOnceAppUsedOnce,
        DeviceModifiedOnceAppUsedRepeatedly,
        DeviceModifiedRepeatedlyAppUsedRepeatedly,

        DeviceModifiedOnceDeviceUsedOnce,
        DeviceModifiedOnceDeviceUsedRepeatedly,
        DeviceModifiedRepeatedlyDeviceUsedRepeatedly,
    };
}