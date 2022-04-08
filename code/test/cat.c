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
	char buffer[size];
	int fileLength;
	OpenFileId fileId;

	/*
	fileName = argv[0];
	

			fileId = Open(fileName);

			if (fileId != -1) {
				fileLength = Read(buffer, size, fileId);
				buffer[fileLength] = '\0';
				PrintString(buffer);
				Close(fileId);
			}


			*/
	
  	Halt();
}
