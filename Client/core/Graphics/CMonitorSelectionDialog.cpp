/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        core/Graphics/CMonitorSelectionDialog.cpp
 *  PURPOSE:     MTA's monitor selection dialog, shown instead of GTA's device selection dialog
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

//
// GTA asks which monitor to use with a small dialog from 2004 that only lists graphics adapter
// names. This replaces it with one that names the monitors, shows where they are in the Windows
// desktop layout (click one to pick it), and can remember the choice.
//
// It runs before the game's Direct3D device exists, so it can't use MTA's in-game GUI; it's a
// normal Win32 dialog. It must stay a standard dialog window (#32770): the loader recognises
// that class as the device selection dialog, keeps it on top of the splash screen and doesn't
// treat the wait as a stuck startup (see IsDeviceSelectionDialogOpen in loader/Utils.cpp).
//
// The layout is an in-memory dialog template in dialog units with "Segoe UI" 9, so Windows
// scales it for the font and DPI, and no resource file is needed.
//

#include "StdInc.h"
#include "CMonitorSelectionDialog.h"
#include <algorithm>

namespace
{
    enum
    {
        IDC_PROMPT = 1001,
        IDC_PREVIEW,
        IDC_LIST,
        IDC_REMEMBER,
    };

    // Accent for the selected monitor (MTA orange)
    const COLORREF kSelectedFill = RGB(232, 124, 20);
    const COLORREF kSelectedText = RGB(255, 255, 255);
    const COLORREF kMonitorFill = RGB(226, 229, 233);
    const COLORREF kMonitorText = RGB(60, 60, 60);
    const COLORREF kMonitorBorder = RGB(120, 124, 130);

    struct SDialogState
    {
        const std::vector<SMonitorDialogEntry>* pEntries;
        const SMonitorDialogText*               pText;
        HICON                                   hIcon;
        int                                     iSelected;
        bool                                    bRemember;
        std::vector<RECT>                       previewRects;  // Monitor rectangles as last drawn, for clicks
    };

    /////////////////////////////////////////////////////////////
    //
    // In-memory DLGTEMPLATEEX builder
    //
    /////////////////////////////////////////////////////////////
    class CDialogTemplate
    {
    public:
        void Begin(const std::wstring& strTitle, DWORD dwStyle, DWORD dwExStyle, short cx, short cy, WORD wItemCount)
        {
            AddWord(1);       // dlgVer
            AddWord(0xFFFF);  // signature: DLGTEMPLATEEX
            AddDword(0);      // helpID
            AddDword(dwExStyle);
            AddDword(dwStyle);
            AddWord(wItemCount);
            AddWord(0);  // x
            AddWord(0);  // y
            AddWord(static_cast<WORD>(cx));
            AddWord(static_cast<WORD>(cy));
            AddWord(0);  // no menu
            AddWord(0);  // default dialog class (#32770)
            AddString(strTitle);
            AddWord(9);    // point size (DS_SETFONT)
            AddWord(400);  // weight
            AddByte(0);    // italic
            AddByte(DEFAULT_CHARSET);
            AddString(L"Segoe UI");
        }

        void AddItem(WORD wClassAtom, const std::wstring& strText, DWORD dwId, DWORD dwStyle, short x, short y, short cx, short cy, DWORD dwExStyle = 0)
        {
            Align(4);
            AddDword(0);  // helpID
            AddDword(dwExStyle);
            AddDword(dwStyle | WS_CHILD | WS_VISIBLE);
            AddWord(static_cast<WORD>(x));
            AddWord(static_cast<WORD>(y));
            AddWord(static_cast<WORD>(cx));
            AddWord(static_cast<WORD>(cy));
            AddDword(dwId);
            AddWord(0xFFFF);  // predefined class by atom
            AddWord(wClassAtom);
            AddString(strText);
            AddWord(0);  // no creation data
        }

        const DLGTEMPLATE* Get()
        {
            Align(4);
            return reinterpret_cast<const DLGTEMPLATE*>(m_Data.data());
        }

