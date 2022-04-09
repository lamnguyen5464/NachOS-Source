/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main()
{
	int size = 100;
	char buffer[size];
	int fileLength;
	OpenFileId fileId1, fileId2;


	fileId1 = Open("");
	if (fileId1 != -1)
		{
			fileLength = Read(buffer, size, fileId1);
			
			fileId2 = Open("");

			if (fileId2 != -1)
			{
				Seek(0, fileId2);
				Write(buffer, fileLength, fileId2);
				Close(fileId2);
			} else {
				PrintString("Cannot open file 2\n");
			}
			Close(fileId1);
		}
	else
		{
			PrintString("Can not open your file.\n");
		}


	Halt();
}
