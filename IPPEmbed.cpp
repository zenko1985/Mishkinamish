#include <Windows.h>
#include <delayimp.h>
#include "resource.h"
#include "MemoryModule.h"
#include "IPPEmbed.h"

// Forward declarations of custom MemoryModule callbacks
static HCUSTOMMODULE __stdcall CustomLoadLibrary(LPCSTR name, void *userdata);
static FARPROC __stdcall CustomGetProcAddress(HCUSTOMMODULE mod, LPCSTR name, void *userdata);
static void __stdcall CustomFreeLibrary(HCUSTOMMODULE mod, void *userdata);

// Forward declarations of our hook functions
static HMODULE WINAPI Hook_LoadLibraryW(LPCWSTR);
static HMODULE WINAPI Hook_LoadLibraryA(LPCSTR);
static FARPROC WINAPI Hook_GetProcAddress(HMODULE, LPCSTR);
static BOOL WINAPI Hook_FreeLibrary(HMODULE);

extern HINSTANCE GZInst;

// Replicate MEMORYMODULE struct to access codeBase
typedef BOOL (WINAPI *DllEntryProc)(HINSTANCE, DWORD, LPVOID);
struct MEMORYMODULE_INTERNAL {
    PIMAGE_NT_HEADERS headers;
    unsigned char *codeBase;
    void *modules;
    int numModules;
    BOOL initialized;
    BOOL isDLL;
    BOOL isRelocated;
    void *alloc;
    void *free;
    void *loadLibrary;
    void *getProcAddress;
    void *freeLibrary;
    void *nameExportsTable;
    void *userdata;
    DllEntryProc exeEntry;
    DWORD pageSize;
};

struct IPPDLL {
    const wchar_t *name;
    int resId;
    HMEMORYMODULE memMod;
    unsigned char *codeBase;
    bool loaded;
};

static IPPDLL g_dlls[] = {
    {L"ippcore.dll", IDR_IPPCORE_DLL, NULL, NULL, false},
    {L"ipps.dll",    IDR_IPPS_DLL,    NULL, NULL, false},
    {L"ippsd1.dll",  IDR_IPPSD1_DLL,  NULL, NULL, false},
    {L"ippsk0.dll",  IDR_IPPSK0_DLL,  NULL, NULL, false},
    {L"ippsl9.dll",  IDR_IPPSL9_DLL,  NULL, NULL, false},
    {L"ippsy8.dll",  IDR_IPPSY8_DLL,  NULL, NULL, false},
};
static const int NUM_DLLS = sizeof(g_dlls) / sizeof(g_dlls[0]);

static int FindDLLByName(const wchar_t *name) {
    for (int i = 0; i < NUM_DLLS; i++) {
        if (_wcsicmp(name, g_dlls[i].name) == 0)
            return i;
    }
    return -1;
}

static int FindDLLByCodeBase(void *codeBase) {
    for (int i = 0; i < NUM_DLLS; i++) {
        if (g_dlls[i].loaded && g_dlls[i].codeBase == codeBase)
            return i;
    }
    return -1;
}

static const void *LoadResourceData(int resId, DWORD *outSize) {
    HRSRC hRes = FindResourceW(GZInst, MAKEINTRESOURCE(resId), RT_RCDATA);
    if (!hRes) return NULL;
    HGLOBAL hMem = LoadResource(GZInst, hRes);
    if (!hMem) return NULL;
    *outSize = SizeofResource(GZInst, hRes);
    return LockResource(hMem);
}

// These capture the REAL system functions (obtained before any hooking)
static HMODULE(WINAPI *Real_LoadLibraryW)(LPCWSTR) = LoadLibraryW;
static FARPROC(WINAPI *Real_GetProcAddress)(HMODULE, LPCSTR) = GetProcAddress;
static BOOL(WINAPI *Real_FreeLibrary)(HMODULE) = FreeLibrary;

