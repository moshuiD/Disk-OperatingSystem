#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "Disk.h"
#include "File.h"
#include "UserManager.hpp"
#include <conio.h>
#include <algorithm>
DWORD User::currentUser = 0;
namespace Sys {
	Disk m_Disk;
	File* m_File;
	UserManager* m_User;
	static bool Login()
	{
		char uName[8] = { 0 };
		char pwd[8] = { 0 };
		for (size_t i = 0; i < 3; i++)
		{
			printf("请输入用户名: ");
			scanf("%s", uName);
			printf("请输入密码: ");
			for (size_t i = 0; i < 8; i++) {
				char ch = _getch();
				if (ch == '\x0D') {
					break;
				}
				pwd[i] = ch;
			}
			if (m_User->LogIn(uName, pwd) == ErrorCode::SUCCESS) {
				return true;
			}
			system("cls");
		}
		return false;
	}

	static string GetUserNameById(DWORD id) {
		DWORD lastUid = User::currentUser;
		User::currentUser = User::ROOT;
		for (auto const& user : m_User->GetUsers()) {
			if (user.uid == id) {
				User::currentUser = lastUid;
				return { user.name };
			}
		}
		User::currentUser = lastUid;

	}

	static bool SystemLoop()
	{
		string userName;
		string localPath = "/";

		userName = GetUserNameById(User::currentUser);

		getchar();
		char cmdBuff[256] = { 0 };
		while (true)
		{
			memset(cmdBuff, 0, 256);

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
			printf("%s:%s$ ", userName.c_str(), localPath.c_str());
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

			gets_s(cmdBuff);
#pragma region ls
			if (memcmp(cmdBuff, "ls", 2) == 0) {
				auto files = m_File->GetFils(localPath);
				for (auto f : files) {
					if (f.fileName == localPath) {
						memcpy(f.fileName, ".", 2);
					}


					if (memcmp(cmdBuff + 3, "-l", 2) == 0) {
						string fileAtti;
						if ((f.fileAtti & File::FILE_READ) == File::FILE_READ) {
							fileAtti += "r";
						}
						else {
							fileAtti += "-";
						}
						if ((f.fileAtti & File::FILE_WRITE) == File::FILE_WRITE) {
							fileAtti += "w";
						}
						else {
							fileAtti += "-";
						}
						if ((f.fileAtti & File::FILE_EXEC) == File::FILE_EXEC) {
							fileAtti += "x";
						}
						else {
							fileAtti += "-";
						}
						printf("%s %s %d %d %s\r\n", fileAtti.c_str(), GetUserNameById(f.fileInfo.author).c_str(), f.fileSize, f.fileInfo.createTime, f.fileName);
					}
					else {
						printf("%s ", f.fileName);
					}
				}
				printf("\r\n");
				continue;
			}
#pragma endregion

#pragma region users
			if (memcmp(cmdBuff, "users", 5) == 0) {
				if (User::currentUser != User::ROOT) {
					printf("该用户没有此权限！\r\n");
					continue;
				}
				if (memcmp(cmdBuff + 6, "-l", 2) == 0) {
					for (auto& const us : m_User->GetUsers()) {
						printf("%x %s\r\n", us.uid, us.name);
					}
				}
				if (memcmp(cmdBuff + 6, "-a", 2) == 0) {
					char userName[8]{ 0 };
					char passWord[8]{ 0 };
					printf("用户名: ");
					gets_s(userName);
					printf("密码: ");
					for (size_t i = 0; i < 8; i++) {
						char ch = _getch();
						if (ch == '\x0D') {
							break;
						}
						passWord[i] = ch;
					}
					m_User->AddUser(userName, passWord);
					printf("\r\n");
				}
				if (memcmp(cmdBuff + 6, "-d", 2) == 0) {
					char userName[8]{ 0 };
					printf("用户名: ");
					gets_s(userName);
					if (strcmp(userName, "root") == 0) {
						printf("禁止删除root！\r\n");
						continue;
					}
					ErrorCode state = m_User->DeleteUser(userName);
					if (state == ErrorCode::NO_ACCESS) {
						printf("该用户没有此权限！\r\n");
					}
					if (state == ErrorCode::INVALID_PARAMETER) {
						printf("错误的参数！\r\n");
					}
				}
				continue;
			}
#pragma endregion

#pragma region cat
			if (memcmp(cmdBuff, "cat", 3) == 0) {
				char fileName[256] = { 0 };
				if (strlen(cmdBuff + 4) > 0) {
					strcpy(fileName, cmdBuff + 4);
					File::DirEntry* pDir = 0;
					ErrorCode state = m_File->GetFileInfo(localPath + fileName, User::currentUser, &pDir);
					if (state == ErrorCode::NO_FILE_OR_DIR) {
						printf("没有此文件或为文件夹！\r\n");
						continue;
					}
					if (state == ErrorCode::NO_ACCESS) {
						printf("该用户没有此权限！\r\n");
						continue;
					}

					char* buff = new char[pDir->fileSize];
					m_File->ReadFile(pDir, buff, pDir->fileSize, 0);
					for (size_t i = 0; i < pDir->fileSize; i++)
					{
						printf("%c", buff[i]);
					}
					delete[] buff;
					printf("\r\n");
				}
				continue;
			}
#pragma endregion

#pragma region touch
			if (memcmp(cmdBuff, "touch", 5) == 0) {
				if (strlen(cmdBuff + 6) > 0) {
					char fileName[256];
					strcpy(fileName, cmdBuff + 6);
					m_File->FileCreate(localPath + fileName, File::FILE_FILE | File::FILE_READ | File::FILE_WRITE | File::FILE_PRIVI_USER, User::currentUser, 0);
				}
				continue;
			}
#pragma endregion

#pragma region cd
			if (memcmp(cmdBuff, "cd", 2) == 0) {
				if (strlen(cmdBuff + 3) > 0) {
					string path;
					if (strcmp(cmdBuff + 3, "/") == 0) {
						path = "/";
					}
					else {
						path = (string(cmdBuff + 3) + '/');
					}

					for (auto const& f : m_File->GetFils(localPath)) {
						if ((f.fileAtti & File::FILE_DIR) == File::FILE_DIR) {
							if (memcmp(f.fileName, path.c_str(), path.size()) == 0) {
								localPath = path;
								goto IL1;
							}
						}
					}
				}
				printf("没有此路径！\r\n");
			IL1:;
				continue;
			}
#pragma endregion

#pragma region mkdir
			if (memcmp(cmdBuff, "mkdir", 5) == 0) {
				if (strlen(cmdBuff + 6) > 0) {
					string dir(localPath + (cmdBuff + 6) + "/");
					m_File->FileCreate(dir.c_str(), File::FILE_DIR, User::currentUser, 0);
				}
				continue;
			}
#pragma endregion

#pragma region edit
			if (memcmp(cmdBuff, "edit", 4) == 0) {
				if (strlen(cmdBuff + 5) > 0) {
					File::DirEntry* pDir = 0;
					ErrorCode state = m_File->GetFileInfo(localPath + (cmdBuff + 5), User::currentUser, &pDir);
					if (state == ErrorCode::NO_ACCESS) {
						printf("该用户没有此权限！\r\n");
						continue;
					}
					if (state == ErrorCode::NO_FILE_OR_DIR) {
						printf("没有此文件！\r\n");
						continue;
					}
					printf("请输入模式(a是追加 e是修改): ");
					char mod;
					scanf("%c", &mod);
					char buff[512]{ 0 };
					if (mod == 'a') {

						printf("请输入追加的内容:");
						getchar();
						gets_s(buff);
						m_File->WriteFile(pDir, buff, strlen(buff), File::FILE_APPEND, pDir->fileSize, false);
					}
					if (mod == 'e') {
						printf("请输入开始修改的位置:");
						int offset = 0;
						scanf("%d", &offset);
						getchar();
						gets_s(buff);
						if(pDir->fileSize< offset + strlen(buff))
							pDir->fileSize = offset + strlen(buff);

						m_File->WriteFile(pDir, buff, strlen(buff), File::FILE_MANUAL, offset, false);
					}
				}
				continue;
			}
#pragma endregion

#pragma region rm
			if (memcmp(cmdBuff, "rm", 2) == 0) {
				if (strlen(cmdBuff + 3) > 0) {
					string path = (localPath + (cmdBuff + 3));
					File::DirEntry* pDir = 0;
					ErrorCode state = m_File->GetFileInfo(path.c_str(), User::currentUser, &pDir);
					if (state == ErrorCode::NO_ACCESS) {
						printf("该用户没有此权限！\r\n");
						continue;
					}
					else if (state == ErrorCode::SUCCESS) {
						m_File->FileDelete(path.c_str());
					}

				}
				continue;
			}
#pragma endregion

#pragma region dk
			if (memcmp(cmdBuff, "dk", 2) == 0) {
				DWORD usedS = m_Disk.GetUsedSector();
				printf("使用的扇区: %d 未使用的扇区 %d 磁盘占用比 %f\r\n", usedS, m_Disk.m_DBR->cluster * m_Disk.m_DBR->clusterNum - usedS, (float)usedS / (float)(m_Disk.m_DBR->cluster * m_Disk.m_DBR->clusterNum));
				continue;
			}
#pragma endregion

#pragma region exit
			if (memcmp(cmdBuff, "exit", 4) == 0) {
				return false;
			}
#pragma endregion

#pragma region shutdown
			if (memcmp(cmdBuff, "shutdown", 8) == 0) {
				return true;
			}
#pragma endregion

		}
	}
	void StartSystem()
	{
		User::currentUser = User::ROOT;
		if (m_Disk.LoadDisk() == ErrorCode::UNINITIALIZED) {
			Log::Error("Disk uninititalize!");
			Log::Info("Must format disk");
			WORD cl = 0, clNum = 0;
			printf("一个簇占的扇区数量： \r\n");
			printf("簇总数： \r\n");
			scanf("%d %d", &cl, &clNum);
			m_Disk.FormatDisk(cl, clNum);
			Log::Success("Init disk success");
		}

		m_File = new File{ m_Disk };
		m_User = new UserManager{ m_Disk ,*m_File };
		Log::Success("Init system success! press any key to login and use");
		_getch();
		do
		{
			system("cls");
			if (Login()) {
				system("cls");
				if (SystemLoop()) {
					break;
				}
			}
			else {
				Log::Error("Due to incorrect password input three times, the system will shut down!");
				break;
			}
		} while (true);
		Log::Success("system shutdown");
	}
};