#include <setup.h>
#include <appsup.h>
#include <Netlistmgr.h>

HBITMAP LoadPngImageFromResources(
    _In_ PCWSTR Name
    )
{
    BOOLEAN success = FALSE;
    UINT frameCount = 0;
    ULONG resourceLength = 0;
    HGLOBAL resourceHandle = NULL;
    HRSRC resourceHandleSource = NULL;
    WICInProcPointer resourceBuffer = NULL;
    HDC screenHdc = NULL;
    HDC bufferDc = NULL;
    BITMAPINFO bitmapInfo = { 0 };
    HBITMAP bitmapHandle = NULL;
    PVOID bitmapBuffer = NULL;
    IWICStream* wicStream = NULL;
    IWICBitmapSource* wicBitmapSource = NULL;
    IWICBitmapDecoder* wicDecoder = NULL;
    IWICBitmapFrameDecode* wicFrame = NULL;
    IWICImagingFactory* wicFactory = NULL;
    IWICBitmapScaler* wicScaler = NULL;
    WICPixelFormatGUID pixelFormat;
    WICRect rect = { 0, 0, 164, 164 };

    // Create the ImagingFactory
    if (FAILED(CoCreateInstance(&CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, &wicFactory)))
        goto CleanupExit;

    // Find the resource
    if ((resourceHandleSource = FindResource(PhLibImageBase, Name, L"PNG")) == NULL)
        goto CleanupExit;

    // Get the resource length
    resourceLength = SizeofResource(PhLibImageBase, resourceHandleSource);

    // Load the resource
    if ((resourceHandle = LoadResource(PhLibImageBase, resourceHandleSource)) == NULL)
        goto CleanupExit;

    if ((resourceBuffer = (WICInProcPointer)LockResource(resourceHandle)) == NULL)
        goto CleanupExit;

    // Create the Stream
    if (FAILED(IWICImagingFactory_CreateStream(wicFactory, &wicStream)))
        goto CleanupExit;

    // Initialize the Stream from Memory
    if (FAILED(IWICStream_InitializeFromMemory(wicStream, resourceBuffer, resourceLength)))
        goto CleanupExit;

    if (FAILED(IWICImagingFactory_CreateDecoder(wicFactory, &GUID_ContainerFormatPng, NULL, &wicDecoder)))
        goto CleanupExit;

    if (FAILED(IWICBitmapDecoder_Initialize(wicDecoder, (IStream*)wicStream, WICDecodeMetadataCacheOnLoad)))
        goto CleanupExit;

    // Get the Frame count
    if (FAILED(IWICBitmapDecoder_GetFrameCount(wicDecoder, &frameCount)) || frameCount < 1)
        goto CleanupExit;

    // Get the Frame
    if (FAILED(IWICBitmapDecoder_GetFrame(wicDecoder, 0, &wicFrame)))
        goto CleanupExit;

    // Get the WicFrame image format
    if (FAILED(IWICBitmapFrameDecode_GetPixelFormat(wicFrame, &pixelFormat)))
        goto CleanupExit;

    // Check if the image format is supported:
    if (IsEqualGUID(&pixelFormat, &GUID_WICPixelFormat32bppPRGBA))
    {
        wicBitmapSource = (IWICBitmapSource*)wicFrame;
    }
    else
    {
        IWICFormatConverter* wicFormatConverter = NULL;

        if (FAILED(IWICImagingFactory_CreateFormatConverter(wicFactory, &wicFormatConverter)))
            goto CleanupExit;

        if (FAILED(IWICFormatConverter_Initialize(
            wicFormatConverter,
            (IWICBitmapSource*)wicFrame,
            &GUID_WICPixelFormat32bppPRGBA,
            WICBitmapDitherTypeNone,
            NULL,
            0.0,
            WICBitmapPaletteTypeCustom
            )))
        {
            IWICFormatConverter_Release(wicFormatConverter);
            goto CleanupExit;
        }

        // Convert the image to the correct format:
        IWICFormatConverter_QueryInterface(wicFormatConverter, &IID_IWICBitmapSource, &wicBitmapSource);

        // Cleanup the converter.
        IWICFormatConverter_Release(wicFormatConverter);

        // Dispose the old frame now that the converted frame is in wicBitmapSource.
        IWICBitmapFrameDecode_Release(wicFrame);
    }

    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = rect.Width;
    bitmapInfo.bmiHeader.biHeight = -((LONG)rect.Height);
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    screenHdc = CreateIC(L"DISPLAY", NULL, NULL, NULL);
    bufferDc = CreateCompatibleDC(screenHdc);
    bitmapHandle = CreateDIBSection(screenHdc, &bitmapInfo, DIB_RGB_COLORS, &bitmapBuffer, NULL, 0);

    // Check if it's the same rect as the requested size.
    //if (width != rect.Width || height != rect.Height)
    if (FAILED(IWICImagingFactory_CreateBitmapScaler(wicFactory, &wicScaler)))
        goto CleanupExit;
    if (FAILED(IWICBitmapScaler_Initialize(wicScaler, wicBitmapSource, rect.Width, rect.Height, WICBitmapInterpolationModeFant)))
        goto CleanupExit;
    if (FAILED(IWICBitmapScaler_CopyPixels(wicScaler, &rect, rect.Width * 4, rect.Width * rect.Height * 4, (PBYTE)bitmapBuffer)))
        goto CleanupExit;

    success = TRUE;

CleanupExit:

    if (wicScaler)
        IWICBitmapScaler_Release(wicScaler);

    if (bufferDc)
        DeleteDC(bufferDc);

    if (screenHdc)
        DeleteDC(screenHdc);

    if (wicBitmapSource)
        IWICBitmapSource_Release(wicBitmapSource);

    if (wicStream)
        IWICStream_Release(wicStream);

    if (wicDecoder)
        IWICBitmapDecoder_Release(wicDecoder);

    if (wicFactory)
        IWICImagingFactory_Release(wicFactory);

    if (resourceHandle)
        FreeResource(resourceHandle);

    if (success)
    {
        return bitmapHandle;
    }

    DeleteObject(bitmapHandle);
    return NULL;
}

