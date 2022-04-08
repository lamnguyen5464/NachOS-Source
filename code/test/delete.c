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

	Remove(fileName);

  	Halt();
}