//-----------------------------------------------------------------------------
// Custom MemoryModule callbacks
//-----------------------------------------------------------------------------
static HCUSTOMMODULE __stdcall CustomLoadLibrary(LPCSTR name, void *userdata) {
    wchar_t wname[64];
    MultiByteToWideChar(CP_ACP, 0, name, -1, wname, 64);
    int idx = FindDLLByName(wname);
    if (idx >= 0) {
        if (g_dlls[idx].loaded)
            return (HCUSTOMMODULE)g_dlls[idx].codeBase;

        DWORD size;
        const void *data = LoadResourceData(g_dlls[idx].resId, &size);
        if (data) {
            HMEMORYMODULE mod = MemoryLoadLibraryEx(data, size,
                MemoryDefaultAlloc, MemoryDefaultFree,
                CustomLoadLibrary, CustomGetProcAddress,
                CustomFreeLibrary, NULL);
            if (mod) {
                g_dlls[idx].memMod = mod;
                g_dlls[idx].codeBase = ((MEMORYMODULE_INTERNAL*)mod)->codeBase;
                g_dlls[idx].loaded = true;
                return (HCUSTOMMODULE)g_dlls[idx].codeBase;
            }
        }
    }
    return MemoryDefaultLoadLibrary(name, userdata);
}

static FARPROC __stdcall CustomGetProcAddress(HCUSTOMMODULE mod, LPCSTR name, void *userdata) {
    HMODULE hK32 = GetModuleHandleW(L"kernel32.dll");
    if ((HMODULE)mod == hK32) {
        if (strcmp(name, "LoadLibraryW") == 0)
            return (FARPROC)Hook_LoadLibraryW;
        if (strcmp(name, "LoadLibraryA") == 0)
            return (FARPROC)Hook_LoadLibraryA;
        if (strcmp(name, "GetProcAddress") == 0)
            return (FARPROC)Hook_GetProcAddress;
        if (strcmp(name, "FreeLibrary") == 0)
            return (FARPROC)Hook_FreeLibrary;
    }
    return MemoryDefaultGetProcAddress(mod, name, userdata);
}

static void __stdcall CustomFreeLibrary(HCUSTOMMODULE mod, void *userdata) {
    // Don't free our pre-loaded IPP DLLs (we manage them)
    for (int i = 0; i < NUM_DLLS; i++) {
        if (g_dlls[i].loaded && g_dlls[i].codeBase == (unsigned char*)mod)
            return;
    }
    MemoryDefaultFreeLibrary(mod, userdata);
}

//-----------------------------------------------------------------------------
// Hook functions used by MemoryModule-loaded IPP DLLs
//-----------------------------------------------------------------------------
static HMODULE WINAPI Hook_LoadLibraryW(LPCWSTR name) {
    int idx = FindDLLByName(name);
    if (idx >= 0) {
        if (g_dlls[idx].loaded)
            return (HMODULE)g_dlls[idx].codeBase;

        DWORD size;
        const void *data = LoadResourceData(g_dlls[idx].resId, &size);
        if (data) {
            HMEMORYMODULE mod = MemoryLoadLibraryEx(data, size,
                MemoryDefaultAlloc, MemoryDefaultFree,
                CustomLoadLibrary, CustomGetProcAddress,
                CustomFreeLibrary, NULL);
            if (mod) {
                g_dlls[idx].memMod = mod;
                g_dlls[idx].codeBase = ((MEMORYMODULE_INTERNAL*)mod)->codeBase;
                g_dlls[idx].loaded = true;
                return (HMODULE)g_dlls[idx].codeBase;
            }
        }
    }
    return Real_LoadLibraryW(name);
}

