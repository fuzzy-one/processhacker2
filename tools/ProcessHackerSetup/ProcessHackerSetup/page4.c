#include <setup.h>
#include <appsup.h>
#include "lib\miniz\miniz.h"

typedef struct _SETUP_EXTRACT_FILE
{
    PSTR FileName;
    PWSTR ExtractFileName;
} SETUP_EXTRACT_FILE, *PSETUP_EXTRACT_FILE;

SETUP_EXTRACT_FILE SetupFiles_X32[] =
{
    { "CHANGELOG.txt",                                L"CHANGELOG.txt" },
    { "COPYRIGHT.txt",                                L"COPYRIGHT.txt" },
    { "LICENSE.txt",                                  L"LICENSE.txt" },
    { "README.txt",                                   L"README.txt" },

    { "Release32/peview.exe",                         L"peview.exe" },
    { "Release32/ProcessHacker.exe",                  L"ProcessHacker.exe" },
    { "Release32/ProcessHacker.sig",                  L"ProcessHacker.sig" },
    { "Release32/kprocesshacker.sys",                 L"kprocesshacker.sys" },

    { "Release32/plugins/CommonUtil.dll",             L"plugins\\CommonUtil.dll" },
    { "Release32/plugins/DotNetTools.dll",            L"plugins\\DotNetTools.dll" },
    { "Release32/plugins/ExtendedNotifications.dll",  L"plugins\\ExtendedNotifications.dll" },
    { "Release32/plugins/ExtendedServices.dll",       L"plugins\\ExtendedServices.dll" },
    { "Release32/plugins/ExtendedTools.dll",          L"plugins\\ExtendedTools.dll" },
    { "Release32/plugins/HardwareDevices.dll",        L"plugins\\HardwareDevices.dll" },
    { "Release32/plugins/NetworkTools.dll",           L"plugins\\NetworkTools.dll" },
    { "Release32/plugins/OnlineChecks.dll",           L"plugins\\OnlineChecks.dll" },
    { "Release32/plugins/SbieSupport.dll",            L"plugins\\SbieSupport.dll" },
    { "Release32/plugins/ToolStatus.dll",             L"plugins\\ToolStatus.dll" },
    { "Release32/plugins/Updater.dll",                L"plugins\\Updater.dll" },
    { "Release32/plugins/UserNotes.dll",              L"plugins\\UserNotes.dll" },
    { "Release32/plugins/WindowExplorer.dll",         L"plugins\\WindowExplorer.dll" },
};

SETUP_EXTRACT_FILE SetupFiles_X64[] =
{
    { "CHANGELOG.txt",                                L"CHANGELOG.txt" },
    { "COPYRIGHT.txt",                                L"COPYRIGHT.txt" },
    { "LICENSE.txt",                                  L"LICENSE.txt" },
    { "README.txt",                                   L"README.txt" },

    { "Release64/peview.exe",                         L"peview.exe" },
    { "Release64/ProcessHacker.exe",                  L"ProcessHacker.exe" },
    { "Release64/ProcessHacker.sig",                  L"ProcessHacker.sig" },
    { "Release64/kprocesshacker.sys",                 L"kprocesshacker.sys" },
    
    { "Release64/plugins/CommonUtil.dll",             L"plugins\\CommonUtil.dll" },
    { "Release64/plugins/DotNetTools.dll",            L"plugins\\DotNetTools.dll" },
    { "Release64/plugins/ExtendedNotifications.dll",  L"plugins\\ExtendedNotifications.dll" },
    { "Release64/plugins/ExtendedServices.dll",       L"plugins\\ExtendedServices.dll" },
    { "Release64/plugins/ExtendedTools.dll",          L"plugins\\ExtendedTools.dll" },
    { "Release64/plugins/HardwareDevices.dll",        L"plugins\\HardwareDevices.dll" },
    { "Release64/plugins/NetworkTools.dll",           L"plugins\\NetworkTools.dll" },
    { "Release64/plugins/OnlineChecks.dll",           L"plugins\\OnlineChecks.dll" },
    { "Release64/plugins/SbieSupport.dll",            L"plugins\\SbieSupport.dll" },
    { "Release64/plugins/ToolStatus.dll",             L"plugins\\ToolStatus.dll" },
    { "Release64/plugins/Updater.dll",                L"plugins\\Updater.dll" },
    { "Release64/plugins/UserNotes.dll",              L"plugins\\UserNotes.dll" },
    { "Release64/plugins/WindowExplorer.dll",         L"plugins\\WindowExplorer.dll" },

    { "Release32/ProcessHacker.exe",                  L"x86\\ProcessHacker.exe" },
    { "Release32/plugins/DotNetTools.dll",            L"x86\\plugins\\DotNetTools.dll" },
};