    private:
        void AddByte(BYTE b) { m_Data.push_back(b); }
        void AddWord(WORD w)
        {
            AddByte(LOBYTE(w));
            AddByte(HIBYTE(w));
        }
        void AddDword(DWORD dw)
        {
            AddWord(LOWORD(dw));
            AddWord(HIWORD(dw));
        }
        void AddString(const std::wstring& str)
        {
            for (wchar_t c : str)
                AddWord(static_cast<WORD>(c));
            AddWord(0);
        }
        void Align(size_t uiBoundary)
        {
            while (m_Data.size() % uiBoundary)
                AddByte(0);
        }

        // DWORD storage keeps the template DWORD-aligned in memory, as DialogBoxIndirect requires
        std::vector<BYTE> m_Data;
    };

    const WORD kButton = 0x0080;
    const WORD kStatic = 0x0082;
    const WORD kListBox = 0x0083;

    /////////////////////////////////////////////////////////////
    //
    // Preview: the monitors as laid out in Windows, scaled to fit the control
    //
    /////////////////////////////////////////////////////////////
    void DrawPreview(SDialogState& state, const DRAWITEMSTRUCT& dis)
    {
        const RECT& rcControl = dis.rcItem;
        const int   iWidth = rcControl.right - rcControl.left;
        const int   iHeight = rcControl.bottom - rcControl.top;

        // Draw off-screen to avoid flicker when the selection changes
        HDC     hdc = CreateCompatibleDC(dis.hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(dis.hDC, iWidth, iHeight);
        HGDIOBJ hOldBitmap = SelectObject(hdc, hBitmap);

        RECT rcBack = {0, 0, iWidth, iHeight};
        FillRect(hdc, &rcBack, GetSysColorBrush(COLOR_WINDOW));
        FrameRect(hdc, &rcBack, GetSysColorBrush(COLOR_3DSHADOW));

        const auto& entries = *state.pEntries;
        state.previewRects.assign(entries.size(), RECT{});

        if (!entries.empty())
        {
            // Bounding box of the whole desktop
            RECT rcAll = entries[0].rcDesktop;
            for (const auto& entry : entries)
                UnionRect(&rcAll, &rcAll, &entry.rcDesktop);

            const int    iMargin = 10;
            const double dScaleX = static_cast<double>(iWidth - 2 * iMargin) / (std::max)(1L, rcAll.right - rcAll.left);
            const double dScaleY = static_cast<double>(iHeight - 2 * iMargin) / (std::max)(1L, rcAll.bottom - rcAll.top);
            const double dScale = (std::min)(dScaleX, dScaleY);
            const int    iOffsetX = (iWidth - static_cast<int>((rcAll.right - rcAll.left) * dScale)) / 2;
            const int    iOffsetY = (iHeight - static_cast<int>((rcAll.bottom - rcAll.top) * dScale)) / 2;

            SetBkMode(hdc, TRANSPARENT);
            for (size_t i = 0; i < entries.size(); i++)
            {
                const RECT& rcDesktop = entries[i].rcDesktop;
                RECT        rc;
                rc.left = iOffsetX + static_cast<int>((rcDesktop.left - rcAll.left) * dScale) + 2;
                rc.top = iOffsetY + static_cast<int>((rcDesktop.top - rcAll.top) * dScale) + 2;
                rc.right = iOffsetX + static_cast<int>((rcDesktop.right - rcAll.left) * dScale) - 2;
                rc.bottom = iOffsetY + static_cast<int>((rcDesktop.bottom - rcAll.top) * dScale) - 2;
                state.previewRects[i] = rc;

                const bool bSelected = static_cast<int>(i) == state.iSelected;
                HBRUSH     hFill = CreateSolidBrush(bSelected ? kSelectedFill : kMonitorFill);
                HPEN       hPen = CreatePen(PS_SOLID, 1, bSelected ? kSelectedFill : kMonitorBorder);
                HGDIOBJ    hOldBrush = SelectObject(hdc, hFill);
                HGDIOBJ    hOldPen = SelectObject(hdc, hPen);
                Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                SelectObject(hdc, hOldPen);
                SelectObject(hdc, hOldBrush);
                DeleteObject(hPen);
                DeleteObject(hFill);

                // Monitor number, as large as fits
                const int iFontHeight = (std::max)(12, (std::min<int>)((rc.bottom - rc.top) / 2, 48));
                HFONT     hFont = CreateFontW(-iFontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                              CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
                HGDIOBJ   hOldFont = SelectObject(hdc, hFont);
                SetTextColor(hdc, bSelected ? kSelectedText : kMonitorText);
                std::wstring strNumber = std::to_wstring(i + 1);
                DrawTextW(hdc, strNumber.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(hdc, hOldFont);
                DeleteObject(hFont);
            }
        }

        BitBlt(dis.hDC, rcControl.left, rcControl.top, iWidth, iHeight, hdc, 0, 0, SRCCOPY);
        SelectObject(hdc, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdc);
    }

    void SelectEntry(HWND hDlg, SDialogState& state, int iEntry)
    {
        if (iEntry < 0 || iEntry >= static_cast<int>(state.pEntries->size()))
            return;
        state.iSelected = iEntry;
        SendDlgItemMessageW(hDlg, IDC_LIST, LB_SETCURSEL, iEntry, 0);
        InvalidateRect(GetDlgItem(hDlg, IDC_PREVIEW), nullptr, FALSE);
    }

    // Monitor under the mouse in the preview, or -1
    int HitTestPreview(HWND hDlg, const SDialogState& state)
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(GetDlgItem(hDlg, IDC_PREVIEW), &pt);
        for (size_t i = 0; i < state.previewRects.size(); i++)
        {
            if (PtInRect(&state.previewRects[i], pt))
                return static_cast<int>(i);
        }
        return -1;
    }

    // Open centred on the preselected monitor, so it shows where the player is looking
    void CenterOnMonitor(HWND hDlg, const RECT& rcMonitor)
    {
        RECT rcDlg;
        GetWindowRect(hDlg, &rcDlg);
        const int iDlgWidth = rcDlg.right - rcDlg.left;
        const int iDlgHeight = rcDlg.bottom - rcDlg.top;
        const int x = rcMonitor.left + (rcMonitor.right - rcMonitor.left - iDlgWidth) / 2;
        const int y = rcMonitor.top + (rcMonitor.bottom - rcMonitor.top - iDlgHeight) / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        auto* pState = reinterpret_cast<SDialogState*>(GetWindowLongPtrW(hDlg, DWLP_USER));

        switch (uMsg)
        {
            case WM_INITDIALOG:
            {
                pState = reinterpret_cast<SDialogState*>(lParam);
                SetWindowLongPtrW(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pState));

                if (pState->hIcon)
                {
                    SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(pState->hIcon));
                    SendMessageW(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(pState->hIcon));
                }

                HWND hList = GetDlgItem(hDlg, IDC_LIST);
                for (size_t i = 0; i < pState->pEntries->size(); i++)
                {
                    const auto&  entry = (*pState->pEntries)[i];
                    std::wstring strLine = std::to_wstring(i + 1) + L":  " + entry.strName;
                    if (!entry.strDetails.empty())
                        strLine += L"   (" + entry.strDetails + L")";
                    SendMessageW(hList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(strLine.c_str()));
                }
                SelectEntry(hDlg, *pState, pState->iSelected);
                CheckDlgButton(hDlg, IDC_REMEMBER, pState->bRemember ? BST_CHECKED : BST_UNCHECKED);
                CenterOnMonitor(hDlg, (*pState->pEntries)[pState->iSelected].rcDesktop);

                // The game window isn't up yet, so make sure the dialog isn't hidden behind something
                SetForegroundWindow(hDlg);
                SetFocus(hList);
                return FALSE;  // Focus was set above
            }

            case WM_DRAWITEM:
                if (pState && wParam == IDC_PREVIEW)
                {
                    DrawPreview(*pState, *reinterpret_cast<const DRAWITEMSTRUCT*>(lParam));
                    return TRUE;
                }
                break;

            case WM_COMMAND:
                if (!pState)
                    break;
                switch (LOWORD(wParam))
                {
                    case IDC_PREVIEW:
                        if (HIWORD(wParam) == STN_CLICKED || HIWORD(wParam) == STN_DBLCLK)
                        {
                            int iHit = HitTestPreview(hDlg, *pState);
                            if (iHit >= 0)
                            {
                                SelectEntry(hDlg, *pState, iHit);
                                if (HIWORD(wParam) == STN_DBLCLK)
                                    PostMessageW(hDlg, WM_COMMAND, IDOK, 0);
                            }
                        }
                        return TRUE;

                    case IDC_LIST:
                        if (HIWORD(wParam) == LBN_SELCHANGE)
                        {
                            int iSel = static_cast<int>(SendDlgItemMessageW(hDlg, IDC_LIST, LB_GETCURSEL, 0, 0));
                            SelectEntry(hDlg, *pState, iSel);
                        }
                        else if (HIWORD(wParam) == LBN_DBLCLK)
                            PostMessageW(hDlg, WM_COMMAND, IDOK, 0);
                        return TRUE;

                    case IDOK:
                        pState->bRemember = IsDlgButtonChecked(hDlg, IDC_REMEMBER) == BST_CHECKED;
                        EndDialog(hDlg, IDOK);
                        return TRUE;

                    case IDCANCEL:  // Cancel button, Esc and the title bar's X
                        EndDialog(hDlg, IDCANCEL);
                        return TRUE;
                }
                break;
        }
        return FALSE;
    }
}  // namespace

