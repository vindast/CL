#pragma once
#include "CLString.h"
#include "CLVector.h"
#include <stdarg.h>

#ifndef CL_STR_MAX_FORMAT_CAPACITY
#define CL_STR_MAX_FORMAT_CAPACITY 2048
#endif

namespace CL
{
	String ToString(unsigned int n) noexcept;
	String ToString(int n) noexcept;
	String FormatString(const char* fmt, ...);
	void SplitBy(CL::Vector<CL::String>& OutStrings, CL::String SourseString, char SplitSymbol);;
}