BOOLEAN SetupExtractBuild(
    _In_ PVOID Arguments
    )
{
    mz_uint64 totalLength = 0;
    mz_bool status = MZ_FALSE;
    mz_zip_archive zip_archive = { 0 };
    ULONG extractDataLength;
    ULONG resourceLength;
    HRSRC resourceHandle = NULL;
    HGLOBAL resourceData;
    PVOID resourceBuffer;
    PSETUP_EXTRACT_FILE extractData;

    if (USER_SHARED_DATA->NativeProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    {
        extractDataLength = ARRAYSIZE(SetupFiles_X64);
        extractData = SetupFiles_X64;
    }
    else
    {
        extractDataLength = ARRAYSIZE(SetupFiles_X32);
        extractData = SetupFiles_X32;
    }

    if (!CreateDirectoryPath(SetupInstallPath))
        goto CleanupExit;

    if (!(resourceHandle = FindResource(PhLibImageBase, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA)))
        goto CleanupExit;

    resourceLength = SizeofResource(PhLibImageBase, resourceHandle);

    if (!(resourceData = LoadResource(PhLibImageBase, resourceHandle)))
        goto CleanupExit;

    if (!(resourceBuffer = LockResource(resourceData)))
        goto CleanupExit;

    if (!(status = mz_zip_reader_init_mem(&zip_archive, resourceBuffer, resourceLength, 0)))
        goto CleanupExit;

    SendMessage(GetDlgItem(Arguments, IDC_INSTALL_PROGRESS), PBM_SETRANGE32, 0, (LPARAM)extractDataLength);

    for (ULONG i = 0; i < extractDataLength; i++)
    {
        mz_ulong file_crc32 = MZ_CRC32_INIT;
        IO_STATUS_BLOCK isb;
        HANDLE fileHandle;
        ULONG indexOfFileName = -1;
        PPH_STRING fullSetupPath;
        PPH_STRING extractPath;
        mz_uint file_index;
        mz_zip_archive_file_stat file_stat;
        size_t bufferLength = 0;

        file_index = mz_zip_reader_locate_file(
            &zip_archive,
            extractData[i].FileName,
            NULL,
            0
            );

        if (file_index == -1)
            goto CleanupExit;

        if (!mz_zip_reader_file_stat(&zip_archive, file_index, &file_stat))
            goto CleanupExit;

        extractPath = PhConcatStrings(2,
            SetupInstallPath->Buffer,
            extractData[i].ExtractFileName
            );

        // Create the directory if it does not exist.
        if (fullSetupPath = PhGetFullPath(extractPath->Buffer, &indexOfFileName))
        {
            PPH_STRING directoryPath;

            if (indexOfFileName == -1)
                goto CleanupExit;

            if (directoryPath = PhSubstring(fullSetupPath, 0, indexOfFileName))
            {
                if (!CreateDirectoryPath(directoryPath))
                {
                    PhDereferenceObject(directoryPath);
                    PhDereferenceObject(fullSetupPath);
                    goto CleanupExit;
                }

                PhDereferenceObject(directoryPath);
            }

            PhDereferenceObject(fullSetupPath);
        }

        STATUS_MSG(L"Extracting: %s", extractData[i].ExtractFileName);
        SetWindowText(
            GetDlgItem(Arguments, IDC_INSTALL_STATUS), 
            PhFormatString(L"Extracting: %s", extractData[i].ExtractFileName)->Buffer
            );
        SetWindowText(
            GetDlgItem(Arguments, IDC_INSTALL_SUBSTATUS),
            PhFormatString(L"Progress: %lu of %lu (%.2f%%)", i, extractDataLength, (FLOAT)((double)i / (double)extractDataLength) * 100)->Buffer
            );

        // TODO: Move existing folder items.
        if (!NT_SUCCESS(PhCreateFileWin32(
            &fileHandle,
            extractPath->Buffer,
            FILE_GENERIC_READ | FILE_GENERIC_WRITE,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            FILE_OVERWRITE_IF,
            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
            )))
        {
            DEBUG_MSG(L"PhCreateFileWin32 Failed\r\n");
            goto CleanupExit;
        }

        PVOID buffer = mz_zip_reader_extract_to_heap(
            &zip_archive,
            file_index,
            &bufferLength,
            0
            );

        // Update the hash
        file_crc32 = mz_crc32(file_crc32, buffer, bufferLength);

        if (file_crc32 != file_stat.m_crc32)
            goto CleanupExit;

        // Write the downloaded bytes to disk.
        if (!NT_SUCCESS(NtWriteFile(
            fileHandle,
            NULL,
            NULL,
            NULL,
            &isb,
            buffer,
            bufferLength,
            NULL,
            NULL
            )))
        {
            goto CleanupExit;
        }

        if (isb.Information != bufferLength)
            goto CleanupExit;

        totalLength += (mz_uint64)bufferLength;
        //SendMessage(GetDlgItem(Arguments, IDC_INSTALL_PROGRESS), PBM_SETPOS, (WPARAM)totalLength, 0);
        SendMessage(GetDlgItem(Arguments, IDC_INSTALL_PROGRESS), PBM_SETPOS, (WPARAM)i, 0);

        NtClose(fileHandle);
        mz_free(buffer);

#ifdef _DEBUG
        Sleep(1000);
#endif
    }

CleanupExit:

    mz_zip_reader_end(&zip_archive);

    if (resourceHandle)
    {
        FreeResource(resourceHandle);
    }

    Sleep(2000);

    return TRUE;
}

NTSTATUS SetupProgressThread(
    _In_ PVOID Arguments
    )
{
    //if (!SetupDownloadBuild(Arguments))

    if (!SetupResetCurrentInstall(Arguments))
        goto CleanupExit;

    if (!SetupExtractBuild(Arguments))
        goto CleanupExit;

    PostMessage(GetParent(Arguments), PSM_SETCURSELID, 0, IDD_DIALOG5);
    return STATUS_SUCCESS;

CleanupExit:
    PostMessage(GetParent(Arguments), PSM_SETCURSELID, 0, IDD_DIALOG4); // Retry download
    return STATUS_FAIL_CHECK;
}

static VOID LoadSetupImage(
    _In_ HWND hwndDlg
    )
{
    HBITMAP imageBitmap;

    imageBitmap = LoadPngImageFromResources(MAKEINTRESOURCE(IDB_PNG1));

    // The image control uses a large square frame so that we can use the VS designer easily.
    // Remove the frame style and apply the bitmap style.
    PhSetWindowStyle(
        GetDlgItem(hwndDlg, IDC_PROJECT_ICON),
        SS_BITMAP | SS_BLACKFRAME,
        SS_BITMAP
        );

    SendMessage(
        GetDlgItem(hwndDlg, IDC_PROJECT_ICON),
        STM_SETIMAGE,
        IMAGE_BITMAP,
        (LPARAM)imageBitmap
        );

    DeleteObject(imageBitmap);
}

BOOL PropSheetPage4_OnInitDialog(
    _In_ HWND hwndDlg,
    _In_ HWND hwndFocus,
    _Inout_ LPARAM lParam
    )
{
    InitializeFont(GetDlgItem(hwndDlg, IDC_MAINHEADER), -17, FW_SEMIBOLD);
    InitializeFont(GetDlgItem(hwndDlg, IDC_SUBHEADER), -12, FW_NORMAL);
    InitializeFont(GetDlgItem(hwndDlg, IDC_INSTALL_STATUS), -12, FW_SEMIBOLD);
    InitializeFont(GetDlgItem(hwndDlg, IDC_INSTALL_SUBSTATUS), -12, FW_NORMAL);
    
    LoadSetupImage(hwndDlg);

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

            NtClose(PhCreateThread(0, SetupProgressThread, hwndDlg));
        }
        break;
    case PSN_QUERYCANCEL:
        {
            //if (UpdateResetState == InstallStateResetting || UpdateResetState == InstallStateInstalling)

            //PropSheet_CancelToClose(GetParent(hwndDlg));
            //EnableMenuItem(GetSystemMenu(GetParent(hwndDlg), FALSE), SC_CLOSE, MF_GRAYED);
            //EnableMenuItem(GetSystemMenu(GetParent(hwndDlg), FALSE), SC_CLOSE, MF_ENABLED);

            //SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LPARAM)TRUE);
            //return TRUE;
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