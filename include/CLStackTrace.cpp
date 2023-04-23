#include "CLStackTrace.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "dbghelp.h"

#pragma comment(lib, "Dbghelp.lib");

#define TRACE_MAX_STACK_FRAMES 1024
#define TRACE_MAX_FUNCTION_NAME_LENGTH 1024

namespace CL
{
	void PrintStack()
	{
        const ULONG framesToSkip = 0;
        const ULONG framesToCapture = 64;
        void* stack[framesToCapture]{};
        ULONG backTraceHash = 0;

        const USHORT nFrame = CaptureStackBackTrace(
            framesToSkip
            , framesToCapture
            , stack
            , &backTraceHash
        );

        HANDLE process = GetCurrentProcess();
        char buf[sizeof(SYMBOL_INFO) + (TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR)];

        SYMBOL_INFO* symbol = (SYMBOL_INFO*)buf;
        symbol->MaxNameLen = TRACE_MAX_FUNCTION_NAME_LENGTH;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        SymInitialize(process, NULL, TRUE);
        SymSetOptions(SYMOPT_LOAD_LINES);

        DWORD displacement = 0;
        IMAGEHLP_LINE64 line;

        for (USHORT iFrame = 0; iFrame < nFrame; ++iFrame) {

            DWORD64 address = (DWORD64)(stack[iFrame]);
            SymFromAddr(process, address, NULL, symbol);
            if (SymGetLineFromAddr64(process, address, &displacement, &line))
            {
                printf("\tat %s in %s: line: %lu: address: 0x%0X\n", symbol->Name, line.FileName, line.LineNumber, symbol->Address);
            }
            else
            {
                printf("\tSymGetLineFromAddr64 returned error code %lu.\n", GetLastError());
                printf("\tat %s, address 0x%0X.\n", symbol->Name, symbol->Address);
            }
        }
    
	}
}