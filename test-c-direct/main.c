#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <unistd.h>

#define DIR_STRUCT_SIZE_NO			0
#define DIR_STRUCT_SIZE_X_Y			1
#define DIR_STRUCT_SIZE_XX_YY		2
//#define DIR_STRUCT_SIZE_XXX_YYY		3
//#define DIR_STRUCT_SIZE_XXXX_YYYY	4

//#define DIR_STRUCT_SIZE 			DIR_STRUCT_SIZE_NO
//#define DIR_STRUCT_SIZE 			DIR_STRUCT_SIZE_X_Y
#define DIR_STRUCT_SIZE 			DIR_STRUCT_SIZE_XX_YY

#define FILES_PATH_PREFIX			"/tmp/hashes/"
#define FILES_PATH_PREFIX_SIZE		12
#if (DIR_STRUCT_SIZE != DIR_STRUCT_SIZE_NO)
#	define FILES_PATH_SIZE 			(FILES_PATH_PREFIX_SIZE + (DIR_STRUCT_SIZE+1)*2)
#else
#	define FILES_PATH_SIZE 			FILES_PATH_PREFIX_SIZE
#endif

#define FILE_NAME_SIZE				64

//#define FILES_COUNT		(16 * 1024)
//#define FILE_SIZE		(16 * 1024)



//#define  FILES_COUNT		(32 * 1024)
//#define  FILE_SIZE		(16 * 1024)

//#define FILES_COUNT		(64 * 1024)
//#define FILE_SIZE		(16 * 1024)

//#define  FILES_COUNT		(64 * 1024)
//#define  FILE_SIZE		(8 * 1024)

//#define FILES_COUNT		(128 * 1024)
//#define FILE_SIZE		(8 * 1024)

//#define  FILES_COUNT		(128 * 1024)
//#define  FILE_SIZE		(4 * 1024)

//#define FILES_COUNT		(256 * 1024)
//#define FILE_SIZE		(4 * 1024)

//#define  FILES_COUNT		(256 * 1024)
//#define  FILE_SIZE		(2 * 1024)

//#define FILES_COUNT		(512 * 1024)
//#define FILE_SIZE		(2 * 1024)

//#define  FILES_COUNT		(512 * 1024)
//#define  FILE_SIZE		(1 * 1024)

//#define FILES_COUNT		(1024 * 1024)
//#define FILE_SIZE		(1 * 1024)

//#define  FILES_COUNT		(1024 * 1024)
//#define  FILE_SIZE		(512)

#define FILES_COUNT		(2048 * 1024)
#define FILE_SIZE		(512)


//#define FILES_COUNT		(2048 * 1024)
//#define FILE_SIZE		(1 * 1024)

//#define FILES_COUNT		(2048 * 1024)
//#define FILE_SIZE		(2 * 1024)

//#define FILES_COUNT		(2048 * 1024)
//#define FILE_SIZE		(4 * 1024)

#pragma pack(1)
struct STestFile
{
	char	pathName[FILES_PATH_SIZE + FILE_NAME_SIZE + 1];
	uint8_t data[FILE_SIZE];
};

#pragma pack(1)
struct STestFile (*filesData)[FILES_COUNT];

clock_t stat_start_first, stat_end_first;
clock_t stat_start_last, stat_end_last;

void GenerateRandomFiles() 
{
	char	*pathName;
	uint8_t *data;
#if(DIR_STRUCT_SIZE != DIR_STRUCT_SIZE_NO)
	struct stat st = {0};
#endif
	filesData = calloc(FILES_COUNT, sizeof(struct STestFile));

	if (!filesData)
		exit(-1);
	for (uint32_t idxFile = 0; idxFile < FILES_COUNT; idxFile++) 
	{
		pathName = (*filesData)[idxFile].pathName;
		data = (*filesData)[idxFile].data;

		srand((uint32_t)time(0) + idxFile);

		strncpy(pathName, FILES_PATH_PREFIX, FILES_PATH_PREFIX_SIZE);

		for (int idxByte = 0; idxByte < FILE_NAME_SIZE; idxByte += 4)
		{
			sprintf(&pathName[FILES_PATH_SIZE + idxByte], "%04X", (uint16_t)rand());
		}
#if(DIR_STRUCT_SIZE != DIR_STRUCT_SIZE_NO)
		memcpy(&pathName[FILES_PATH_PREFIX_SIZE], 
				&pathName[FILES_PATH_SIZE], 
				DIR_STRUCT_SIZE);
		memcpy(&pathName[FILES_PATH_PREFIX_SIZE + DIR_STRUCT_SIZE + 1], 
				&pathName[FILES_PATH_SIZE + DIR_STRUCT_SIZE], 
				DIR_STRUCT_SIZE);
		pathName[FILES_PATH_PREFIX_SIZE + DIR_STRUCT_SIZE] = 0;
		if (stat(pathName, &st) == -1)
			mkdir(pathName, 0777);
		pathName[FILES_PATH_PREFIX_SIZE + DIR_STRUCT_SIZE] = '/';
		pathName[FILES_PATH_SIZE-1] = 0;
		if (stat(pathName, &st) == -1)
			mkdir(pathName, 0777);
		pathName[FILES_PATH_SIZE-1] = '/';
#endif
		for (int idxByte = 0; idxByte < FILE_SIZE; idxByte += 2)
			*(uint16_t*)&(data[idxByte]) = (uint16_t)rand();
	}
}

