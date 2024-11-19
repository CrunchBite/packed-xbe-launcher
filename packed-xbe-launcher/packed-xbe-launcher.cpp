//-------------------------------------------------------------------------------------------------
// File: packed-xbe-launcher.cpp
//
// An example implementation of an ENDGAME payload unpacker using xUnzip with an embedded zip file
//-------------------------------------------------------------------------------------------------

// How to use:
// 1. Put your payload contents in the payload folder
// 2. If you are making an ENDGAME payload make sure your XBEs are all habibi signed with xbedump. Example command: xbedump.exe default.xbe -habibi
// 3. Select the "Release_LTCG" configuration
// 4. Select "Build" then "Rebuild solution"
// 5. The payload will be packaged up and the launcher compiled
// 6. An ENDGAME zip and two XBEs will be output to the "output" folder. The 'signed' XBE is habibi signed for use with ENDGAME and other exploits.

// START CONFIG

// Where should the payload be extracted? (Z:\ is the cache partition)
#define OUTPUT_DIR "Z:\\temp\\"
// What XBE should be launched after extraction?
#define PAYLOAD_XBE "Z:\\temp\\default.xbe"
// Where should the zip be dropped? (You probably wont need to change this)
#define PAYLOAD_FILENAME "Z:\\payload.zip"

// END CONFIG

#include <xtl.h>
#include <stdio.h>
#include <string.h>
#include "xunzip.h"
#include <sys/stat.h>

// Function prototypes as we don't have a header
void unpackPayload();
void mountAllDrives();
HRESULT LaunchXBE(char * XBEFile);
extern "C"
{
	extern VOID	 WINAPI HalReturnToFirmware(DWORD);
}

// Entry point to the program with minimal error checking performed
VOID __cdecl main() {
	// Uncomment this if you want to have a clean cache drive (Z:\) to use
	// Mount cache drive and format it
	/*if (!XMountUtilityDrive(true)) {
		// Error, reboot
		HalReturnToFirmware(0);
		return;
	}*/

	// Mount all drives
	// The way this is done is a simple 'jailbreak' out of the XDK's usual constraints on disk access.
	// The hard drive partitions will all be mounted using the usual scene drive conventions.
	// C: is where the dashboard lives (usually)
	// E: has gamesaves and DLC
	// F: and G: are extra partitions
	// Z:/Y:/Z: are the cache partitions.
	// If you want to be sure you have enough space available for your payload to unpack uncomment the code block above to have a clean Z:\ drive and don't call mountAllDrives()
	mountAllDrives();

	// Unpack the payload zip file to PAYLOAD_FILENAME
	unpackPayload();

	// Extract the payload zip
	if (!xExtractZip(PAYLOAD_FILENAME, OUTPUT_DIR, true, true)) {
		// Error, reboot
		HalReturnToFirmware(0);
		return;
	}

	// Launch the payload XBE
	HRESULT launchRet = LaunchXBE(PAYLOAD_XBE);

	// If we get this far we failed to start the payload (wrong path?), reboot!
	HalReturnToFirmware(0);
	return;
}

