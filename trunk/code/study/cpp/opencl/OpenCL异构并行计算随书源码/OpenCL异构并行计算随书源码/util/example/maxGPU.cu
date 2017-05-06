#include "../inc/yyfnutil.h"

int main(int argc, char *argv[]) {
	int device = getMaxFlopsGPUWithCC(2,0);
	printf("%d\n",device);

	return 0;
}
