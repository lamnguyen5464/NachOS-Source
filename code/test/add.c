/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main()
{
  OpenFileId *fid = Open("hello.txt");

  PrintNum(Close(fid));

  Halt();
  /* not reached */
}
