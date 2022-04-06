/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main()
{
  const int size = 100;

  char buffer[1000];

  OpenFileId fileId1, fileId2;
  int i;
  int fileLength;

  fileId1 = Open("hello.txt");
  fileId2 = Open("hello1.txt");

  fileLength = Read(buffer, size, fileId1);

  PrintNum(fileLength);

  // Seek(2, fileId2);

  Write(buffer, fileLength, fileId2);



  // Read(buffer, size, fileId2);
  // PrintNum(buffer[0]);


  PrintNum(Close(fileId1));
  PrintNum(Close(fileId2));

  Remove("hello1.txt");


  Halt();
  /* not reached */
}
