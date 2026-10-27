/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        core/Graphics/CMonitorSelectionDialog.h
 *  PURPOSE:     MTA's monitor selection dialog, shown instead of GTA's device selection dialog
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include <windows.h>
#include <string>
#include <vector>

// The dialog only knows about what it shows: CCore::ShowMonitorSelectionDialog builds the list
// from the Direct3D adapters and saves the choice. Keeping it free of MTA state means it can be
// built and tried on its own.

struct SMonitorDialogEntry
{
    std::wstring strName;     // e.g. "DELL U2720Q"
    std::wstring strDetails;  // e.g. "2560 x 1440, primary"
    RECT         rcDesktop;   // Position in the Windows desktop layout, for the preview
};

struct SMonitorDialogText
{
    std::wstring strTitle;
    std::wstring strPrompt;
    std::wstring strRemember;
    std::wstring strOk;
    std::wstring strCancel;
};

enum class EMonitorDialogResult
{
    Chosen,
    Cancelled,  // The player closed the dialog: quit, like GTA's dialog does
    Failed,     // The dialog couldn't be shown: use GTA's dialog instead
};

// Modal. iPreselect is the entry selected when it opens and bRemember the initial state of
// "Remember my choice"; iOutChoice and bOutRemember are what the player picked.
EMonitorDialogResult ShowMonitorSelectionDialog(const std::vector<SMonitorDialogEntry>& entries, int iPreselect, bool bRemember, const SMonitorDialogText& text,
                                                HICON hIcon, int& iOutChoice, bool& bOutRemember);
