// copyright. zmkjh 2025/11/20 - --

#ifndef ZTREAM_ADMIN_C
#define ZTREAM_ADMIN_C

#include "admin.h"

static inline BOOL ztream_is_admin() {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return FALSE;

    TOKEN_ELEVATION elev = { 0 };
    DWORD len = sizeof(elev);
    BOOL fRet = GetTokenInformation(hToken, TokenElevation, &elev, len, &len);
    CloseHandle(hToken);
    return fRet && elev.TokenIsElevated;
}

static inline void ztream_be_admin() {
    char path[MAX_PATH] = { 0 };
    if (!GetModuleFileNameA(NULL, path, MAX_PATH))
        return;

    SHELLEXECUTEINFOA sei = { sizeof(sei) };
    sei.lpVerb     = "runas";
    sei.lpFile     = path;
    sei.nShow      = SW_SHOWNORMAL;
    sei.fMask      = SEE_MASK_NOCLOSEPROCESS;

    ShellExecuteExA(&sei);
}

static inline void ztream_check_admin() {
    if (!ztream_is_admin()) {
        ztream_be_admin();
        exit(0);
    }
}

#endif
