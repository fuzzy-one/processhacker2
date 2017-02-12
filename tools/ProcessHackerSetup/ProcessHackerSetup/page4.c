#include <setup.h>
#include <appsup.h>

NTSTATUS SetupDownloadInstallThread(
    _In_ PVOID Arguments
    )
{
    //if (!SetupDownloadBuild(Arguments))
    //    goto error;

    if (!SetupResetCurrentInstall(Arguments))
        goto CleanupExit;

    if (!SetupExtractBuild(Arguments))
        goto CleanupExit;

    PostMessage(Arguments, PSM_SETCURSELID, 0, IDD_DIALOG5);
    return STATUS_SUCCESS;

CleanupExit:
    PostMessage(Arguments, PSM_SETCURSELID, 0, IDD_DIALOG4); // Retry download
    return STATUS_FAIL_CHECK;
}

BOOL PropSheetPage4_OnInitDialog(
    _In_ HWND hwndDlg,
    _In_ HWND hwndFocus,
    _Inout_ LPARAM lParam
    )
{
    InitializeFont(GetDlgItem(hwndDlg, IDC_MAINHEADER), -17, FW_SEMIBOLD);
    InitializeFont(GetDlgItem(hwndDlg, IDC_MAINHEADER1), -12, FW_SEMIBOLD);

    //SetWindowSubclass(GetDlgItem(hwndDlg, IDC_PROGRESS1), SubclassWindowProc, IDC_PROGRESS1, 0);

    // Enable the themed dialog background texture.
    EnableThemeDialogTexture(hwndDlg, ETDT_ENABLETAB);

    return TRUE;
}

BOOL PropSheetPage4_OnNotify(
    _In_ HWND hwndDlg,
    _In_ INT idCtrl,
    _Inout_ LPNMHDR lpNmh
    )
{
    LPPSHNOTIFY pageNotify = (LPPSHNOTIFY)lpNmh;

    switch (pageNotify->hdr.code)
    {
    case PSN_SETACTIVE:
        {
            HWND hwPropSheet = pageNotify->hdr.hwndFrom;

            // Disable Next/Back buttons
            PropSheet_SetWizButtons(hwPropSheet, 0);

            PhCreateThread(0, SetupDownloadInstallThread, hwPropSheet);
        }
        break;
    case PSN_QUERYCANCEL:
        {
            //if (UpdateResetState == InstallStateResetting || UpdateResetState == InstallStateInstalling)

            //PropSheet_CancelToClose(GetParent(hwndDlg));
            EnableMenuItem(GetSystemMenu(GetParent(hwndDlg), FALSE), SC_CLOSE, MF_GRAYED);
            EnableMenuItem(GetSystemMenu(GetParent(hwndDlg), FALSE), SC_CLOSE, MF_ENABLED);

            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LPARAM)TRUE);
            return TRUE;
        }
        break;
    case PSN_KILLACTIVE:
        {
      
        }
        break;
    }

    return FALSE;
}

INT_PTR CALLBACK PropSheetPage4_WndProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _Inout_ WPARAM wParam,
    _Inout_ LPARAM lParam
    )
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, PropSheetPage4_OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, PropSheetPage4_OnNotify);
    }

    return FALSE;
}