VOID InitializeFont(
    _In_ HWND ControlHandle,
    _In_ LONG Height,
    _In_ LONG Weight
    )
{
    NONCLIENTMETRICS metrics = { sizeof(NONCLIENTMETRICS) };
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);

    metrics.lfMessageFont.lfHeight = -PhMultiplyDivideSigned(Height, PhGlobalDpi, 72);
    metrics.lfMessageFont.lfWeight = Weight;
    //metrics.lfMessageFont.lfQuality = CLEARTYPE_QUALITY;

    HFONT fontHandle = CreateFontIndirect(&metrics.lfMessageFont);

    SendMessage(ControlHandle, WM_SETFONT, (WPARAM)fontHandle, FALSE);

    //DeleteFont(fontHandle);
}

BOOLEAN ConnectionAvailable(
    VOID
    )
{
    INetworkListManager* networkListManager = NULL;

    // Create an instance of the INetworkListManger COM object.
    if (SUCCEEDED(CoCreateInstance(&CLSID_NetworkListManager, NULL, CLSCTX_ALL, &IID_INetworkListManager, &networkListManager)))
    {
        VARIANT_BOOL isConnected = VARIANT_FALSE;
        VARIANT_BOOL isConnectedInternet = VARIANT_FALSE;

        // Query the relevant properties.
        INetworkListManager_get_IsConnected(networkListManager, &isConnected);
        INetworkListManager_get_IsConnectedToInternet(networkListManager, &isConnectedInternet);

        // Cleanup the INetworkListManger COM objects.
        INetworkListManager_Release(networkListManager);

        // Check if Windows is connected to a network and it's connected to the internet.
        if (isConnected == VARIANT_TRUE && isConnectedInternet == VARIANT_TRUE)
            return TRUE;

        // We're not connected to anything
    }

    return FALSE;
}

BOOLEAN CreateLink(
    _In_ PWSTR DestFilePath,
    _In_ PWSTR FilePath,
    _In_ PWSTR FileParentDir,
    _In_ PWSTR FileComment
    )
{
    IShellLink* shellLinkPtr = NULL;
    IPersistFile* persistFilePtr = NULL;

    if (FAILED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, &shellLinkPtr)))
        return FALSE;

    if (FAILED(IShellLinkW_QueryInterface(shellLinkPtr, &IID_IPersistFile, &persistFilePtr)))
    {
        IShellLinkW_Release(shellLinkPtr);
        return FALSE;
    }

    // Load existing shell item if it exists...
    //if (SUCCEEDED(IPersistFile_Load(persistFilePtr, DestFilePath, STGM_READ)))
    IShellLinkW_SetDescription(shellLinkPtr, FileComment);
    IShellLinkW_SetWorkingDirectory(shellLinkPtr, FileParentDir);
    IShellLinkW_SetIconLocation(shellLinkPtr, FilePath, 0);

    // Set the shortcut target path...
    if (FAILED(IShellLinkW_SetPath(shellLinkPtr, FilePath)))
    {
        IPersistFile_Release(persistFilePtr);
        IShellLinkW_Release(shellLinkPtr);
        return FALSE;
    }

   // Save the shortcut to the file system...
    if (FAILED(IPersistFile_Save(persistFilePtr, DestFilePath, TRUE)))
    {
        IPersistFile_Release(persistFilePtr);
        IShellLinkW_Release(shellLinkPtr);
        return FALSE;
    }

    IPersistFile_Release(persistFilePtr);
    IShellLinkW_Release(shellLinkPtr);

    return TRUE;
}

