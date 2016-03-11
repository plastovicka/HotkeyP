/*
 (C) Uwe Sieber - www.uwe-sieber.de
 */
#include "hdr.h"
#pragma hdrstop
#include <Setupapi.h>
#include <cfgmgr32.h>
#include "hotkeyp.h"

typedef DWORD(__stdcall *TCM_Request_Device_EjectW)(DEVINST       dnDevInst, PPNP_VETO_TYPE pVetoType, LPWSTR pszVetoName, ULONG         ulNameLength, ULONG         ulFlags);
typedef DWORD(__stdcall *TCM_Get_Parent)(PDEVINST      pdnDevInst, DEVINST       dnDevInst, ULONG         ulFlags);
typedef BOOL(__stdcall *TSetupDiDestroyDeviceInfoList)(HDEVINFO DeviceInfoSet);
typedef BOOL(__stdcall *TSetupDiGetDeviceInterfaceDetailA)(HDEVINFO DeviceInfoSet, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData, PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData, DWORD DeviceInterfaceDetailDataSize, PDWORD RequiredSize, PSP_DEVINFO_DATA DeviceInfoData);
typedef BOOL(__stdcall *TSetupDiEnumDeviceInterfaces)(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, const GUID* InterfaceClassGuid, DWORD MemberIndex, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData);
typedef HDEVINFO(__stdcall *TSetupDiGetClassDevsA)(const GUID* ClassGuid, PCSTR Enumerator, HWND hwndParent, DWORD Flags);

static TCM_Request_Device_EjectW fCM_Request_Device_EjectW;
static TCM_Get_Parent fCM_Get_Parent;
static TSetupDiDestroyDeviceInfoList fSetupDiDestroyDeviceInfoList;
static TSetupDiGetDeviceInterfaceDetailA fSetupDiGetDeviceInterfaceDetailA;
static TSetupDiEnumDeviceInterfaces fSetupDiEnumDeviceInterfaces;
static TSetupDiGetClassDevsA fSetupDiGetClassDevsA;

