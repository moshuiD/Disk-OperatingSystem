#include "File.h"

inline WORD File::GetDirsChunk() const
{
	return m_Disk.m_DBR->reserveSector / m_Disk.m_DBR->cluster;
}

inline WORD File::GetDirEntry(const char* filePath) const
{
	for (size_t i = 0; i < 16; i++) {
		if (strcmp(m_Dirs[i].fileName, filePath) == 0) {
			return i;
		}
	}
	return -1;
}

File::File(Disk& dk) :
	m_Disk(dk)
{
	Log::Info("Begin init File module...");
	if (m_Disk.LoadData(m_Dirs, 512, GetDirsChunk(), 0) == ErrorCode::SUCCESS) {
		if (!(strcmp(m_Dirs[0].fileName, "/") == 0 && (m_Dirs[0].fileAtti & FILE_DIR) == FILE_DIR)) {
			Log::Warning("No any DirEntry");
			FileCreate("/", FILE_DIR | FILE_PRIVI_USER, User::ROOT, 0);
			Log::Success("Create root success");
		}
	}
	Log::Success("File module init success!");
}

ErrorCode File::FileCreate(std::string&& filePath, WORD FileAtti, DWORD author, WORD* pStartChunk)
{
	for (size_t i = 0; i < 16; i++) {
		if (strcmp(filePath.c_str(), m_Dirs[i].fileName) == 0) {
			return ErrorCode::FILE_EXIST;
		}
		if (m_Dirs[i].fileAtti == 0) {
			memcpy(m_Dirs[i].fileName, filePath.c_str(), 16);
			WORD startChunk = 0;
			m_Disk.SaveData(0, 0, 0, 0, 0, &startChunk);
			m_Dirs[i].fileAtti = FileAtti;
			m_Dirs[i].fileSize = 0;
			m_Dirs[i].startChunkIndex = startChunk;
			m_Dirs[i].fileInfo.author = author;
			m_Dirs[i].fileInfo.createTime = time(0);
			m_Disk.SaveData(&m_Dirs[i], sizeof(DirEntry), GetDirsChunk(), i * sizeof(DirEntry), false, 0);
			if (pStartChunk != 0) {
				*pStartChunk = startChunk;
			}
			return ErrorCode::SUCCESS;
		}
	}
	return ErrorCode::WRITE_ERROR;
}

ErrorCode File::FileDelete(std::string&& filePath)
{
	WORD fileI = GetDirEntry(filePath.c_str());
	if (fileI == -1) {
		return ErrorCode::NO_FILE_OR_DIR;
	}
	m_Disk.DeleteData(m_Dirs[fileI].startChunkIndex);
	memset(&m_Dirs[fileI], 0, sizeof(DirEntry));
	return m_Disk.SaveData(&m_Dirs[fileI], sizeof(DirEntry), GetDirsChunk(), fileI * sizeof(DirEntry), false, 0);
}

ErrorCode File::GetFileInfo(std::string&& filePath, DWORD user, DirEntry** pDirs)
{
	WORD fileI = GetDirEntry(filePath.c_str());

	if (fileI == 0xffff) {
		return ErrorCode::NO_FILE_OR_DIR;
	}

	if (m_Dirs[fileI].fileInfo.author != user && user != User::ROOT) {
		return ErrorCode::NO_ACCESS;
	}
	*pDirs = &m_Dirs[fileI];
	return ErrorCode();
}

ErrorCode File::WriteFile(DirEntry* pDirEntry, LPVOID buff, DWORD size, WORD fileAct,DWORD offset, bool isEof)
{
	if ((pDirEntry->fileAtti & FILE_DIR) != 0) {
		return ErrorCode::INVALID_PARAMETER;
	}

	if ((pDirEntry->fileAtti & FILE_WRITE) == 0 ||
		((pDirEntry->fileAtti & FILE_PRIVI_SYSTEM) && User::currentUser != User::ROOT) ||
		(pDirEntry->fileInfo.author != User::currentUser && User::currentUser != User::ROOT)) {
		return ErrorCode::NO_ACCESS;
	}
	
	if (fileAct == FILE_APPEND) {
		pDirEntry->fileSize += size;
	}
	if (fileAct == FILE_REDUCE) {
		pDirEntry->fileSize -= size;
	}
	ErrorCode state = m_Disk.SaveData(buff, size, pDirEntry->startChunkIndex, offset, isEof, 0);
	if (state != ErrorCode::SUCCESS) {
		return state;
	}
	

	for (size_t i = 0; i < 16; i++) {
		if (strcmp(pDirEntry->fileName, m_Dirs[i].fileName) == 0) {
			state = m_Disk.SaveData(pDirEntry, sizeof(DirEntry), GetDirsChunk(), i * sizeof(DirEntry), 0, 0);
			break;
		}
	}

}

ErrorCode File::ReadFile(DirEntry* pDirEntry, LPVOID buff, DWORD size, DWORD offset)
{
	if ((pDirEntry->fileAtti & FILE_DIR) != 0 ||
		size > pDirEntry->fileSize ||
		offset > pDirEntry->fileSize) {
		return ErrorCode::INVALID_PARAMETER;
	}

	if ((pDirEntry->fileAtti & FILE_READ) == 0 ||
		((pDirEntry->fileAtti & FILE_PRIVI_SYSTEM) && User::currentUser != User::ROOT) ||
		(pDirEntry->fileInfo.author != User::currentUser && User::currentUser != User::ROOT)) {
		return ErrorCode::NO_ACCESS;
	}
	ErrorCode state;
	for (size_t i = 0; i < 16; i++) {
		if (strcmp(pDirEntry->fileName, m_Dirs[i].fileName) == 0) {
			state = m_Disk.LoadData(buff, size, pDirEntry->startChunkIndex, offset);
			break;
		}
	}

}

std::vector<File::DirEntry> File::GetFils(std::string& filePath) const
{
	std::vector<File::DirEntry> retV{};
	for (size_t i = 0; i < 16; i++)
	{
		if (memcmp(m_Dirs[i].fileName, filePath.c_str(), filePath.size()) == 0) {
			retV.push_back(m_Dirs[i]);
		}
	}
	return retV;
}
