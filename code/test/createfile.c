
#include "syscall.h"

int main(int argc, char *argv[])
{
	int status;
	Create("");		// read from command line 

	if (status == -1) {
		PrintString("Failed to create file\n");		
	} else {
		PrintString("Create file successfully\n");		
	}

	while (1 != 0) {

		int size = 100;	// maximun length
		char name[size];	// name of file that is typed by user
		PrintString("Type your file name (type '0' for stop'):");		// condition for break
		ReadString(name, size);		// read from console

		status = Create(name);			// create file

		if (status == -1) {
			PrintString("Failed to create file\n");		
		} else {
			PrintString("Create file successfully\n");		
		}

		if (name[0] == '0') {		// if user type '0';
			break;
		}
	}

  	Halt();
	return 0;
}