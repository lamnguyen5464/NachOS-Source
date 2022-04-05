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

  OpenFileId fid = Open("hello.txt");

  Read(buffer, size, fid);

  // PrintNum(Close(fid));

  Halt();
  /* not reached */
}