EMonitorDialogResult ShowMonitorSelectionDialog(const std::vector<SMonitorDialogEntry>& entries, int iPreselect, bool bRemember, const SMonitorDialogText& text,
                                                HICON hIcon, int& iOutChoice, bool& bOutRemember)
{
    if (entries.empty())
        return EMonitorDialogResult::Failed;

    SDialogState state{};
    state.pEntries = &entries;
    state.pText = &text;
    state.hIcon = hIcon;
    state.iSelected = (iPreselect >= 0 && iPreselect < static_cast<int>(entries.size())) ? iPreselect : 0;
    state.bRemember = bRemember;

    // Layout in dialog units
    const short     cxDialog = 300;
    const short     cyDialog = 202;
    CDialogTemplate dlg;
    dlg.Begin(text.strTitle, DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU, WS_EX_APPWINDOW, cxDialog, cyDialog, 6);
    dlg.AddItem(kStatic, text.strPrompt, IDC_PROMPT, SS_LEFT, 7, 7, 286, 10);
    dlg.AddItem(kStatic, L"", IDC_PREVIEW, SS_OWNERDRAW | SS_NOTIFY, 7, 20, 286, 86);
    dlg.AddItem(kListBox, L"", IDC_LIST, LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP, 7, 111, 286, 46, WS_EX_CLIENTEDGE);
    dlg.AddItem(kButton, text.strRemember, IDC_REMEMBER, BS_AUTOCHECKBOX | WS_TABSTOP, 7, 162, 286, 12);
    dlg.AddItem(kButton, text.strOk, IDOK, BS_DEFPUSHBUTTON | WS_TABSTOP, 183, 181, 52, 14);
    dlg.AddItem(kButton, text.strCancel, IDCANCEL, BS_PUSHBUTTON | WS_TABSTOP, 241, 181, 52, 14);

    INT_PTR result = DialogBoxIndirectParamW(GetModuleHandleW(nullptr), dlg.Get(), nullptr, DialogProc, reinterpret_cast<LPARAM>(&state));
    if (result == -1 || result == 0)
        return EMonitorDialogResult::Failed;
    if (result != IDOK)
        return EMonitorDialogResult::Cancelled;

    iOutChoice = state.iSelected;
    bOutRemember = state.bRemember;
    return EMonitorDialogResult::Chosen;
}
