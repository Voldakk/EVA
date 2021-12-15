#pragma once

namespace EVA
{
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
} // namespace EVA