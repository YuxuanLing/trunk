#include <stdio.h>
#include <stdlib.h>
#include "../inc/yyfnutil.h"

int main (int argc, char *argv[]) {
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

 return 0;
}


