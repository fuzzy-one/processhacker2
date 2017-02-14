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
    mz_uint64 total_size = 0;
    PPH_STRING totalSizeStr = NULL;
    mz_bool status = MZ_FALSE;
    mz_zip_archive zip_archive = { 0 };
    PSETUP_EXTRACT_FILE extractData;
    ULONG extractDataLength;
    ULONG resourceLength;
    HRSRC resourceHandle = NULL;
    HGLOBAL resourceData;
    PVOID resourceBuffer;

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

    // Get filesize information for extract progress
    for (mz_uint i = 0; i < extractDataLength; i++)
    {
        mz_uint index = mz_zip_reader_locate_file(
            &zip_archive,
            extractData[i].FileName,
            NULL,
            0
            );

        mz_zip_archive_file_stat file_stat;

        if (!mz_zip_reader_file_stat(&zip_archive, index, &file_stat))
        {
            //mz_zip_reader_end(&zip_archive);
            //break;
        }

        total_size += file_stat.m_uncomp_size;

        //DEBUG_MSG(L"Filename: \"%hs\", Uncompressed size: %I64u, Compressed size: %I64u\r\n",
        //    file_stat.m_filename,
        //    file_stat.m_uncomp_size,
        //    file_stat.m_comp_size
        //    );
    }

    totalSizeStr = PhFormatSize(total_size, -1);
    DEBUG_MSG(L"Opened Zip: %hs (%s)\r\n", "processhacker-2.38-bin.zip", totalSizeStr->Buffer);

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
        {
            goto CleanupExit;
        }

        if (!mz_zip_reader_file_stat(&zip_archive, file_index, &file_stat))
        {
            goto CleanupExit;
        }

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
        {
            goto CleanupExit;
        }

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
        {
            goto CleanupExit;
        }

        totalLength += (mz_uint64)bufferLength;
        //SetProgress((LONG)totalLength, (LONG)total_size);
        SendMessage(GetDlgItem(Arguments, IDC_INSTALL_PROGRESS), PBM_SETPOS, totalLength, total_size);

        NtClose(fileHandle);
        mz_free(buffer);
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