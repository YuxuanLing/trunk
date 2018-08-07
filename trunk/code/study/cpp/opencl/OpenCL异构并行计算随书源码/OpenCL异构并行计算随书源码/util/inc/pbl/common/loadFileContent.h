#ifndef H_LOAD_FILE_CONTENT
#define H_LOAD_FILE_CONTENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file
 */
/**
 * @brief load all contents in file and return them
 *
 * @param filename the name of file you want to load 
 * @param s file content return, please free it in your code
 * @param length length of content if != NULL
 *
 * @return
 * 	-1 error
 * 	0 success
 *
 * @warning please free the memory allocated in this function
 *
 */
extern "C" PBLStatus_t loadFileContent(const char *filename, char** s, size_t* length){
	if((NULL != *s) || (NULL == filename) || (0 == strlen(filename)) ) return PBL_BAD_PARAM;

	FILE *fp = fopen(filename, "rb");
	if(NULL == fp) return PBL_FAIL_TO_OPEN_FILE;

	rewind(fp);
	if(0 != fseek(fp, 0, SEEK_END)) return PBL_FAIL_TO_SEEK_END;

	int len = ftell(fp);
	if(NULL != length) *length = len;

	char* str = (char*) malloc(len+1);
	if(NULL == str) return PBL_FAIL_TO_ALLOC;

	rewind(fp);

	if(1 != fread(str, len, 1, fp)){
		free(str);
		return PBL_FAIL_TO_READ_DATA;
	}
	str[len] = '\0';

	*s = str;

	fclose(fp);

	return PBL_SUCCESS;
}

#endif
