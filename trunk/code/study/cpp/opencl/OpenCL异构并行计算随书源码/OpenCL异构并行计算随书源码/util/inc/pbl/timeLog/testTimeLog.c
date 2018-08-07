#include "log.h"

int main(){
	initLogging("fuck.txt");
	startLogging();
	printf("hello\n");
	endLogging();

	printTimeLog("main");

	cleanLogging();
	return 0;
}
