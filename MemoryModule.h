/*
 * Memory DLL loading code
 * Version 0.0.4
 *
 * Copyright (c) 2004-2015 by Joachim Bauch / mail@joachim-bauch.de
 * http://www.joachim-bauch.de
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is MemoryModule.h
 *
 * The Initial Developer of the Original Code is Joachim Bauch.
 *
 * Portions created by Joachim Bauch are Copyright (C) 2004-2015
 * Joachim Bauch. All Rights Reserved.
 *
 */

#ifndef __MEMORY_MODULE_HEADER
#define __MEMORY_MODULE_HEADER

#include <windows.h>

typedef void *HMEMORYMODULE;

typedef void *HMEMORYRSRC;

typedef void *HCUSTOMMODULE;

#ifdef __cplusplus
extern "C" {
#endif

typedef LPVOID (*CustomAllocFunc)(LPVOID, SIZE_T, DWORD, DWORD, void*);
typedef BOOL (*CustomFreeFunc)(LPVOID, SIZE_T, DWORD, void*);
typedef HCUSTOMMODULE (*CustomLoadLibraryFunc)(LPCSTR, void *);
typedef FARPROC (*CustomGetProcAddressFunc)(HCUSTOMMODULE, LPCSTR, void *);
typedef void (*CustomFreeLibraryFunc)(HCUSTOMMODULE, void *);

HMEMORYMODULE MemoryLoadLibrary(const void *, size_t);

HMEMORYMODULE MemoryLoadLibraryEx(const void *, size_t,
    CustomAllocFunc,
    CustomFreeFunc,
    CustomLoadLibraryFunc,
    CustomGetProcAddressFunc,
    CustomFreeLibraryFunc,
    void *);

FARPROC MemoryGetProcAddress(HMEMORYMODULE, LPCSTR);

void MemoryFreeLibrary(HMEMORYMODULE);

int MemoryCallEntryPoint(HMEMORYMODULE);

HMEMORYRSRC MemoryFindResource(HMEMORYMODULE, LPCTSTR, LPCTSTR);

HMEMORYRSRC MemoryFindResourceEx(HMEMORYMODULE, LPCTSTR, LPCTSTR, WORD);

DWORD MemorySizeofResource(HMEMORYMODULE, HMEMORYRSRC);

LPVOID MemoryLoadResource(HMEMORYMODULE, HMEMORYRSRC);

int MemoryLoadString(HMEMORYMODULE, UINT, LPTSTR, int);

int MemoryLoadStringEx(HMEMORYMODULE, UINT, LPTSTR, int, WORD);

LPVOID MemoryDefaultAlloc(LPVOID, SIZE_T, DWORD, DWORD, void *);

BOOL MemoryDefaultFree(LPVOID, SIZE_T, DWORD, void *);

HCUSTOMMODULE MemoryDefaultLoadLibrary(LPCSTR, void *);

FARPROC MemoryDefaultGetProcAddress(HCUSTOMMODULE, LPCSTR, void *);

void MemoryDefaultFreeLibrary(HCUSTOMMODULE, void *);

#ifdef __cplusplus
}
#endif

#endif
