/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main()
{
	int size = 100;	// maximun length of file name
	char buffer[size];	// buffer to store
	int fileLength;		// length of the file
	OpenFileId fileId;		// fileId


	fileId = Open(""); 	// read file name from command line

	if (fileId != -1) {		// if can open file
		fileLength = Read(buffer, size, fileId);		// file content of file to buffer
		buffer[fileLength] = '\0';			// set terminating null
		PrintString(buffer);		// print in console
		PrintString("\n");
		Close(fileId);			// close file
	} else {
		PrintString("Failed to open file\n");
	}
	
  	Halt();
}
