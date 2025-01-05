#pragma once
#include "File.h"
#include "Disk.h"
#include "User.h"
#include <vector>
class UserManager
{
	Disk& m_Disk;
	File& m_File;
	struct UsInfo
	{
		DWORD uid;
		CHAR name[8];
		CHAR passwd[8];
	};

public:

	ErrorCode AddUser(const char* userName, const char* passWord) {
		File::DirEntry* file = nullptr;
		ErrorCode state = m_File.GetFileInfo("/pwd", User::ROOT, &file);
		if (state == ErrorCode::NO_FILE_OR_DIR) {
			return ErrorCode::UNINITIALIZED;
		}
		if (strlen(userName) > 8 || strlen(passWord) > 8) {
			return ErrorCode::INVALID_PARAMETER;
		}

		UsInfo info{};
		if (strcmp(userName, "root") == 0) {
			info.uid = User::ROOT;
		}
		else {
			info.uid = time(0);
		}
		
		memcpy(info.name, userName, strlen(userName));
		memcpy(info.passwd, passWord, strlen(passWord));

		return m_File.WriteFile(file, &info, sizeof(UsInfo), File::FILE_APPEND, file->fileSize, true);
	}

	ErrorCode LogIn(const char* userName, const char* passWord) {
		File::DirEntry* file = nullptr;
		ErrorCode state = m_File.GetFileInfo("/pwd", User::ROOT, &file);
		if (state == ErrorCode::NO_FILE_OR_DIR || file->fileSize == 0) {
			return ErrorCode::UNINITIALIZED;
		}
		UsInfo info{};
		for (size_t i = 0; i < file->fileSize / sizeof(UsInfo); i++) {
			m_File.ReadFile(file, &info, sizeof(UsInfo), i * sizeof(UsInfo));
			if (!strcmp(info.name, userName) && !strcmp(info.passwd, passWord)) {
				User::currentUser = info.uid;
				return ErrorCode::SUCCESS;
			}
		}
		return ErrorCode::NO_ACCESS;
	}

	ErrorCode DeleteUser(const char* userName) {
		File::DirEntry* pFile = 0;
		std::vector<UsInfo> users;
		m_File.GetFileInfo("/pwd", User::ROOT, &pFile);
		for (size_t i = 0; i < pFile->fileSize; i += sizeof(UsInfo)) {
			UsInfo userInfo{};
			m_File.ReadFile(pFile, &userInfo, sizeof(UsInfo), i);
			users.push_back(userInfo);
		}
		for (auto it = users.begin(); it != users.end();) {
			if (strcmp(it->name, userName) == 0) {
				it = users.erase(it);
			}
			else {
				++it;
			}
		}
		pFile->fileSize -= sizeof(UsInfo);
		return m_File.WriteFile(pFile, users.data(), pFile->fileSize, File::FILE_MANUAL, 0, true);
	}

	std::vector<UsInfo> GetUsers() const {
		File::DirEntry* pFile = 0;
		std::vector<UsInfo> users;
		m_File.GetFileInfo("/pwd", User::ROOT, &pFile);
		for (size_t i = 0; i < pFile->fileSize; i += sizeof(UsInfo)) {
			UsInfo userInfo{};
			m_File.ReadFile(pFile, &userInfo, sizeof(UsInfo), i);
			users.push_back(userInfo);
		}
		return users;
	}

	UserManager(Disk& dk, File& fl) :
		m_Disk(dk),
		m_File(fl)
	{
		Log::Info("Begin init UserManager module...");
		File::DirEntry* file = 0;
		ErrorCode state = m_File.GetFileInfo("/pwd", User::ROOT, &file);
		if (state == ErrorCode::NO_FILE_OR_DIR) {
			Log::Warning("No any users!");
			m_File.FileCreate("/pwd", File::FILE_PRIVI_SYSTEM | File::FILE_FILE | File::FILE_READ | File::FILE_WRITE, User::ROOT, 0);
			AddUser("root", "root");
			Log::Info("Create success!\r\nUserName -> root , PassWord -> root");
		}
		Log::Success("UserManager modul init success!");
	}

};