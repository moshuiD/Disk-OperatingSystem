#pragma once
#include "Log.hpp"
#include <Windows.h>
#include <string>
using std::string;

class VDisk
{
	string m_DiskPath;
	HANDLE m_DiskFileHandle;
	HANDLE m_MappingHandle;
	PUCHAR m_Disk;

	bool SetFileSizeIfNeeded(DWORD fileSize) {
		DWORD fileSizeHigh;
		DWORD existingSize = GetFileSize(m_DiskFileHandle, &fileSizeHigh);

		if (existingSize == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
			Log::Error("Failed to get file size! Error Code:", GetLastError());
			return false;
		}

		if (existingSize == 0) {
			LARGE_INTEGER li;
			li.QuadPart = fileSize;

			if (!SetFilePointerEx(m_DiskFileHandle, li, NULL, FILE_BEGIN)) {
				Log::Error("Failed to move file pointer! Error Code:", GetLastError());
				return false;
			}

			if (!SetEndOfFile(m_DiskFileHandle)) {
				Log::Error("Failed to set end of file! Error Code:", GetLastError());
				return false;
			}

			Log::Info("File size set to", fileSize, "bytes.");
		}
		else {
			Log::Info("File already exists. Skipping file size configuration.");
		}

		return true;
	}

public:
	static constexpr WORD DISK_SECTOR_SIZE = 512;
	static constexpr DWORD MAX_DISK_SIZE = 0x80000;//4 * 512 * 256


	VDisk(string&& diskPath, DWORD fileSize) :
		m_DiskPath(std::move(diskPath)), m_DiskFileHandle(INVALID_HANDLE_VALUE), m_MappingHandle(INVALID_HANDLE_VALUE), m_Disk(nullptr)
	{
		Log::Info("Begin to connect virtual disk...");
		m_DiskFileHandle = CreateFileA(m_DiskPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_DiskFileHandle == INVALID_HANDLE_VALUE) {
			Log::Error("Failed to open virtual disk! Error Code:", GetLastError());
			return;
		}

		if (!SetFileSizeIfNeeded(fileSize)) {
			Log::Error("Failed to configure file size.");
			CloseHandle(m_DiskFileHandle);
			return;
		}

		m_MappingHandle = CreateFileMappingA(m_DiskFileHandle, NULL, PAGE_READWRITE, 0, 0, NULL);
		if (m_MappingHandle == NULL) {
			Log::Error("Failed to map disk! Error Code:", GetLastError());
			CloseHandle(m_DiskFileHandle);
			return;
		}

		m_Disk = (PUCHAR)MapViewOfFile(m_MappingHandle, FILE_MAP_WRITE, 0, 0, 0);
		if (!m_Disk) {
			Log::Error("Failed to map view of disk! Error Code:", GetLastError());
			CloseHandle(m_MappingHandle);
			CloseHandle(m_DiskFileHandle);
			return;
		}

		Log::Success("Connect virtual disk success!");
	}

	~VDisk() {
		if (m_Disk) {
			FlushViewOfFile(m_Disk, 0);
			UnmapViewOfFile(m_Disk);
		}
		if (m_MappingHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(m_MappingHandle);
		}
		if (m_DiskFileHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(m_DiskFileHandle);
		}
	}

	inline PUCHAR GetDiskPointer() {
		return m_Disk;
	}
};
