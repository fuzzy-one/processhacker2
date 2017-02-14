#ifndef _SETUP_H
#define _SETUP_H

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "WindowsCodecs.lib")

#define CINTERFACE
#define COBJMACROS

#include <ph.h>
#include <guisup.h>
#include <prsht.h>

#include <WindowsX.h>
#include <Wincodec.h>
#include <uxtheme.h>
#include <Shlwapi.h>
#include <shlobj.h>

#include "..\resource.h"

// PropertySheet Control IDs
#define IDD_PROPSHEET_ID            1006  // ID of the propsheet dialog template in comctl32.dll
#define IDC_PROPSHEET_CANCEL        0x0002
#define IDC_PROPSHEET_APPLYNOW      0x3021
#define IDC_PROPSHEET_DLGFRAME      0x3022
#define IDC_PROPSHEET_BACK          0x3023
#define IDC_PROPSHEET_NEXT          0x3024
#define IDC_PROPSHEET_FINISH        0x3025
#define IDC_PROPSHEET_DIVIDER       0x3026
#define IDC_PROPSHEET_TOPDIVIDER    0x3027

// Debug Macro
#ifdef _DEBUG
#define DEBUG_MSG(Format, ...) \
{ \
    PPH_STRING debugString = PhFormatString(Format, __VA_ARGS__); \
    if (debugString) \
    {                \
        OutputDebugString(debugString->Buffer); \
        PhDereferenceObject(debugString); \
    } \
}
#else
#define DEBUG_MSG(Format, ...)
#endif

extern PPH_STRING SetupInstallPath;

INT_PTR CALLBACK PropSheetPage1_WndProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _Inout_ WPARAM wParam,
    _Inout_ LPARAM lParam
    );

INT_PTR CALLBACK PropSheetPage2_WndProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _Inout_ WPARAM wParam,
    _Inout_ LPARAM lParam
    );

INT_PTR CALLBACK PropSheetPage3_WndProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _Inout_ WPARAM wParam,
    _Inout_ LPARAM lParam
    );

INT_PTR CALLBACK PropSheetPage4_WndProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _Inout_ WPARAM wParam,
    _Inout_ LPARAM lParam
    );

INT_PTR CALLBACK PropSheetPage5_WndProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _Inout_ WPARAM wParam,
    _Inout_ LPARAM lParam
    );

_Check_return_
BOOLEAN ProcessHackerShutdown(
    VOID
    );
_Check_return_
BOOLEAN RemoveAppCompatEntries(
    VOID
    );

_Check_return_
ULONG UninstallKph(
    _In_ BOOLEAN Kph2Uninstall
    );

BOOLEAN SetupResetCurrentInstall(
    _In_ PVOID Arguments
    );
BOOLEAN SetupExtractBuild(
    _In_ PVOID Arguments
    );

VOID SubclassButton(HWND WindowHandle);

#endif