static HMODULE WINAPI Hook_LoadLibraryA(LPCSTR name) {
    wchar_t wname[64];
    MultiByteToWideChar(CP_ACP, 0, name, -1, wname, 64);
    int idx = FindDLLByName(wname);
    if (idx >= 0) {
        if (g_dlls[idx].loaded)
            return (HMODULE)g_dlls[idx].codeBase;

        DWORD size;
        const void *data = LoadResourceData(g_dlls[idx].resId, &size);
        if (data) {
            HMEMORYMODULE mod = MemoryLoadLibraryEx(data, size,
                MemoryDefaultAlloc, MemoryDefaultFree,
                CustomLoadLibrary, CustomGetProcAddress,
                CustomFreeLibrary, NULL);
            if (mod) {
                g_dlls[idx].memMod = mod;
                g_dlls[idx].codeBase = ((MEMORYMODULE_INTERNAL*)mod)->codeBase;
                g_dlls[idx].loaded = true;
                return (HMODULE)g_dlls[idx].codeBase;
            }
        }
    }
    return LoadLibraryA(name);
}

static FARPROC WINAPI Hook_GetProcAddress(HMODULE mod, LPCSTR name) {
    int idx = FindDLLByCodeBase(mod);
    if (idx >= 0 && g_dlls[idx].memMod) {
        FARPROC result = MemoryGetProcAddress(g_dlls[idx].memMod, name);
        if (result) return result;
    }
    return Real_GetProcAddress(mod, name);
}

static BOOL WINAPI Hook_FreeLibrary(HMODULE mod) {
    int idx = FindDLLByCodeBase(mod);
    if (idx >= 0 && g_dlls[idx].memMod) {
        return TRUE;
    }
    return Real_FreeLibrary(mod);
}

//-----------------------------------------------------------------------------
// Delay-load notification hook
//-----------------------------------------------------------------------------
static FARPROC WINAPI DelayLoadHook(unsigned dliNotify, PDelayLoadInfo pdli) {
    if (dliNotify == dliNotePreLoadLibrary) {
        wchar_t wname[64];
        MultiByteToWideChar(CP_ACP, 0, pdli->szDll, -1, wname, 64);
        int idx = FindDLLByName(wname);
        if (idx >= 0) {
            if (g_dlls[idx].loaded)
                return (FARPROC)g_dlls[idx].codeBase;

            DWORD size;
            const void *data = LoadResourceData(g_dlls[idx].resId, &size);
            if (data) {
                HMEMORYMODULE mod = MemoryLoadLibraryEx(data, size,
                    MemoryDefaultAlloc, MemoryDefaultFree,
                    CustomLoadLibrary, CustomGetProcAddress,
                    CustomFreeLibrary, NULL);
                if (mod) {
                    g_dlls[idx].memMod = mod;
                    g_dlls[idx].codeBase = ((MEMORYMODULE_INTERNAL*)mod)->codeBase;
                    g_dlls[idx].loaded = true;
                    return (FARPROC)g_dlls[idx].codeBase;
                }
            }
        }
    }
    return NULL;
}

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------
bool InitIPPEmbedding() {
    *(PfnDliHook*)&__pfnDliNotifyHook2 = DelayLoadHook;

    // Pre-load core DLLs first (in dependency order)
    static const int preload_order[] = {0, 1, 2, 3, 4, 5}; // index into g_dlls
    for (int i = 0; i < NUM_DLLS; i++) {
        int idx = preload_order[i];
        if (g_dlls[idx].loaded) continue;

        DWORD size;
        const void *data = LoadResourceData(g_dlls[idx].resId, &size);
        if (!data) {
            return false;
        }

        HMEMORYMODULE mod = MemoryLoadLibraryEx(data, size,
            MemoryDefaultAlloc, MemoryDefaultFree,
            CustomLoadLibrary, CustomGetProcAddress,
            CustomFreeLibrary, NULL);
        if (mod) {
            g_dlls[idx].memMod = mod;
            g_dlls[idx].codeBase = ((MEMORYMODULE_INTERNAL*)mod)->codeBase;
            g_dlls[idx].loaded = true;
        }
    }

    return true;
}

void FreeAllIPPDLLs() {
    for (int i = 0; i < NUM_DLLS; i++) {
        if (g_dlls[i].memMod) {
            MemoryFreeLibrary(g_dlls[i].memMod);
            g_dlls[i].memMod = NULL;
            g_dlls[i].codeBase = NULL;
            g_dlls[i].loaded = false;
        }
    }
}
