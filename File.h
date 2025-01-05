#pragma once
#include "Disk.h"
#include "User.h"
#include <vector>
class File
{
public:
	struct FileInfo
	{
		DWORD createTime;
		DWORD author;
	};
	struct DirEntry
	{
		CHAR fileName[16];
		WORD fileAtti;
		WORD startChunkIndex;
		DWORD fileSize;
		FileInfo fileInfo;
	};
	
	static constexpr WORD FILE_DIR = 0x20;
	static constexpr WORD FILE_FILE = 0x40;
	static constexpr WORD FILE_PRIVI_SYSTEM = 0x1;
	static constexpr WORD FILE_PRIVI_USER = 0x2;
	static constexpr WORD FILE_READ = 0x4;
	static constexpr WORD FILE_WRITE = 0x8;
	static constexpr WORD FILE_EXEC = 0x10;

	static constexpr WORD FILE_APPEND = 0;
	static constexpr WORD FILE_REDUCE = 1;
	static constexpr WORD FILE_MANUAL = 2;
private:
	
	DirEntry m_Dirs[16];//local
	Disk& m_Disk;
	inline WORD GetDirsChunk() const;
	inline WORD GetDirEntry(const char* filePath) const;
public:
	File(Disk& dk);
	ErrorCode FileCreate(std::string&& filePath, WORD FileAtti, DWORD author,WORD* startChunk);
	ErrorCode FileDelete(std::string&& filePath);
	ErrorCode GetFileInfo(std::string&& filePath,DWORD user, DirEntry** pDirs);
	//if remove bytes must set size < 0
	ErrorCode WriteFile(DirEntry* pDirEntry, LPVOID buff, DWORD size,WORD fileAct, DWORD offset, bool isEof);
	ErrorCode ReadFile(DirEntry* pDirEntry, LPVOID buff, DWORD size, DWORD offset);
	std::vector<DirEntry>GetFils(std::string& filePath) const;
};