BOOLEAN DialogPromptExit(
    _In_ HWND hwndDlg
    )
{
    INT nButtonPressed = 0;

    TASKDIALOGCONFIG tdConfig = { sizeof(TASKDIALOGCONFIG) };
    tdConfig.hwndParent = hwndDlg;
    tdConfig.hInstance = PhLibImageBase;
    tdConfig.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW;
    tdConfig.nDefaultButton = IDNO;
    tdConfig.dwCommonButtons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
    tdConfig.pszMainIcon = TD_WARNING_ICON;
    tdConfig.pszMainInstruction = L"Exit Setup";
    tdConfig.pszWindowTitle = PhApplicationName;
    tdConfig.pszContent = L"Are you sure you want to cancel the Setup?";

    TaskDialogIndirect(&tdConfig, &nButtonPressed, NULL, NULL);

    return nButtonPressed == IDNO;
}

VOID DialogPromptProcessHackerIsRunning(
    _In_ HWND hwndDlg
    )
{
    INT buttonPressed = 0;
    TASKDIALOGCONFIG config = { sizeof(TASKDIALOGCONFIG) };
    config.hwndParent = hwndDlg;
    config.hInstance = PhLibImageBase;
    config.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW;
    config.nDefaultButton = IDOK;
    config.dwCommonButtons = TDCBF_OK_BUTTON;
    config.pszMainIcon = TD_INFORMATION_ICON;
    config.pszMainInstruction = L"Please close Process Hacker before continuing.";
    config.pszWindowTitle = PhApplicationName;
    //config.pszContent = L"Please close Process Hacker before continuing.";

    TaskDialogIndirect(&config, &buttonPressed, NULL, NULL);
}

_Check_return_
BOOLEAN IsProcessHackerRunning(
    VOID
    )
{
    HANDLE mutantHandle;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING mutantName;

    RtlInitUnicodeString(&mutantName, L"\\BaseNamedObjects\\ProcessHacker2Mutant");
    InitializeObjectAttributes(
        &oa,
        &mutantName,
        0,
        NULL,
        NULL
        );

    if (NT_SUCCESS(NtOpenMutant(
        &mutantHandle,
        MUTANT_QUERY_STATE,
        &oa
        )))
    {
        NtClose(mutantHandle);
        return TRUE;
    }

    return FALSE;
}

