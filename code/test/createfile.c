
#include "syscall.h"

int main(int argc, char *argv[])
{

	// char *fileName = argv[0];
	int size = 100;
	char name[size];


	  ReadString(name, size);
	  PrintNum(argc);

	  PrintString(name);
	 

	  PrintString(argv[0]);

	 // Create("dat.txt");

  	Halt();
	return 0;
}