extern "C" const GUID DECLSPEC_SELECTANY GUID_DEVINTERFACE_DISK ={0x53f56307L, 0xb6bf, 0x11d0, {0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};
extern "C" const GUID DECLSPEC_SELECTANY GUID_DEVINTERFACE_CDROM ={0x53f56308L, 0xb6bf, 0x11d0, {0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};
extern "C" const GUID DECLSPEC_SELECTANY GUID_DEVINTERFACE_FLOPPY ={0x53f56311L, 0xb6bf, 0x11d0, {0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};


// returns the device instance handle of a storage volume or 0 on error
DEVINST GetDrivesDevInstByDeviceNumber(long DeviceNumber, UINT DriveType, TCHAR* szDosDeviceName)
{
	bool IsFloppy = (_tcsstr(szDosDeviceName, _T("\\Floppy")) != NULL); // who knows a better way?

	GUID* guid;

	switch(DriveType) {
		case DRIVE_REMOVABLE:
			if(IsFloppy) {
				guid = (GUID*)&GUID_DEVINTERFACE_FLOPPY;
			}
			else {
				guid = (GUID*)&GUID_DEVINTERFACE_DISK;
			}
			break;
		case DRIVE_FIXED:
			guid = (GUID*)&GUID_DEVINTERFACE_DISK;
			break;
		case DRIVE_CDROM:
			guid = (GUID*)&GUID_DEVINTERFACE_CDROM;
			break;
		default:
			return 0;
	}

	// Get device interface info set handle for all devices attached to system
	HDEVINFO hDevInfo = fSetupDiGetClassDevsA(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if(hDevInfo == INVALID_HANDLE_VALUE)	{
		return 0;
	}

	// Retrieve a context structure for a device interface of a device information set
	DWORD dwIndex = 0;
	long res;

	BYTE Buf[1024];
	PSP_DEVICE_INTERFACE_DETAIL_DATA_A pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)Buf;
	SP_DEVICE_INTERFACE_DATA         spdid;
	SP_DEVINFO_DATA                  spdd;
	DWORD                            dwSize;

	spdid.cbSize = sizeof(spdid);

	for(;;)	{
		res = fSetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex, &spdid);
		if(!res) {
			break;
		}

		dwSize = 0;
		fSetupDiGetDeviceInterfaceDetailA(hDevInfo, &spdid, NULL, 0, &dwSize, NULL); // check the buffer size

		if(dwSize!=0 && dwSize<=sizeof(Buf)) {

			pspdidd->cbSize = sizeof(*pspdidd); // 5 Bytes!

			ZeroMemory(&spdd, sizeof(spdd));
			spdd.cbSize = sizeof(spdd);

			res = fSetupDiGetDeviceInterfaceDetailA(hDevInfo, &spdid, pspdidd, dwSize, &dwSize, &spdd);
			if(res) {

				// in case you are interested in the USB serial number:
				// the device id string contains the serial number if the device has one,
				// otherwise a generated id that contains the '&' TCHAR...
				/*
				DEVINST DevInstParent = 0;
				CM_Get_Parent(&DevInstParent, spdd.DevInst, 0);
				TCHAR szDeviceIdString[MAX_PATH];
				CM_Get_Device_ID(DevInstParent, szDeviceIdString, MAX_PATH, 0);
				printf("DeviceId=%s\n", szDeviceIdString);
				*/

				// open the disk or cdrom or floppy
				HANDLE hDrive = CreateFileA(pspdidd->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
				if(hDrive != INVALID_HANDLE_VALUE) {
					// get its device number
					STORAGE_DEVICE_NUMBER sdn;
					DWORD dwBytesReturned = 0;
					res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
					if(res) {
						if(DeviceNumber == (long)sdn.DeviceNumber) {  // match the given device number with the one of the current device
							CloseHandle(hDrive);
							fSetupDiDestroyDeviceInfoList(hDevInfo);
							return spdd.DevInst;
						}
					}
					CloseHandle(hDrive);
				}
			}
		}
		dwIndex++;
	}

	fSetupDiDestroyDeviceInfoList(hDevInfo);

	return 0;
}

int removeDrive1(TCHAR DriveLetter)
{
	DriveLetter &= ~0x20; // uppercase

	if(DriveLetter < 'A' || DriveLetter > 'Z') {
		return 1;
	}

	TCHAR szRootPath[] = _T("X:\\");   // "X:\"  -> for GetDriveType
	szRootPath[0] = DriveLetter;

	TCHAR szDevicePath[] = _T("X:");   // "X:"   -> for QueryDosDevice
	szDevicePath[0] = DriveLetter;

	TCHAR szVolumeAccessPath[] = _T("\\\\.\\X:");   // "\\.\X:"  -> to open the volume
	szVolumeAccessPath[4] = DriveLetter;

	long DeviceNumber = -1;

	// open the storage volume
	HANDLE hVolume = CreateFile(szVolumeAccessPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if(hVolume == INVALID_HANDLE_VALUE) {
		return 1;
	}

	// get the volume's device number
	STORAGE_DEVICE_NUMBER sdn;
	DWORD dwBytesReturned = 0;
	long res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
	if(res) {
		DeviceNumber = sdn.DeviceNumber;
	}
	CloseHandle(hVolume);

	if(DeviceNumber == -1) {
		return 1;
	}

	// get the drive type which is required to match the device numbers correctely
	UINT DriveType = GetDriveType(szRootPath);

	// get the dos device name (like \device\floppy0) to decide if it's a floppy or not - who knows a better way?
	TCHAR szDosDeviceName[MAX_PATH];
	res = QueryDosDevice(szDevicePath, szDosDeviceName, MAX_PATH);
	if(!res) {
		return 1;
	}

	// get the device instance handle of the storage volume by means of a SetupDi enum and matching the device number
	DEVINST DevInst = GetDrivesDevInstByDeviceNumber(DeviceNumber, DriveType, szDosDeviceName);

	if(DevInst == 0) {
		return 1;
	}

	PNP_VETO_TYPE VetoType = PNP_VetoTypeUnknown;
	WCHAR VetoNameW[MAX_PATH];

	// get drives's parent, e.g. the USB bridge, the SATA port, an IDE channel with two drives!
	DEVINST DevInstParent = 0;
	res = fCM_Get_Parent(&DevInstParent, DevInst, 0);

	for(int tries=1; tries<=3; tries++) { // sometimes we need some tries...

		VetoNameW[0] = 0;

		// CM_Query_And_Remove_SubTree doesn't work for restricted users
		//res = CM_Query_And_Remove_SubTreeW(DevInstParent, &VetoType, VetoNameW, MAX_PATH, CM_REMOVE_NO_RESTART); // CM_Query_And_Remove_SubTreeA is not implemented under W2K!
		//res = CM_Query_And_Remove_SubTreeW(DevInstParent, NULL, NULL, 0, CM_REMOVE_NO_RESTART);  // with messagebox (W2K, Vista) or balloon (XP)

		res = fCM_Request_Device_EjectW(DevInstParent, &VetoType, VetoNameW, MAX_PATH, 0);
		//res = CM_Request_Device_EjectW(DevInstParent, NULL, NULL, 0, 0); // with messagebox (W2K, Vista) or balloon (XP)

		if(res==CR_SUCCESS && VetoType==PNP_VetoTypeUnknown) return 0;

		Sleep(500); // required to give the next tries a chance!
	}

	msglng(767, "Cannot remove drive %c:", DriveLetter /*, res, VetoNameW*/);
	return 1;
}

void removeDrive(TCHAR DriveLetter)
{
	HMODULE lib = LoadLibraryA("setupapi.dll");
	if(lib){
		fCM_Request_Device_EjectW = (TCM_Request_Device_EjectW)GetProcAddress(lib, "CM_Request_Device_EjectW");
		fCM_Get_Parent = (TCM_Get_Parent)GetProcAddress(lib, "CM_Get_Parent");
		fSetupDiDestroyDeviceInfoList = (TSetupDiDestroyDeviceInfoList)GetProcAddress(lib, "SetupDiDestroyDeviceInfoList");
		fSetupDiGetDeviceInterfaceDetailA = (TSetupDiGetDeviceInterfaceDetailA)GetProcAddress(lib, "SetupDiGetDeviceInterfaceDetailA");
		fSetupDiEnumDeviceInterfaces = (TSetupDiEnumDeviceInterfaces)GetProcAddress(lib, "SetupDiEnumDeviceInterfaces");
		fSetupDiGetClassDevsA = (TSetupDiGetClassDevsA)GetProcAddress(lib, "SetupDiGetClassDevsA");
		if(fCM_Request_Device_EjectW) removeDrive1(DriveLetter);
	}
	FreeLibrary(lib);
	if(!fCM_Request_Device_EjectW) msg(_T("Remove drive function requires Windows 2000 or later"));
}

void removeUSBdrives()
{
	//find all USB drives
	TCHAR szVolumeAccessPath[] = _T("\\\\.\\X:");
	for(TCHAR drive = 'A'; drive<='Z'; drive++)
	{
		szVolumeAccessPath[4] = drive;
		HANDLE hVolume = CreateFile(szVolumeAccessPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		if(hVolume != INVALID_HANDLE_VALUE)
		{
			STORAGE_PROPERTY_QUERY query;
			query.PropertyId = StorageAdapterProperty;
			query.QueryType = PropertyStandardQuery;
			UCHAR outBuf[512];
			ULONG returnedLength;
			((STORAGE_ADAPTER_DESCRIPTOR*)outBuf)->BusType = 0;

			DeviceIoControl(hVolume, IOCTL_STORAGE_QUERY_PROPERTY,
				&query, sizeof(STORAGE_PROPERTY_QUERY),
				&outBuf, 512, &returnedLength, NULL);
			CloseHandle(hVolume);

			if(((STORAGE_ADAPTER_DESCRIPTOR*)outBuf)->BusType == 7 /*BusTypeUSB*/)
			{
				removeDrive(drive);
			}
		}
	}
}
