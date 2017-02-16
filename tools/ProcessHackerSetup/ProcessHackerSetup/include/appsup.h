#ifndef _APPSUP_H
#define _APPSUP_H

#define STATUS_MSG(Format, ...) \
{ \
    PPH_STRING msgString = PhFormatString(Format, __VA_ARGS__); \
    if (msgString) \
    { \
        DEBUG_MSG(L"%s\n", msgString->Buffer); \
        PhDereferenceObject(msgString); \
    } \
}

VOID InitializeFont(
    _In_ HWND ControlHandle,
    _In_ LONG Height,
    _In_ LONG Weight
    );


BOOLEAN CreateLink(
    _In_ PWSTR DestFilePath,
    _In_ PWSTR FilePath,
    _In_ PWSTR FileParentDir,
    _In_ PWSTR FileComment
    );


HBITMAP LoadPngImageFromResources(
    _In_ PCWSTR Name
    );

_Maybenull_
PPH_STRING GetProcessHackerInstallPath(
    VOID
    );

BOOLEAN CreateDirectoryPath(
    _In_ PPH_STRING DirectoryPath
    );

#endif _APPSUP_H