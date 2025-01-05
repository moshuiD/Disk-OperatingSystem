
#include "System.hpp"
int main() {
	Sys::StartSystem();
	//Disk dk{};
	//if (dk.LoadDisk() == ErrorCode::UNINITIALIZED)
	//	dk.FormatDisk(1, 256);
	//char a[522];
	//memset(a, 0x31, 522);
	//WORD chunkI = 0;
	//dk.SaveData((PUCHAR)a, 128, 0, 0, false, chunkI);
	//dk.SaveData((PUCHAR)a, 522-128, chunkI, 128, true, chunkI);
	//unsigned char b[522];
	//dk.LoadData(b, sizeof(a), chunkI - 1, 0);
	//for (size_t i = 0; i < 522; i++)
	//{
	//	printf("%d : %c   ", i+1, b[i]);
	//}
}