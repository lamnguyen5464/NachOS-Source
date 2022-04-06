/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main(int argc, char *argv[0])
{

	char fileName1[100];
	char fileName2[100];
	int size = 10000;
	char buffer[size];
	int fileLength;
	OpenFileId fileId1, fileId2;

	fileName1 = argv[0];
	fileName2 = argv[1];
		

	if (fileName != NULL) {
		fileId = Open(fileName);

		if (fileId != -1) {
			fileLength = Read(buffer, size, fileId1);
			buffer[fileLength] = '\0';
			PrintString(buffer);
			Close(fileId);
		}

	}
	PrintString("Enter fileName (enter '0' to stop): ");

  	Halt();
}
