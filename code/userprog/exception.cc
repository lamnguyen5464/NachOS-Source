// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "filesys.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in kernel->machine.h.
//----------------------------------------------------------------------

// Doi thanh ghi Program counter cua he thong ve sau 4 byte de tiep tuc nap lenh
void IncreasePC()
{
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

// Input: Khong gian dia chi User(int) - gioi han cua buffer(int)
// Output: Bo nho dem Buffer(char*)
// Chuc nang: Sao chep vung nho User sang vung nho System
char *User2System(int virtAddr, int limit)
{
	int i; // chi so index
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; // can cho chuoi terminal
	if (kernelBuf == NULL)
		return kernelBuf;

	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: Khong gian vung nho User(int) - gioi han cua buffer(int) - bo nho dem buffer(char*)
// Output: So byte da sao chep(int)
// Chuc nang: Sao chep vung nho System sang vung nho User
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

			ASSERTNOTREACHED();
			break;

		case SC_Create:
		{
			// Input: Dia chi tu vung nho user cua ten file
			// Output: -1 = Loi, 0 = Thanh cong
			// Chuc nang: Tao ra file voi tham so la ten file

			char *filename;
			DEBUG(dbgSys, " CreateFile call ...\n");

			int virtAddr = kernel->machine->ReadRegister(4); // Doc dia chi cua file tu thanh ghi R4

			// Sao chep khong gian bo nho User sang System, voi do dang toi da la (32 + 1) bytes
			int MaxFileLength = 32;
			filename = User2System(virtAddr, MaxFileLength + 1);

			if (strlen(filename) == 0)
			{
				DEBUG(dbgSys, "Invalid fileName\n");
				kernel->machine->WriteRegister(2, -1); // Return -1 vao thanh ghi R2
				IncreasePC();
				return;
				break;
			}

			if (filename == NULL) // Neu khong doc duoc
			{
				DEBUG(dbgSys, "Invalid fileName\n");
				kernel->machine->WriteRegister(2, -1); // Return -1 vao thanh ghi R2
				delete filename;
				IncreasePC();
				return;
				break;
			}

			if (!kernel->fileSystem->Create(filename)) // Tao file bang ham Create cua kernel->fileSystem, tra ve ket qua
			{
				// Tao file that bai
				DEBUG(dbgSys, "Failed to create file\n");
				kernel->machine->WriteRegister(2, -1);
				delete filename;
				IncreasePC();
				return;
				break;
			}

			// Tao file thanh cong
			kernel->machine->WriteRegister(2, 0);
			delete filename;
			IncreasePC();
			return;
			break;
		}

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			IncreasePC();

			return;

			ASSERTNOTREACHED();

			break;

		case SC_ReadNum:
		{
			int MAX_LENGTH_INT = 11;
			char *buffer = new char[MAX_LENGTH_INT + 1];
			int currentLength = 0;

			int MAX_INT = 2147483647;  // 2^31 - 1
			int MIN_INT = -2147483648; // -2^31

			bool isNegative = false;

			while (true)
			{
				char currentDigit = kernel->synchConsoleIn->GetChar(); // input character from console

				bool isValidMinus = (currentDigit == '-' && currentLength == 0);
				bool isValidDigit = ('0' <= currentDigit && currentDigit <= '9');

				if (isValidDigit || isValidMinus)
				{
					buffer[currentLength++] = currentDigit;
				}
				else
				{
					// break if there is invalid character (or '\n')
					break;
				}
				isNegative = isNegative || isValidMinus;
			}

			long long convertedValue = 0;
			for (int i = isNegative ? 1 : 0; i < currentLength; i++)
			{ // accumulate char to number
				convertedValue = convertedValue * 10 + (buffer[i] - '0');
			}

			convertedValue *= isNegative ? -1 : 1; // add - if it is negative

			if (currentLength > MAX_LENGTH_INT || convertedValue < MIN_INT || convertedValue > MAX_INT)
			{
				convertedValue = 0;
				// DEBUG(dbgSys, "Over size of integer");
			}
			else
			{
				DEBUG(dbgSys, "Result: " << convertedValue << "\n"); // print result
			}

			kernel->machine->WriteRegister(2, convertedValue);
			IncrementPC();
			delete buffer;
			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_PrintNum:
		{
			long long res = kernel->machine->ReadRegister(4); // input number

			if (res == 0)
			{ // handle adhoc case
				kernel->synchConsoleOut->PutChar('0');
				IncrementPC();
				return;
			}

			// init phase
			char *buffer = new char[12];

			int currentIndex = 0;

			long long tmp = res;
			int countDigit = 0;

			while (tmp)
			{ // count number of digit
				tmp /= 10;
				countDigit++;
			}
			if (res < 0)
			{ // add minus if it is negative number
				buffer[countDigit] = '-';
				tmp = -res;
			}
			else
			{
				countDigit--;
				tmp = res;
			}

			while (tmp)
			{ // split each digit in num
				buffer[currentIndex] = (char)((tmp % 10) + 48);
				currentIndex++;
				tmp /= 10;
			}
			for (int i = countDigit; i >= 0; i--)
			{ // print out
				kernel->synchConsoleOut->PutChar(buffer[i]);
			}

			delete buffer;
			IncrementPC();
			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_ReadChar:
		{
			int MAX_BUFFER = 256;
			char *input = new char[MAX_BUFFER + 1];
			int index = -1;

			do
			{
				input[++index] = kernel->synchConsoleIn->GetChar(); // read until reach break line sign or reach limit szie
			} while (input[index] != '\n' && index <= MAX_BUFFER);

			if (index > 1)
			{
				DEBUG(dbgSys, "Not a single character \n");
			}

			kernel->machine->WriteRegister(2, input[0]);

			IncrementPC();
			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_PrintChar:
		{
			char res = (char)(kernel->machine->ReadRegister(4));

			kernel->synchConsoleOut->PutChar(res);

			// kernel->synchConsoleOut->PutChar('\n');

			IncrementPC();
			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_RandomNum:
		{
			int i, stime;
			long ltime;

			/* get the current calendar time */
			ltime = time(NULL);
			stime = (unsigned)ltime / 2;
			srand(stime);

			kernel->machine->WriteRegister(2, rand());

			IncrementPC();
			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_ReadString:
		{
			int addressBuffer = kernel->machine->ReadRegister(4);
			int length = kernel->machine->ReadRegister(5);

			int index = 0;
			int tmp = length;
			char *buffer = User2System(addressBuffer, length);

			while (tmp)
			{
				buffer[index] = kernel->synchConsoleIn->GetChar();
				if (buffer[index] == '\n')
					break; // read until reach LineFeed
				index++;
				tmp--;
			}

			System2User(addressBuffer, length, buffer);

			delete buffer;

			IncrementPC();
			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_PrintString:
		{
			int MAX_LENGTH_STRING = 1000;
			int addressBuffer = kernel->machine->ReadRegister(4);

			char *buffer = User2System(addressBuffer, MAX_LENGTH_STRING);

			int index = 0;
			while (buffer[index] != '\0')
			{
				index++;
			}

			for (int i = 0; i < index; i++)
			{
				kernel->synchConsoleOut->PutChar(buffer[i]);
			}

			delete buffer;

			IncrementPC();
			return;
			ASSERTNOTREACHED();
			break;
		}

		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;

	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
	ASSERTNOTREACHED();
}
