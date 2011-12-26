#include <stdio.h>
#include <unistd.h>
#include "osmosdr.h"
#include "sam3u.h"

int osmoSDRDetect(int fd)
{
	uint32_t chipID;

	if(sam3uDetect(fd, &chipID) < 0)
		return -1;

	if((chipID & 0xffffffe0) == 0x28000960) {
		printf("Chip ID: 0x%08x -- ok\n", chipID);
	} else {
		printf("Chip ID: 0x%08x -- ERROR\n", chipID);
		return -1;
	}

	return 0;
}

int osmoSDRPrintUID(int fd, int bank)
{
	uint8_t uniqueID[16];
	int i;

	if(sam3uReadUniqueID(fd, bank, uniqueID) < 0) {
		fprintf(stderr, "could not read unique ID\n");
		return -1;
	}

	printf("UID%d   : ", bank);
	for(i = 0; i < 16; i++)
		printf("%02x", uniqueID[i]);
	printf("\n");

	return 0;
}

int osmoSDRBlink(int fd)
{
	int i;

	if(sam3uWrite32(fd, 0x400e0e00 + 0x44, 1 << 19) < 0)
		return -1;
	if(sam3uWrite32(fd, 0x400e0e00 + 0x60, 1 << 19) < 0)
		return -1;
	if(sam3uWrite32(fd, 0x400e0e00 + 0x54, 1 << 19) < 0)
		return -1;
	if(sam3uWrite32(fd, 0x400e0e00 + 0x10, 1 << 19) < 0)
		return -1;
	if(sam3uWrite32(fd, 0x400e0e00 + 0x00, 1 << 19) < 0)
		return -1;

	for(i = 0; i < 10; i++) {
		printf("on."); fflush(stdout);
		if(sam3uWrite32(fd, 0x400e0e00 + 0x34, 1 << 19) < 0)
			return -1;
		usleep(250 * 1000);
		printf("off."); fflush(stdout);
		if(sam3uWrite32(fd, 0x400e0e00 + 0x30, 1 << 19) < 0)
			return -1;
		usleep(250 * 1000);
	}

	printf(" -- ok\n");
	return 0;
}

int osmoSDRRamLoad(int fd, void* bin, size_t binSize)
{
	size_t ofs;
	uint32_t tmp;

	printf("Uploading %zu bytes @ 0x20001000", binSize);

	for(ofs = 0; ofs < binSize; ofs += 4) {
		if(sam3uWrite32(fd, 0x20001000 + ofs, ((uint32_t*)bin)[ofs / 4]) < 0)
			return -1;
	}

	for(ofs = 0; ofs < binSize; ofs += 4) {
		if(sam3uRead32(fd, 0x20001000 + ofs, &tmp) < 0)
			return -1;
		if(tmp != ((uint32_t*)bin)[ofs / 4]) {
			printf(" -- RAM verify failed\n");
			return -1;
		}
	}
	printf(" -- ok\n");

	printf("Starting @ 0x%08x\n", ((uint32_t*)bin)[1]);

	if(sam3uRun(fd, ((uint32_t*)bin)[1]) < 0)
		return -1;

	return 0;
}

int osmoSDRFlash(int fd, void* bin, size_t binSize)
{
	return sam3uFlash(fd, 0, bin, binSize);
}
