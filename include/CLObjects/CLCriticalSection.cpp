#include "CLCriticalSection.h"
#include <synchapi.h> 

namespace CL
{
	CriticalSection::CriticalSection()
	{
		InitializeCriticalSectionAndSpinCount(&_hCS, 1024);
	}

	void CriticalSection::lock()
	{
		EnterCriticalSection(&_hCS);
	}

	void CriticalSection::unlock()
	{
		LeaveCriticalSection(&_hCS);
	}

	CriticalSection::~CriticalSection()
	{
		unlock();
		DeleteCriticalSection(&_hCS);
	}
}
