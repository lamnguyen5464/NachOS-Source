/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main(int argc, char *argv[0])
{

	char *fileName = argv[0];
	int size = 10000;
	char name[size];

	if (fileName != NULL) {
		CreateFile(fileName);
	} else {
		PrintString("Enter fileName (enter '0' to stop): ");

		do {
			ReadString(name, size);

			PrintString(name);
			CreateFile(name);

		 	if (name != NULL && strlen(name) == 1 && name[0] == '0'){
				 break;
			 }
		} while (true);
	}


  	Halt();
}