void CreateWriteFiles()
{
	FILE *f;
	for (int idxFile = 0; idxFile < FILES_COUNT; idxFile++)
	{
		if (idxFile == 1)
			stat_start_first = clock();
		if (idxFile == FILES_COUNT - 1)
			stat_start_last = clock();
		f = fopen((*filesData)[idxFile].pathName, "w");
		fwrite((*filesData)[idxFile].data, FILE_SIZE, 1, f);
		fflush(f);
		fclose(f);
		if (idxFile == 1)
			stat_end_first = clock();
		if (idxFile == FILES_COUNT - 1)
			stat_end_last = clock();
	}
}

void AccessFiles()
{
	FILE *f;
	for (int idxFile = 0; idxFile < FILES_COUNT; idxFile++)
	{
		if (idxFile == 1)
			stat_start_first = clock();
		if (idxFile == FILES_COUNT - 1)
			stat_start_last = clock();
		f = fopen((*filesData)[idxFile].pathName, "r");
		if (!f)
			exit(-1);
		fflush(f);
		fclose(f);
		if (idxFile == 1)
			stat_end_first = clock();
		if (idxFile == FILES_COUNT - 1)
			stat_end_last = clock();
	}
}

void ReadFiles()
{
	FILE *f;
	uint8_t buf[FILE_SIZE];
	int ret;
	for (int idxFile = 0; idxFile < FILES_COUNT; idxFile++)
	{
		if (idxFile == 1)
			stat_start_first = clock();
		if (idxFile == FILES_COUNT - 1)
			stat_start_last = clock();
		f = fopen((*filesData)[idxFile].pathName, "r");
		if (!f)
			exit(-1);
		ret = fread(buf, 1, FILE_SIZE, f);
		if (!ret)
			exit(-1);
		fflush(f);
		fclose(f);
		if (idxFile == 1)
			stat_end_first = clock();
		if (idxFile == FILES_COUNT - 1)
			stat_end_last = clock();
	}
}

void DeleteFiles()
{
#if(DIR_STRUCT_SIZE != DIR_STRUCT_SIZE_NO)
	struct stat st = {0};
#endif
	char *pathName;
	for (int idxFile = 0; idxFile < FILES_COUNT; idxFile++)
	{
		pathName = (*filesData)[idxFile].pathName;
		if (idxFile == 1)
			stat_start_first = clock();
		if (idxFile == (FILES_COUNT - 1))
			stat_start_last = clock();
		remove(pathName);
		if (idxFile == 1)
			stat_end_first = clock();
		if (idxFile == (FILES_COUNT - 1))
			stat_end_last = clock();
	}
#if DIR_STRUCT_SIZE != DIR_STRUCT_SIZE_NO
	for (int idxFile = 0; idxFile < FILES_COUNT; idxFile++)
	{
		pathName = (*filesData)[idxFile].pathName;
		pathName[FILES_PATH_SIZE] = 0;
		if (stat(pathName, &st) != -1)
			rmdir(pathName);
		pathName[FILES_PATH_SIZE - DIR_STRUCT_SIZE - 1] = 0;
		if (stat(pathName, &st) != -1)
			rmdir(pathName);
	}
#endif
}

struct statvfs___ {
   unsigned long  f_bsize;    /* file system block size */
   unsigned long  f_frsize;   /* fragment size */
   fsblkcnt_t     f_blocks;   /* size of fs in f_frsize units */
   fsblkcnt_t     f_bfree;    /* # free blocks */
   fsblkcnt_t     f_bavail;   /* # free blocks for unprivileged users */
   fsfilcnt_t     f_files;    /* # inodes */
   fsfilcnt_t     f_ffree;    /* # free inodes */
   fsfilcnt_t     f_favail;   /* # free inodes for unprivileged users */
   unsigned long  f_fsid;     /* file system ID */
   unsigned long  f_flag;     /* mount flags */
   unsigned long  f_namemax;  /* maximum filename length */
};

double TimeDelta(struct timeb *start, struct timeb *end)
{
	return 1000.0 * (end->time - start->time) + 1.0 * (end->millitm - start->millitm);
}

void PrintTimeStats(const char* fname, double delta)
{
	printf("%s %dms (%dus/file, %ld...%ldus) \n", 
		fname, (int)delta, (int)(delta / FILES_COUNT * 1000), 
		stat_end_first - stat_start_first, stat_end_last - stat_start_last);
}

int main()
{
	struct timeb start, end;
	struct statvfs stat0, stat1;
	int ret;

	printf("Generating files data, %d files of %d bytes... ", FILES_COUNT, FILE_SIZE);
	GenerateRandomFiles();
	printf("Done\n");

	ret = statvfs(FILES_PATH_PREFIX, &stat0);
	if (ret)
		exit(-1);

	ftime(&start);
	CreateWriteFiles();
	ftime(&end);
	PrintTimeStats("CreateWriteFiles", TimeDelta(&start, &end));

	ret = statvfs(FILES_PATH_PREFIX, &stat1);
	printf("Space free: %5.1f Mb\n", (float)stat0.f_bsize * stat0.f_bfree / (1024 * 1024));
	printf("Space used: %5.1f Mb\n", ((float)stat0.f_bsize * stat0.f_bfree - (float)stat1.f_bsize * stat1.f_bfree) / (1024 * 1024));

	ftime(&start);
	AccessFiles();
	ftime(&end);
	PrintTimeStats("AccessFiles", TimeDelta(&start, &end));


	ftime(&start);
	ReadFiles();
	ftime(&end);
	PrintTimeStats("ReadFiles", TimeDelta(&start, &end));

	ftime(&start);
	DeleteFiles();
	ftime(&end);
	PrintTimeStats("DeleteFiles", TimeDelta(&start, &end));

	return 0;
}