_Check_return_
BOOLEAN IsProcessHackerInstalledUsingSetup(
    VOID
    )
{
    static PH_STRINGREF keyName = PH_STRINGREF_INIT(L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Process_Hacker2_is1");
    HANDLE keyHandle;

    // Check uninstall entries for the 'Process_Hacker2_is1' registry key.
    if (NT_SUCCESS(PhOpenKey(
        &keyHandle,
        KEY_READ,
        PH_KEY_LOCAL_MACHINE,
        &keyName,
        0
        )))
    {
        NtClose(keyHandle);
        return TRUE;
    }

    return FALSE;
}

_Check_return_
BOOLEAN IsProcessHackerInstalled(
    VOID
    )
{
    static PH_STRINGREF keyName = PH_STRINGREF_INIT(L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Process_Hacker2_is1");
    BOOLEAN installed = FALSE;
    HANDLE keyHandle;
    PPH_STRING installPath = NULL;

    if (NT_SUCCESS(PhOpenKey(
        &keyHandle,
        KEY_READ | KEY_WOW64_64KEY, // 64bit key
        PH_KEY_LOCAL_MACHINE,
        &keyName,
        0
        )))
    {
        installPath = PhQueryRegistryString(keyHandle, L"InstallLocation");
        NtClose(keyHandle);
    }

    if (!PhIsNullOrEmptyString(installPath) && PhEndsWithString2(installPath, L"ProcessHacker.exe", TRUE))
    {
        // Check if the value has a valid file path.
        installed = GetFileAttributes(installPath->Buffer) != INVALID_FILE_ATTRIBUTES;
    }

    return installed;
}

_Maybenull_
PPH_STRING GetProcessHackerInstallPath(
    VOID
    )
{
    static PH_STRINGREF keyName = PH_STRINGREF_INIT(L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Process_Hacker2_is1");
    HANDLE keyHandle;
    PPH_STRING installPath = NULL;

    if (NT_SUCCESS(PhOpenKey(
        &keyHandle,
        KEY_READ | KEY_WOW64_64KEY,
        PH_KEY_LOCAL_MACHINE,
        &keyName,
        0
        )))
    {
        installPath = PhQueryRegistryString(keyHandle, L"InstallLocation");
        NtClose(keyHandle);
    }

    return installPath;
}

_Check_return_
BOOLEAN ProcessHackerShutdown(
    VOID
    )
{
    HWND WindowHandle;

    WindowHandle = FindWindow(L"ProcessHacker", NULL);

    if (WindowHandle)
    {
        HANDLE processHandle;
        ULONG processID = 0;

        GetWindowThreadProcessId(WindowHandle, &processID);

        SendMessageTimeout(WindowHandle, WM_QUIT, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 5000, NULL);

        if (NT_SUCCESS(PhOpenProcess(&processHandle, SYNCHRONIZE | PROCESS_TERMINATE, ULongToHandle(processID))))
        {
            //PostMessage(WindowHandle, WM_QUIT, 0, 0);
            // Wait on the process handle, if we timeout, kill it.
            //if (WaitForSingleObject(processHandle, 10000) != WAIT_OBJECT_0)

            NtTerminateProcess(processHandle, 1);
            NtClose(processHandle);
        }
    }

    return FALSE;
}

_Check_return_
ULONG UninstallKph(
    _In_ BOOLEAN Kph2Uninstall
    )
{
    ULONG status = ERROR_SUCCESS;
    SC_HANDLE scmHandle;
    SC_HANDLE serviceHandle;

    if (!(scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT)))
        return PhGetLastWin32ErrorAsNtStatus();

    if (serviceHandle = OpenService(
        scmHandle,
        Kph2Uninstall ? L"KProcessHacker2" : L"KProcessHacker3",
        SERVICE_STOP | DELETE
        ))
    {
        SERVICE_STATUS serviceStatus;

        ControlService(serviceHandle, SERVICE_CONTROL_STOP, &serviceStatus);

        if (!DeleteService(serviceHandle))
        {
            status = GetLastError();
        }

        CloseServiceHandle(serviceHandle);
    }
    else
    {
        status = GetLastError();
    }

    CloseServiceHandle(scmHandle);

    return status;
}


static VOID RemoveAppCompatEntry(
    _In_ HANDLE ParentKey
    )
{
    static PH_STRINGREF keyName = PH_STRINGREF_INIT(L"ProcessHacker.exe");
    ULONG bufferLength;
    KEY_FULL_INFORMATION fullInfo;

    memset(&fullInfo, 0, sizeof(KEY_FULL_INFORMATION));

    if (!NT_SUCCESS(NtQueryKey(
        ParentKey,
        KeyFullInformation,
        &fullInfo,
        sizeof(KEY_FULL_INFORMATION),
        &bufferLength
        )))
    {
        return;
    }

    for (ULONG i = 0; i < fullInfo.Values; i++)
    {
        PPH_STRING value;
        PKEY_VALUE_FULL_INFORMATION buffer;

        bufferLength = sizeof(KEY_VALUE_FULL_INFORMATION);
        buffer = PhAllocate(bufferLength);
        memset(buffer, 0, bufferLength);

        if (NT_SUCCESS(NtEnumerateValueKey(
            ParentKey,
            i,
            KeyValueFullInformation,
            buffer,
            bufferLength,
            &bufferLength
            )))
        {
            PhFree(buffer);
            break;
        }

        //bufferLength = bufferLength;
        buffer = PhReAllocate(buffer, bufferLength);
        memset(buffer, 0, bufferLength);

        if (!NT_SUCCESS(NtEnumerateValueKey(
            ParentKey,
            i,
            KeyValueFullInformation,
            buffer,
            bufferLength,
            &bufferLength
            )))
        {
            PhFree(buffer);
            break;
        }

        if (value = PhCreateStringEx(buffer->Name, buffer->NameLength))
        {
            UNICODE_STRING us;

            PhStringRefToUnicodeString(&value->sr, &us);

            if (PhEndsWithStringRef(&value->sr, &keyName, TRUE))
            {
                NtDeleteValueKey(ParentKey, &us);
            }

            PhDereferenceObject(value);
        }

        PhFree(buffer);
    }
}

_Check_return_
BOOLEAN RemoveAppCompatEntries(
    VOID
    )
{
    static PH_STRINGREF appCompatLayersName = PH_STRINGREF_INIT(L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers");
    static PH_STRINGREF appCompatPersistedName = PH_STRINGREF_INIT(L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Persisted");
    HANDLE keyHandle;

    if (NT_SUCCESS(PhOpenKey(
        &keyHandle,
        KEY_ALL_ACCESS | KEY_WOW64_64KEY,
        PH_KEY_CURRENT_USER,
        &appCompatLayersName,
        0
        )))
    {
        RemoveAppCompatEntry(keyHandle);
        NtClose(keyHandle);
    }

    if (NT_SUCCESS(PhOpenKey(
        &keyHandle,
        KEY_ALL_ACCESS | KEY_WOW64_64KEY,
        PH_KEY_CURRENT_USER,
        &appCompatPersistedName,
        0
        )))
    {
        RemoveAppCompatEntry(keyHandle);
        NtClose(keyHandle);
    }

    if (NT_SUCCESS(PhOpenKey(
        &keyHandle,
        KEY_ALL_ACCESS | KEY_WOW64_64KEY,
        PH_KEY_LOCAL_MACHINE,
        &appCompatLayersName,
        0
        )))
    {
        RemoveAppCompatEntry(keyHandle);
        NtClose(keyHandle);
    }

    if (NT_SUCCESS(PhOpenKey(
        &keyHandle,
        KEY_ALL_ACCESS | KEY_WOW64_64KEY,
        PH_KEY_LOCAL_MACHINE,
        &appCompatPersistedName,
        0
        )))
    {
        RemoveAppCompatEntry(keyHandle);
        NtClose(keyHandle);
    }

    return TRUE;
}

//BOOLEAN FileMakeDirPathRecurse(
//    _In_ PWSTR DirPath
//    )
//{
//    BOOLEAN isSuccess = FALSE;
//    PPH_STRING dirPathString = NULL;
//    PWSTR dirTokenNext = NULL;
//    PWSTR dirTokenDup = NULL;
//    PWSTR dirTokenString = NULL;
//
//    __try
//    {
//        if ((dirTokenDup = PhDuplicateStringZ(DirPath)) == NULL)
//            __leave;
//
//        // Find the first directory path token...
//        if ((dirTokenString = _tcstok_s(dirTokenDup, L"\\", &dirTokenNext)) == NULL)
//            __leave;
//
//        while (dirTokenString)
//        {
//            if (dirPathString)
//            {
//                // Copy the new folder path to the previous folder path...
//                PPH_STRING tempPathString = PhFormatString(L"%s\\%s",
//                    dirPathString->Buffer,
//                    dirTokenString
//                    );
//
//                if (!FileExists(tempPathString->Buffer))
//                {
//                    if (_tmkdir(tempPathString->Buffer) != 0)
//                    {
//                        DEBUG_MSG(L"ERROR: _tmkdir (%u)\n", _doserrno);
//                        PhFree(tempPathString);
//                        __leave;
//                    }
//                    else
//                    {
//                        DEBUG_MSG(L"CreateDir: %s\n", tempPathString->Buffer);
//                    }
//                }
//
//                PhFree(dirPathString);
//                dirPathString = tempPathString;
//            }
//            else
//            {
//                // Copy the drive letter and root folder...
//                dirPathString  = PhFormatString(L"%s", dirTokenString);
//            }
//
//            // Find the next directory path token...
//            dirTokenString = _tcstok_s(NULL, L"\\", &dirTokenNext);
//        }
//
//        isSuccess = TRUE;
//    }
//    __finally
//    {
//        if (dirPathString)
//        {
//            PhFree(dirPathString);
//        }
//
//        if (dirTokenDup)
//        {
//            PhFree(dirTokenDup);
//        }
//    }
//
//    return isSuccess;
//}

//BOOLEAN FileRemoveDirPathRecurse(
//    _In_ PWSTR DirPath
//    )
//{
//    struct _tfinddata_t findData;
//    intptr_t findHandle = (intptr_t)INVALID_HANDLE_VALUE;
//    PPH_STRING dirPath = NULL;
//
//    dirPath = PhFormatString(L"%s\\*.*", DirPath);
//
//    // Find the first file...
//    if ((findHandle = _tfindfirst(dirPath->Buffer, &findData)) == (intptr_t)INVALID_HANDLE_VALUE)
//    {
//        if (errno == ENOENT) // ERROR_FILE_NOT_FOUND
//        {
//            PhFree(dirPath);
//            return TRUE;
//        }
//
//        DEBUG_MSG(TEXT("_tfindfirst: (%u) %s\n"), _doserrno, dirPath->Buffer);
//        PhFree(dirPath);
//        return FALSE;
//    }
//
//    do
//    {
//        // Check for "." and ".."
//        if (!_tcsicmp(findData.name, TEXT(".")) || !_tcsicmp(findData.name, TEXT("..")))
//            continue;
//
//        if (findData.attrib & _A_SUBDIR)
//        {
//            PPH_STRING subDirPath = PhFormatString(
//                L"%s\\%s",
//                DirPath,
//                findData.name
//                );
//
//            // Found a directory...
//            if (!FileRemoveDirPathRecurse(subDirPath->Buffer))
//            {
//                PhFree(subDirPath);
//                _findclose(findHandle);
//                return FALSE;
//            }
//
//            PhFree(subDirPath);
//        }
//        else
//        {
//            PPH_STRING subDirPath = StringFormat(
//                L"%s\\%s",
//                DirPath,
//                findData.name
//                );
//
//            if (findData.attrib & _A_RDONLY)
//            {
//                if (_tchmod(subDirPath->Buffer, _S_IWRITE) == 0)
//                {
//                    DEBUG_MSG(L"_tchmod: %s\n", subDirPath->Buffer);
//                }
//                else
//                {
//                    DEBUG_MSG(L"ERROR _tchmod: (%u) %s\n", _doserrno, subDirPath->Buffer);
//                }
//            }
//
//            if (_tremove(subDirPath->Buffer) == 0)
//            {
//                DEBUG_MSG(L"DeleteFile: %s\n", subDirPath->Buffer);
//            }
//            else
//            {
//                DEBUG_MSG(L"ERROR DeleteFile: (%u) %s\n", _doserrno, subDirPath->Buffer);
//            }
//
//            PhFree(subDirPath);
//        }
//
//        // Find the next entry
//    } while (_tfindnext(findHandle, &findData) == 0);
//
//    // Close the file handle
//    _findclose(findHandle);
//
//    // Check for write permission (read-only attribute ignored on directories?)
//    //if (_taccess(DirPath, 2) == -1)
//    //{
//    //    if (_tchmod(DirPath, _S_IWRITE) == 0)
//    //    {
//    //        DEBUG_MSG(L"_tchmod: %s\n", DirPath);
//    //    }
//    //    else
//    //    {
//    //        DEBUG_MSG(L"ERROR _tchmod: (%u) %s\n", _doserrno, DirPath);
//    //    }
//    //}
//
//    // Lastly delete the parent directory
//    if (_trmdir(DirPath) == 0)
//    {
//        DEBUG_MSG(L"DeleteDirectory: %s\n", DirPath);
//    }
//    else
//    {
//        DEBUG_MSG(L"ERROR DeleteDirectory: (%u) %s\n", _doserrno, DirPath);
//    }
//
//    PhFree(dirPath);
//    return TRUE;
//}


BOOLEAN CreateDirectoryPath(
    _In_ PPH_STRING DirectoryPath
    )
{
    BOOLEAN directoryExists = FALSE;
    FILE_NETWORK_OPEN_INFORMATION directoryInfo;

    if (NT_SUCCESS(PhQueryFullAttributesFileWin32(DirectoryPath->Buffer, &directoryInfo)))
    {
        if (directoryInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            directoryExists = TRUE;
        }
    }

    if (!directoryExists)
    {
        if (SHCreateDirectoryEx(
            NULL, 
            DirectoryPath->Buffer, 
            NULL
            ) == ERROR_SUCCESS)
        {
            DEBUG_MSG(L"Created Directory: %s\r\n", DirectoryPath->Buffer);
            directoryExists = TRUE;
        }
        else
        {
            DEBUG_MSG(L"SHCreateDirectoryEx Failed\r\n");
        }
    }

    return directoryExists;
}