// Unpack the embedded payload zip to the location specified by PAYLOAD_FILENAME
void unpackPayload() {
	HANDLE sectionHandle = XGetSectionHandle("payload");
	if (sectionHandle == INVALID_HANDLE_VALUE) {
		return;
	}
	DWORD sectionSize = XGetSectionSize(sectionHandle);

	// Check if file already exists
	HANDLE existingFile = CreateFile(PAYLOAD_FILENAME, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (existingFile != INVALID_HANDLE_VALUE) {
		// Is the file on disk the same size as what we would write?
		DWORD dwSize = GetFileSize(existingFile, NULL);
		if (dwSize == sectionSize) {
			// Size matches, assume we're good to leave it alone
			// This would be a good place to do a file checksum if you wanted to really be sure
			CloseHandle(existingFile);
			return;
		}
	}
	CloseHandle(existingFile);

	// Write the file to disk
	DWORD bytesWritten;
	PVOID mem = XLoadSectionByHandle(sectionHandle);
	HANDLE file = CreateFile(PAYLOAD_FILENAME, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(file, mem, sectionSize, &bytesWritten, NULL);
	CloseHandle(file);
	XFreeSectionByHandle(sectionHandle);
}

// Helper functions
// Everything below here is used for hard drive access and XBE launching

typedef struct _STRING {
	USHORT	Length;
	USHORT	MaximumLength;
	PSTR	Buffer;
} UNICODE_STRING, *PUNICODE_STRING, ANSI_STRING, *PANSI_STRING;
extern "C" { 
	XBOXAPI LONG WINAPI IoCreateSymbolicLink(IN PUNICODE_STRING SymbolicLinkName,IN PUNICODE_STRING DeviceName);
	XBOXAPI LONG WINAPI IoDeleteSymbolicLink(IN PUNICODE_STRING SymbolicLinkName);
}
struct pathconv_s {
	char * DriveLetter;
	char * FullPath;
} pathconv_table[] = {
	{ "DVD:", "\\Device\\Cdrom0" },// Can't use D:
	{ "C:", "\\Device\\Harddisk0\\Partition2" },
	{ "E:", "\\Device\\Harddisk0\\Partition1" },
	{ "F:", "\\Device\\Harddisk0\\Partition6" },
	{ "G:", "\\Device\\Harddisk0\\Partition7" },
	{ "X:", "\\Device\\Harddisk0\\Partition3" },
	{ "Y:", "\\Device\\Harddisk0\\Partition4" },
	{ "Z:", "\\Device\\Harddisk0\\Partition5" },
	{ NULL, NULL }
};
#define DeviceC "\\Device\\Harddisk0\\Partition2"
#define DeviceE "\\Device\\Harddisk0\\Partition1"
#define CdRom "\\Device\\Cdrom0"
#define DeviceX "\\Device\\Harddisk0\\Partition3"
#define DeviceY "\\Device\\Harddisk0\\Partition4"
#define DeviceZ "\\Device\\Harddisk0\\Partition5"
#define DeviceF "\\Device\\Harddisk0\\Partition6"
#define DeviceG "\\Device\\Harddisk0\\Partition7"
#define DriveC "\\??\\C:"
#define DriveD "\\??\\D:"
#define DriveE "\\??\\E:"
#define DriveF "\\??\\F:"
#define DriveG "\\??\\G:"
#define DriveX "\\??\\X:"
#define DriveY "\\??\\Y:"
#define DriveZ "\\??\\Z:"
LONG MountDevice(LPSTR sSymbolicLinkName, char *sDeviceName) {
	UNICODE_STRING 	deviceName;
	deviceName.Buffer  = (PSTR)sDeviceName;
	deviceName.Length = (USHORT)strlen(sDeviceName);
	deviceName.MaximumLength = (USHORT)strlen(sDeviceName) + 1;
	UNICODE_STRING 	symbolicLinkName;
	symbolicLinkName.Buffer  = sSymbolicLinkName;
	symbolicLinkName.Length = (USHORT)strlen(sSymbolicLinkName);
	symbolicLinkName.MaximumLength = (USHORT)strlen(sSymbolicLinkName) + 1;
	return IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
}
LONG UnMountDevice(LPSTR sSymbolicLinkName) {
	UNICODE_STRING 	symbolicLinkName;
	symbolicLinkName.Buffer  = sSymbolicLinkName;
	symbolicLinkName.Length = (USHORT)strlen(sSymbolicLinkName);
	symbolicLinkName.MaximumLength = (USHORT)strlen(sSymbolicLinkName) + 1;
	return IoDeleteSymbolicLink(&symbolicLinkName);
}
void mountAllDrives() {
	UnMountDevice(DriveX);
	UnMountDevice(DriveY);
	UnMountDevice(DriveZ);
	UnMountDevice(DriveC);
	UnMountDevice(DriveE);
	UnMountDevice(DriveF);
	UnMountDevice(DriveG);
	MountDevice(DriveX, DeviceX);
	MountDevice(DriveY, DeviceY);
	MountDevice(DriveZ, DeviceZ);
	MountDevice(DriveC, DeviceC);
	MountDevice(DriveE, DeviceE);
	MountDevice(DriveF, DeviceF);
	MountDevice(DriveG, DeviceG);
}
BOOL fileExists(LPSTR filePath) {
    struct stat buffer;
    return (stat(filePath, &buffer) == 0);
}
HRESULT LaunchXBE(char * XBEFile) {
	HRESULT r;
	char *umFilename, *mFilename, *mDrivePath, *mDriveLetter, *mFullPath, *mDevicePath, *tempname;

	umFilename = (char*)malloc(strlen(XBEFile) + 1);
	lstrcpy(umFilename, XBEFile);

	tempname = strrchr(umFilename, '\\');
	mFilename = tempname;
	if (tempname) {
		mFilename = tempname + 1;
	}
	mDrivePath = umFilename;
	tempname = strrchr(mDrivePath, '\\');
	if (tempname) {
		tempname[0] = 0;
	}
	int tm = 0;
	while(mDrivePath[tm]!=':' && mDrivePath[tm] != 0) tm++;
	mDriveLetter = (char *)malloc(tm+3);
	lstrcpyn(mDriveLetter, mDrivePath, tm+2);
	mDriveLetter[tm + 3] = 0;
	mDrivePath += tm + 1;

	int i;
	for (i = 0; pathconv_table[i].DriveLetter != NULL; i++) {
		if(!lstrcmpi(pathconv_table[i].DriveLetter, mDriveLetter)) {
			mDevicePath = pathconv_table[i].FullPath;
			break;
		}
	}

	mFullPath = (char*)malloc(strlen(mDevicePath) + strlen(mDrivePath) + 1);
	sprintf(mFullPath, "%s%s", mDevicePath, mDrivePath);
	if(mFullPath[(strlen(mFullPath)-1)] == '\\') {
		mFullPath[(strlen(mFullPath)-1)] = 0;
	}

	ANSI_STRING DeviceName = { strlen(mFullPath), strlen(mFullPath) + 1, mFullPath};
	ANSI_STRING LinkName = {strlen("\\??\\D:"), strlen("\\??\\D:") + 1, "\\??\\D:"};

	IoDeleteSymbolicLink(&LinkName);
	IoCreateSymbolicLink(&LinkName, &DeviceName);

	mFullPath = (char*)malloc(strlen(mFilename) + 4);
	sprintf(mFullPath, "D:\\%s", mFilename);
	r = XLaunchNewImage(mFullPath, NULL);
	return r;
}
