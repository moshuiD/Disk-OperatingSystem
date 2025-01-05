#include "Disk.h"

Disk::Disk() :
	m_Vdk("dk1.vdk", VDisk::MAX_DISK_SIZE)
{
	m_DiskData = m_Vdk.GetDiskPointer();
}

ErrorCode Disk::FormatDisk(WORD cluster, WORD clNum)
{
	if (VDisk::DISK_SECTOR_SIZE * cluster * clNum > VDisk::MAX_DISK_SIZE) {
		return ErrorCode::SIZE_OVERFLOW;
	}
	if (cluster == 0 || clNum == 0) {
		return ErrorCode::INVALID_PARAMETER;
	}
	memset(m_DiskData, 0, VDisk::MAX_DISK_SIZE);
	m_DBR = (DBR*)m_DiskData;
	m_DBR->sectorSize = VDisk::DISK_SECTOR_SIZE;
	m_DBR->reserveSector = (WORD)((clNum * sizeof(FATEntry)) / VDisk::DISK_SECTOR_SIZE + 2);
	m_DBR->cluster = cluster;
	m_DBR->clusterNum = clNum;

	m_FAT = (FAT)m_DiskData + m_DBR->sectorSize;
	m_FAT[0].state = SYSTEM_CLUSTER;

	for (size_t i = 0; i < m_DBR->reserveSector / m_DBR->cluster; i++) {
		m_FAT[i].state = SYSTEM_CLUSTER;
	}

	return ErrorCode::SUCCESS;
}

ErrorCode Disk::LoadDisk()
{
	m_DBR = (DBR*)m_DiskData;
	if (m_DBR->cluster == 0 || m_DBR->clusterNum == 0 || m_DBR->reserveSector == 0 || m_DBR->sectorSize == 0) {
		return ErrorCode::UNINITIALIZED;
	}

	m_FAT = (FAT)m_DiskData + m_DBR->sectorSize;
	return ErrorCode::SUCCESS;
}

WORD Disk::GetFreeCluster() const
{
	for (size_t i = 0; i < m_DBR->clusterNum; i++) {
		if (m_FAT[i].state == UNUSED_CLUSTER) {
			return i;
		}
	}
	return -1;
}

inline WORD Disk::GetChunkSize() const
{
	return m_DBR->cluster * m_DBR->sectorSize;
}

inline ErrorCode Disk::GetChunkDataByFAT(FATEntry fat, PUCHAR buff)
{
	memcpy(buff, GetChunkPtrByFAT(fat), GetChunkSize());
	return ErrorCode::SUCCESS;
}

inline PUCHAR Disk::GetChunkPtrByFAT(FATEntry fat)
{
	return (m_DiskData + m_DBR->reserveSector * m_DBR->sectorSize) +
		fat.state * m_DBR->cluster * m_DBR->sectorSize;
}

ErrorCode Disk::SaveData(LPVOID src, WORD size, WORD flChunkIndex, WORD ckOffset, bool isEof, WORD* currCkIndex)
{
	WORD chunkIndex = 0;
	PUCHAR sectorBuff = 0;

	if (ckOffset > GetChunkSize() ||
		flChunkIndex != 0 && (
			m_FAT[flChunkIndex].state == SYSTEM_CLUSTER ||
			m_FAT[flChunkIndex].state == 0 ||
			flChunkIndex > m_DBR->clusterNum)) {
		return ErrorCode::INVALID_PARAMETER;
	}

	if (flChunkIndex == 0) {
		chunkIndex = GetFreeCluster();
		if (chunkIndex == -1) {
			return ErrorCode::WRITE_ERROR;
		}
	}
	else {
		chunkIndex = flChunkIndex;
	}
	for (; ; chunkIndex = GetFreeCluster())
	{
		if (flChunkIndex != 0) {
			m_FAT[flChunkIndex].state = chunkIndex;
		}
		sectorBuff = GetChunkPtrByFAT({ chunkIndex }) + ckOffset;
		if (GetChunkSize() >= size + ckOffset) {
			memset(sectorBuff, 0, size);
			memcpy(sectorBuff, src, size);
			break;
		}
		else {
			memset(sectorBuff, 0, GetChunkSize() - ckOffset);
			memcpy(sectorBuff, src, GetChunkSize() - ckOffset);
			size -= GetChunkSize() - ckOffset;
			ckOffset = 0;
			flChunkIndex = chunkIndex;
			m_FAT[chunkIndex].state = RESERVE_CLUSTER;
		}
	}


	if (isEof) {
		m_FAT[chunkIndex].state = EOF_CLUSTER;
	}
	else {
		m_FAT[chunkIndex].state = RESERVE_CLUSTER;
	}

	if (currCkIndex != 0) {
		*currCkIndex = chunkIndex;
	}

	return ErrorCode::SUCCESS;
}

ErrorCode Disk::LoadData(LPVOID buff, WORD size, WORD clIndex, WORD dataOffset)
{
	ErrorCode statue = ErrorCode::SUCCESS;
	PUCHAR chunkBuff = new UCHAR[GetChunkSize()];
	DWORD copyOffset = 0;
	if (clIndex > m_DBR->clusterNum) {
		return ErrorCode::INVALID_PARAMETER;
	}

	for (FATEntry clI = FATEntry{ clIndex }; clI.state != EOF_CLUSTER; clI = m_FAT[clI.state]) {
		if (dataOffset < GetChunkSize()) {
			if (size <= GetChunkSize() - dataOffset) {
				statue = GetChunkDataByFAT(clI, chunkBuff);
				memcpy((PUCHAR)buff + copyOffset, chunkBuff + dataOffset, size);
				break;
			}
			else {
				statue = GetChunkDataByFAT(clI, chunkBuff);
				memcpy((PUCHAR)buff + copyOffset, chunkBuff + dataOffset, GetChunkSize() - dataOffset);
				size -= GetChunkSize() - dataOffset;
				copyOffset += GetChunkSize() - dataOffset;
				dataOffset = 0;
			}
		}
	}
	return statue;
}

ErrorCode Disk::DeleteData(WORD startChunkIndex)
{
	DWORD nextI = 0;
	for (FATEntry& FatEnt = m_FAT[startChunkIndex];; FatEnt = m_FAT[nextI])
	{
		nextI = FatEnt.state;
		FatEnt.state = UNUSED_CLUSTER;
		if (nextI != UNUSED_CLUSTER || nextI >= RESERVE_CLUSTER) {
			break;
		}
	}
	return ErrorCode::SUCCESS;
}

DWORD Disk::GetUsedSector()
{
	DWORD UsedChunk = 0;
	for (size_t i = 0; i < m_DBR->clusterNum; i++) {
		if (m_FAT[i].state != UNUSED_CLUSTER) {
			UsedChunk++;
		}
	}

	return (UsedChunk * m_DBR->cluster) + m_DBR->reserveSector;
}


