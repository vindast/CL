#include "CLMacroHelper.h"
#include <Windows.h>
#include <tchar.h>

void CL::OutputDebugStringCharPtr(const char* pStr)
{
	OutputDebugString(_T(pStr));
	OutputDebugString(_T("\n"));
}
