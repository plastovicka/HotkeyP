/*
 (C) Petr Lastovicka
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/  
#define _WIN32_WINNT 0x600
#define _WIN32_IE 0x0500
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <wininet.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <ctype.h>
#include <time.h>
#include <locale.h>
#include <math.h>
#include <malloc.h>
#include <winioctl.h>
#include <psapi.h>
#include <string>

typedef std::basic_string<TCHAR> tstring;

#ifndef WM_MOUSEHWHEEL

#define WM_MOUSEHWHEEL                  0x020E
#define MOUSEEVENTF_HWHEEL      0x01000 /* hwheel button rolled */
#define MAPVK_VK_TO_VSC     (0)
#define MAPVK_VSC_TO_VK     (1)
#define ERROR_ELEVATION_REQUIRED         740L

enum STORAGE_PROPERTY_ID {
  StorageDeviceProperty,
  StorageAdapterProperty,
  StorageDeviceIdProperty,
  StorageDeviceUniqueIdProperty,
  StorageDeviceWriteCacheProperty,
  StorageMiniportProperty,
  StorageAccessAlignmentProperty
};

enum STORAGE_QUERY_TYPE {
  PropertyStandardQuery,
  PropertyExistsQuery,
  PropertyMaskQuery,
  PropertyQueryMaxDefined
};

struct STORAGE_PROPERTY_QUERY {
  STORAGE_PROPERTY_ID PropertyId;
  STORAGE_QUERY_TYPE QueryType;
  BYTE  AdditionalParameters[1];
};

struct STORAGE_ADAPTER_DESCRIPTOR {
  DWORD Version;
  DWORD Size;
  DWORD MaximumTransferLength;
  DWORD MaximumPhysicalPages;
  DWORD AlignmentMask;
  BOOLEAN AdapterUsesPio;
  BOOLEAN AdapterScansDown;
  BOOLEAN CommandQueueing;
  BOOLEAN AcceleratedTransfer;
  BYTE  BusType;
  WORD   BusMajorVersion;
  WORD   BusMinorVersion;
};

#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifndef SECURITY_MANDATORY_LABEL_AUTHORITY

#define SECURITY_MANDATORY_LABEL_AUTHORITY          {0,0,0,0,0,16}
#define SECURITY_MANDATORY_MEDIUM_RID               (0x00002000L)
#define SE_GROUP_INTEGRITY                 (0x00000020L)
#define TokenIntegrityLevel ((TOKEN_INFORMATION_CLASS)25)

struct TOKEN_MANDATORY_LABEL {
	SID_AND_ATTRIBUTES Label;
};

#define PROCESS_QUERY_LIMITED_INFORMATION  (0x1000)  

#endif

#endif
