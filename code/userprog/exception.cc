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

#include "synchconsole.h"
#include <stdlib.h>
#include <time.h>
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

    char *argFileName1 = kernel -> tempArgv[0];
    char *argFileName2 = kernel -> tempArgv[1];

    switch (which)
    {
    case SyscallException:
        switch (type)
        {
        case SC_ExecV:
        {

            break;
        }
        case SC_Halt:
            DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

            SysHalt();

            ASSERTNOTREACHED();
            break;

        case SC_Create:
        {
            char *filename;
            DEBUG(dbgSys, " CreateFile call ...\n");
            // Read the address of the memory storing the filename.
            int virtAddr = kernel->machine->ReadRegister(4);
            // Get the value of the filename from the address.
            int MaxFileLength = 32;
            filename = User2System(virtAddr, MaxFileLength + 1);
            // Read arguments from the command lines.
            if (strlen(filename) == 0) {
                filename = argFileName1 != NULL ? argFileName1 : argFileName2;
                kernel -> tempArgv[0] = NULL;
            }
            DEBUG(dbgSys, "Check filename command line:" << filename << "\n");
            // Check if a filename is available or not.
            if (filename == NULL || strlen(filename) == 0)
            {
                DEBUG(dbgSys, "Invalid fileName\n");
                kernel->machine->WriteRegister(2, -1); // Return -1 if not valid.
                IncreasePC();
                return;
                break;
            }
            // Create the file by the function from kernel.
            if (!kernel->fileSystem->Create(filename))
            {
                // Fail to create.
                DEBUG(dbgSys, "Failed to create file\n");
                kernel->machine->WriteRegister(2, -1);
                delete filename;
                IncreasePC();
                return;
                break;
            }
            // Create successfully.
            kernel->machine->WriteRegister(2, 0);
            delete filename;
            IncreasePC();
            return;
            break;
        }

        case SC_Open:
        {
            // Read the address of the memory storing the filename.
            int virtAddr = kernel->machine->ReadRegister(4);
            char *filename;
            int MaxFileLength = 32;
            // Get the current open file id --> to store file in the new position.
            int currentFileId = kernel->fileSystem->numOfOpenedFiles;
            int lengthOfName = strlen(filename);
            int i;
            // Get the string filename from the virtAddress.
            filename = User2System(virtAddr, MaxFileLength);

            if (strlen(filename) == 0) { // user argfileName from command line
                filename = argFileName1 != NULL ? argFileName1 : argFileName2;
                kernel -> tempArgv[0] = NULL;;
            }

	        DEBUG(dbgSys, "Opening file: " << filename << "\n");
            // Check if the filename is valid or not.
            if (filename == NULL) {
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                return;
            }
            OpenFile* file = kernel->fileSystem->Open(filename);
            if (file == NULL) { // Return -1 if open file fails.
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                return;
            }
            // Increase the number of open file.
            kernel->fileSystem->numOfOpenedFiles = (kernel->fileSystem->numOfOpenedFiles + 1) % 10000;
            // Store the file id opening.
            kernel->fileSystem->listOpenedFiles[currentFileId] = file;
            // Create the clone variable to store the filename.
            char* cloneName = new char[100];
            // Copy name.
            strcpy(cloneName, filename);
            DEBUG(dbgSys, "cloneName: " << cloneName << "\n");
            // Store the file name opening.
            kernel->fileSystem->listOpenFileNames[currentFileId] = cloneName;
            kernel->machine->WriteRegister(2, currentFileId);
            IncreasePC();
            return;
        }

        case SC_Close:
        {
            // Get the openFileId from the user.
            OpenFileId openFileId = (OpenFileId)kernel->machine->ReadRegister(4);
            // Get the OpenFile type from the openFileId.
            OpenFile *currentFile = kernel->fileSystem->listOpenedFiles[(int)openFileId];
            if (currentFile == NULL) // If the file does not exist.
            {
                kernel->machine->WriteRegister(2, -1);
            }
            else // If the file exists.
            {
                delete currentFile;
                kernel->machine->WriteRegister(2, (int)openFileId); // Return the OpenFileId of the file which is closed.
            }
            IncreasePC();
            return;
            break;
        }

        case SC_Read:
        {
            int virtAddr = kernel->machine->ReadRegister(4);
            int charcount = kernel->machine->ReadRegister(5);
            int openFileId = (int)kernel->machine->ReadRegister(6);
            int OldPos;
            int NewPos;
            char *buf;
            int size;

            // Get the OpenFile from its id.
            OpenFile *file = kernel->fileSystem->listOpenedFiles[(int)openFileId];
            size = file->Length() + 1;
            buf = new char[size];

            DEBUG(dbgSys, "Start reading:\n");
            DEBUG(dbgSys, "currentFileId: " << openFileId << "\n");

            // Check if the file is opened or not.
            if (file != NULL) { // If file is opened.
                // Call the Read function from class OpenFile to read the content of file.
                file->Read(buf, size - 1);
                // Pass the content of file back to the user.
                System2User(virtAddr, size, buf);
                // Return the length of the file's content.
                kernel->machine->WriteRegister(2, size - 1);
            }
            else // If file is not opened. --> fail.
            {
                kernel->machine->WriteRegister(2, -1);
            }
            delete buf;
            IncreasePC();
            return;
        }

        case SC_Write:
        {
            int virtAddr = kernel->machine->ReadRegister(4);
            int sizeOfInput = kernel->machine->ReadRegister(5);
            int openFileId = kernel->machine->ReadRegister(6);

            // Get the file from the id.
            OpenFile *file = kernel->fileSystem->listOpenedFiles[(int)openFileId];

            if (file != NULL) { // The file is opened.
                // Write the content to the file.
                char *buffer = User2System(virtAddr, sizeOfInput);
                file->Write(buffer, sizeOfInput);
                kernel->machine->WriteRegister(2, sizeOfInput);
            } else { // The file is not opened. --> fail.
                kernel->machine->WriteRegister(2, -1);
            }
            IncreasePC();
            return;
        }

        case SC_Remove:
        {
            char *filename;
            int virtAddr = kernel->machine->ReadRegister(4);
            int MaxFileLength = 32;
            // Get the string file name from the virt address.
            filename = User2System(virtAddr, MaxFileLength + 1);
            bool isOpen = false;
            bool isMatch = false;
            int indexOfFile = 0;
            int i = 0, j = 0;           // iterator that we use in this function
            bool isRemoved = false;

            if (strlen(filename) == 0) { // user argfileName from command line
                filename = argFileName1 != NULL ? argFileName1 : argFileName2;
                kernel -> tempArgv[0] = NULL;;
            }

            DEBUG(dbgSys, "Start removing file: " << filename << "\n");

            for(i = 0; i < 10000; i++) {       // scan through all stored openFile to ensure the removed file is not opened.
                isMatch = true;     // detect if the name is matching

                char *currentFileName = kernel->fileSystem->listOpenFileNames[i];
                // If the file is not opened --> skip
                if (kernel->fileSystem->listOpenedFiles[i] != NULL || currentFileName == NULL ) {
                    // this file is being opened
                    continue;
                }

                // the file name is not matched with the open file name --> skip.
                if (strlen(currentFileName) != strlen(filename)) {
                    continue;
                }
                isMatch = strcmp(filename, currentFileName);

                DEBUG(dbgSys, "FileName: " << filename << "\n");
                DEBUG(dbgSys, "currentFileName: " << filename <<"\n");
                DEBUG(dbgSys, "isMatch: " << isMatch <<"\n");

                // for(j = 0; j < strlen(currentFileName); j++) {
                //     if (filename[j] != currentFileName[j]) {
                //         isMatch = false;
                //         break;
                //     }
                // }

                if (isMatch) {
                    isOpen = true;
                    indexOfFile = i;
                    break;
                }
            }

            DEBUG(dbgSys, "is open: " << isOpen << "\n");

            if (isOpen) { // if the file is opening --> can not remove.
                DEBUG(dbgSys, "File is opening!" << "\n");
                kernel->machine->WriteRegister(2, 0);
            } else { // if the removed file is not open.
                char *fileName = kernel->fileSystem->listOpenFileNames[indexOfFile];

                // call the Remove function from the system.
                isRemoved = kernel->fileSystem->Remove(fileName);

                // Return -1 if removed failed and 0 if success.
                if (!isRemoved) {
                    DEBUG(dbgSys, "Remove fails" << "\n");
                    kernel->machine->WriteRegister(2, -1);
                } else{
                    kernel->machine->WriteRegister(2, 0);
                }
            }
            IncreasePC();
            return;

        }

        case SC_Seek:
        {
            int position = kernel->machine->ReadRegister(4);
            OpenFileId openFileId = (OpenFileId)kernel->machine->ReadRegister(5);
            int seekPos = -1;

            OpenFile *currentFile = kernel->fileSystem->listOpenedFiles[(int)openFileId];
            // Check if the file is opened or not.
            if (currentFile != NULL) {
                // Get the position to seek to.
                int currentFileLength = currentFile->Length();
                seekPos = (position != -1 ? position : currentFileLength);
                // If the seek position larger then the file length --> seek to the file length
                if (seekPos > currentFileLength)
                {
                    seekPos = currentFileLength;
                }
                currentFile->Seek(seekPos);
            } else {
                DEBUG(dbgSys, "Seek fails." << "\n");
            }
            kernel->machine->WriteRegister(2, seekPos);
            IncreasePC();
            return;
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
            IncreasePC();
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
                IncreasePC();
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
            IncreasePC();
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

            IncreasePC();
            return;
            ASSERTNOTREACHED();
            break;
        }

        case SC_PrintChar:
        {
            char res = (char)(kernel->machine->ReadRegister(4));

            kernel->synchConsoleOut->PutChar(res);

            // kernel->synchConsoleOut->PutChar('\n');

            IncreasePC();
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

            IncreasePC();
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

            IncreasePC();
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

            IncreasePC();
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

