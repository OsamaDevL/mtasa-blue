/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        core/CVideoModeManager.h
 *  PURPOSE:     Header file for the video mode manager class
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

enum
{
    VIDEO_MODE_FULLSCREEN = -1,
    VIDEO_MODE_WINDOWED = 0,
};

// A monitor the game can start on, as listed in Settings > Video and the monitor selection dialog
struct SAdapterMonitor
{
    int     iAdapter;       // Direct3D adapter index, the same as GTA's
    SString strDeviceName;  // e.g. \\.\DISPLAY2
    SString strName;        // Friendly name ("DELL U2720Q"), or the adapter's description if there is none
    RECT    rcDesktop;      // Position in the Windows desktop layout
    int     iWidth;         // Current desktop resolution
    int     iHeight;
    bool    bPrimary;
};

class CVideoModeManagerInterface
{
public:
    virtual ~CVideoModeManagerInterface() {}

    virtual void PreCreateDevice(D3DPRESENT_PARAMETERS* pp) = 0;
    virtual void PostCreateDevice(IDirect3DDevice9* pD3DDevice, D3DPRESENT_PARAMETERS* pp) = 0;
    virtual void PreReset(D3DPRESENT_PARAMETERS* pp) = 0;
    virtual void PostReset(D3DPRESENT_PARAMETERS* pp) = 0;
    virtual void GetNextVideoMode(int& iOutNextVideoMode, bool& bOutNextWindowed, bool& bOutNextFullScreenMinimize, int& iNextFullscreenStyle) = 0;
    virtual bool SetVideoMode(int nextVideoMode, bool bNextWindowed, bool bNextFullScreenMinimize, int iNextFullscreenStyle) = 0;
    virtual bool IsWindowed() = 0;
    virtual bool IsMultiMonitor() = 0;
    virtual bool IsMinimizeEnabled() = 0;
    virtual void OnGainFocus() = 0;
    virtual void OnLoseFocus() = 0;
    virtual void OnPaint() = 0;
    virtual bool GetRequiredDisplayResolution(int& iOutWidth, int& iOutHeight, int& iOutColorBits, int& iOutAdapterIndex) = 0;
    virtual int  GetFullScreenStyle() = 0;
    virtual bool IsDisplayModeWindowed() = 0;
    virtual bool IsDisplayModeFullScreenWindow() = 0;

    // Monitor ("adapter") the game starts on. It is saved by name in the display_monitor cvar
    // (e.g. \\.\DISPLAY2) as well as by index in display_resolution, because the index of a
    // monitor can change when monitors are plugged in or out.
    virtual int     GetAdapterCount() = 0;
    virtual SString GetAdapterDeviceName(int iAdapter) = 0;
    virtual int     GetCurrentAdapter() = 0;
    virtual int     GetNextAdapter() = 0;
    virtual bool    SetNextAdapter(int iAdapter) = 0;  // Returns true if a restart is needed
    virtual bool    IsSavedMonitorMissing() = 0;

    virtual std::vector<SAdapterMonitor> GetMonitors() = 0;
};

CVideoModeManagerInterface* GetVideoModeManager();
