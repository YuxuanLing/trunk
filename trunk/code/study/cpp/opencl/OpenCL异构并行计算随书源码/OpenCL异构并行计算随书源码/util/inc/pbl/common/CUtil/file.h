#ifndef H_CUTIL_FILE
#define H_CUTIL_FILE
/**
 * @file
 * @author yyfn
 * @date 20120713
 * @brief wrap for file api of C
 *
 **/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @param file file name
 * @param mode the same as fopen
 *
 * @return the same as fopen
 */
FILE* Fopen(const char *file, const char *mode) {
	FILE *fp = fopen(file, mode);
	if (NULL == fp) {
		printf("%s, %d, open file %s failed\n", __FILE__, __LINE__, file);

	}
	return fp;
}

#define Fclose(file) {\
	if(0 != fclose(file)){\
		printf("%s, %d, close file failed\n", __FILE__, __LINE__);\
	}\
}

#define Fflush(file) {\
	if(0 != fflush(file)){\
		printf("%s, %d, fflush failed\n", __FILE__, __LINE__);\
	}\
}

//fgets
//fgetpos
//fprintf
#define Fread(ptr, size, count, file) {\
	size_t numReaded = fread(ptr, size, count, file);\
	if(count != numReaded){\
		printf("%s, %d, fread error, just read %d\n", __FILE__, __LINE__, numReaded);\
	}\
}

#define Fwrite(ptr, size, count, file) {\
	size_t numWrited = fwrite(ptr, size, count, file);\
	if(count != numWrited){\
		printf("%s, %d, fwrite error, just write %d\n", __FILE__, __LINE__, numWrited);\
	}\
}
//
//
//fscanf
//
#define Fseek(file, offset, origin) {\
	if(0 != fseek(file, offset, origin)){\
		printf("%s, %d\n", __FILE__, __LINE__);\
	}\
}

inline long Ftell(FILE *file) {
	long ret = ftell(file);
	if (-1 == ret) {
		perror("ftell failed:");
	}

	return ret;
}

#define Remove(file) {\
	if(0 != remove(file)){\
		perror("remove "#file" failed:");\
	}\
}
#define Rename(file1, file2) {\
	if(0 != rename(file1, file2)){\
		perror("rename "#file1" to "#file2" error:");\
	}\
}
//rewind

FILE* Tmpfile() {
	FILE *fp = tmpfile();
	if (NULL == fp) {
		printf("%s, %d, create tmp file failed\n", __FILE__, __LINE__);
	}
	return fp;
}
#endif
/*
 * #include <stdio.h>
#include <stdlib.h>
#include "yyfnutil.h"

int main(int argc, char* argv[]) {
	FILE *fp = Fopen("test.data", "a+");
	char ptr[] = {1, 34, 65, 87, 98,54};
	Fwrite(ptr, sizeof(ptr), 1, fp);
	Fflush(fp);

	printf("%d\n", Ftell(fp));
	Fseek(fp, -2, SEEK_CUR);
	printf("%d\n", Ftell(fp));
	rewind(fp);
	char ptra[6];
	Fread(ptra, sizeof(ptr), 1, fp);

	printf("%d\n", ptra[4]);

	Fclose(fp);

	Rename("test.data", "test.data1");

	Remove("test.data1");

}
*/

