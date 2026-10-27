/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  PURPOSE:     Friendly monitor names ("DELL U2720Q") for display device names ("\\.\DISPLAY1")
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include <windows.h>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>

// Shared by GTA's device selection dialog (multiplayer_sa) and the monitor option in
// Settings > Video (core), so both show the same names.
//
// This is copied from SilentPatch:
// https://github.com/CookiePLMonster/SilentPatch/blob/dev/SilentPatch/FriendlyMonitorNames.cpp
//
// Returns a map from GDI device name (as in D3DADAPTER_IDENTIFIER9::DeviceName and
// MONITORINFOEX::szDevice) to the monitor's friendly name. Monitors without a friendly name,
// or Windows versions without the DisplayConfig API, are left out.
inline std::unordered_map<std::string, std::string> GetFriendlyMonitorNamesForDevicePaths()
{
    std::unordered_map<std::string, std::string> monitorNames;

    HMODULE user32Lib = LoadLibrary(TEXT("user32"));
    if (!user32Lib)
        return monitorNames;

    using GetDisplayConfigBufferSizesFn = decltype(&GetDisplayConfigBufferSizes);
    using QueryDisplayConfigFn = decltype(&QueryDisplayConfig);
    using DisplayConfigGetDeviceInfoFn = decltype(&DisplayConfigGetDeviceInfo);

    GetDisplayConfigBufferSizesFn getDisplayConfigBufferSizes = nullptr;
    QueryDisplayConfigFn          queryDisplayConfig = nullptr;
    DisplayConfigGetDeviceInfoFn  displayConfigGetDeviceInfo = nullptr;

    const auto procGetDisplayConfigBufferSizes = GetProcAddress(user32Lib, "GetDisplayConfigBufferSizes");
    const auto procQueryDisplayConfig = GetProcAddress(user32Lib, "QueryDisplayConfig");
    const auto procDisplayConfigGetDeviceInfo = GetProcAddress(user32Lib, "DisplayConfigGetDeviceInfo");

    static_assert(sizeof(getDisplayConfigBufferSizes) == sizeof(procGetDisplayConfigBufferSizes), "Unexpected function pointer size");
    static_assert(sizeof(queryDisplayConfig) == sizeof(procQueryDisplayConfig), "Unexpected function pointer size");
    static_assert(sizeof(displayConfigGetDeviceInfo) == sizeof(procDisplayConfigGetDeviceInfo), "Unexpected function pointer size");

    if (procGetDisplayConfigBufferSizes)
        std::memcpy(&getDisplayConfigBufferSizes, &procGetDisplayConfigBufferSizes, sizeof(getDisplayConfigBufferSizes));
    if (procQueryDisplayConfig)
        std::memcpy(&queryDisplayConfig, &procQueryDisplayConfig, sizeof(queryDisplayConfig));
    if (procDisplayConfigGetDeviceInfo)
        std::memcpy(&displayConfigGetDeviceInfo, &procDisplayConfigGetDeviceInfo, sizeof(displayConfigGetDeviceInfo));
    if (!getDisplayConfigBufferSizes || !queryDisplayConfig || !displayConfigGetDeviceInfo)
    {
        FreeLibrary(user32Lib);
        return monitorNames;
    }

    UINT32                                     pathCount, modeCount;
    std::unique_ptr<DISPLAYCONFIG_PATH_INFO[]> paths;
    std::unique_ptr<DISPLAYCONFIG_MODE_INFO[]> modes;

    LONG result = ERROR_SUCCESS;
    do
    {
        result = getDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount);
        if (result != ERROR_SUCCESS)
        {
            break;
        }
        paths = std::make_unique<DISPLAYCONFIG_PATH_INFO[]>(pathCount);
        modes = std::make_unique<DISPLAYCONFIG_MODE_INFO[]>(modeCount);
        result = queryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.get(), &modeCount, modes.get(), nullptr);
    } while (result == ERROR_INSUFFICIENT_BUFFER);

    if (result != ERROR_SUCCESS)
    {
        FreeLibrary(user32Lib);
        return monitorNames;
    }

    for (size_t i = 0; i < pathCount; i++)
    {
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        const LONG targetNameResult = displayConfigGetDeviceInfo(&targetName.header);

        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
        sourceName.header.adapterId = paths[i].sourceInfo.adapterId;
        sourceName.header.id = paths[i].sourceInfo.id;
        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        sourceName.header.size = sizeof(sourceName);
        const LONG sourceNameResult = displayConfigGetDeviceInfo(&sourceName.header);
        if (targetNameResult == ERROR_SUCCESS && sourceNameResult == ERROR_SUCCESS && targetName.monitorFriendlyDeviceName[0] != '\0')
        {
            char gdiDeviceName[std::size(sourceName.viewGdiDeviceName)];
            char monitorFriendlyDeviceName[std::size(targetName.monitorFriendlyDeviceName)];
            WideCharToMultiByte(CP_ACP, 0, sourceName.viewGdiDeviceName, -1, gdiDeviceName, static_cast<int>(std::size(gdiDeviceName)), nullptr, nullptr);
            WideCharToMultiByte(CP_ACP, 0, targetName.monitorFriendlyDeviceName, -1, monitorFriendlyDeviceName,
                                static_cast<int>(std::size(monitorFriendlyDeviceName)), nullptr, nullptr);

            monitorNames.try_emplace(gdiDeviceName, monitorFriendlyDeviceName);
        }
    }

    FreeLibrary(user32Lib);
    return monitorNames;
}
