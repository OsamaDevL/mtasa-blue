/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        multiplayer_sa/CMultiplayerSA_DeviceSelection.cpp
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"

#include <cstring>
#include <utils/FriendlyMonitorNames.h>
#define FUNC_rwDeviceSystemRequest     0x7F2AB0
#define FUNC_DialogFunc                0x745E50
#define FUNC_RwEngineGetSubSystemInfo  0x7F2C30
#define CLASS_RwGlobals                0xC97B24
#define CLASS_IDirect3D9               0xC97C20
#define NUM_DialogFuncStackPushAddress 0x746239

struct RwSubSystemInfo
{
    char name[80];
};

using rwDeviceSystemRequest = RwSubSystemInfo*(__cdecl*)(RwDevice * device, std::int32_t requestId, RwSubSystemInfo* pOut, void* pInOut, std::int32_t numIn);
static RwSubSystemInfo* RwEngineGetSubSystemInfo_Hooked(RwSubSystemInfo* subSystemInfo, std::int32_t subSystemIndex)
{
    auto* rwGlobals = *(RwGlobals**)CLASS_RwGlobals;
    auto* rwDeviceSystemRequestFunc = (rwDeviceSystemRequest)(FUNC_rwDeviceSystemRequest);
    if (!rwDeviceSystemRequestFunc(&rwGlobals->dOpenDevice, 14, subSystemInfo, nullptr, subSystemIndex))
        return nullptr;

    auto* pDxDevice = *(IDirect3D9**)CLASS_IDirect3D9;
    if (!pDxDevice)
        return subSystemInfo;

    D3DADAPTER_IDENTIFIER9 identifier;
    if (FAILED(pDxDevice->GetAdapterIdentifier(subSystemIndex, 0, &identifier)))
        return subSystemInfo;

    static const auto friendlyNames = GetFriendlyMonitorNamesForDevicePaths();

    // If we can't find the friendly name, either because it doesn't exist or we're on an ancient Windows, fall back to the device name
    auto it = friendlyNames.find(identifier.DeviceName);
    if (it != friendlyNames.end())
    {
        strncpy_s(subSystemInfo->name, it->second.c_str(), _TRUNCATE);
    }
    else
    {
        strncpy_s(subSystemInfo->name, identifier.Description, _TRUNCATE);
    }

    return subSystemInfo;
}

INT_PTR CALLBACK CustomDlgProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto* orgDialogFunc = (DLGPROC)FUNC_DialogFunc;
    if (msg != WM_INITDIALOG)
        return orgDialogFunc(window, msg, wParam, lParam);

    orgDialogFunc(window, msg, wParam, lParam);

    // Set Icon
    HMODULE hGameModule = GetModuleHandle(nullptr);
    SendMessage(window, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(LoadIcon(hGameModule, MAKEINTRESOURCE(100))));

    // Make the dialog visible in the task bar
    // https://stackoverflow.com/a/1462811
    SetWindowLongPtr(window, GWL_EXSTYLE, WS_EX_APPWINDOW);
    ShowWindow(window, SW_HIDE);
    ShowWindow(window, SW_SHOW);
    return FALSE;
}

void CMultiplayerSA::InitHooks_DeviceSelection()
{
    // 0x746239 -> Exact address where the original DialogFunc address is being pushed as an argument to DialogBoxParamA(),
    // we're replacing it with out own proxy function
    MemPut<DLGPROC>(NUM_DialogFuncStackPushAddress, (DLGPROC)&CustomDlgProc);
    HookInstall(FUNC_RwEngineGetSubSystemInfo, (DWORD)RwEngineGetSubSystemInfo_Hooked, 6);
}
