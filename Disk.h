#pragma once
#include <stdlib.h>
#include "VDisk.hpp"
#include "ErrorCode.h"

class Disk
{
	struct FATEntry
	{
		WORD state;
	};
	typedef FATEntry* FAT;

	struct DBR
	{
		WORD sectorSize;	//�����ֽ���
		WORD reserveSector; //������������
		WORD cluster;		//ÿ��������
		WORD clusterNum;	//��������
	};

	

	static constexpr WORD UNUSED_CLUSTER = 0;
	static constexpr WORD RESERVE_CLUSTER = 0xFFF0;
	static constexpr WORD SYSTEM_CLUSTER = 0xFFFE;
	static constexpr WORD EOF_CLUSTER = 0xFFFF;
	unsigned char* m_DiskData;

	
	FAT m_FAT;
	
	VDisk m_Vdk;


	inline WORD GetFreeCluster() const;
	inline WORD GetChunkSize() const;
	inline ErrorCode GetChunkDataByFAT(FATEntry fat, PUCHAR buff);
	inline PUCHAR GetChunkPtrByFAT(FATEntry fat);
public:
	DBR* m_DBR;
	Disk();
	ErrorCode FormatDisk(WORD cluster,WORD ckNum);
	ErrorCode LoadDisk();
	ErrorCode SaveData(LPVOID src, WORD size, WORD flChunkIndex, WORD clOffset, bool isEof,WORD* currCkIndex);
	ErrorCode LoadData(LPVOID buff, WORD size, WORD clIndex, WORD dataOffset);
	ErrorCode DeleteData(WORD startChunkIndex);
	DWORD GetUsedSector();
};

