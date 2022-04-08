/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main(int argc, char *argv[0])
{
	char *fileName1 = argv[0];
	char *fileName2 = argv[1];
	int size = 10000;
	char buffer[size];
	int fileLength;
	OpenFileId fileId1, fileId2;


		fileId1 = Open(fileName1);
		if (fileId1 != -1)
		{
			fileLength = Read(buffer, size, fileId1);
			buffer[fileLength] = '\0';

			Create(fileName2);
			fileId2 = Open(fileName2);

			if (fileId2 != -1)
			{
				Seek(-1, fileId2);
				Write(buffer, fileLength, fileId2);
				Close(fileId2);
			}
			Close(fileId1);
		}
		else
		{
			PrintString("Can not open your file.");
		}


	Halt();
}
