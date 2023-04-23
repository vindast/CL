#pragma once  
#include "CLLockGuard.h"
#include <Windows.h> 

#undef min
#undef max

namespace CL
{
	class CriticalSection
	{
		friend class ConditionVariable;
		CL_DELETE_COPY_OPERATORS(CriticalSection)
	public:
		CriticalSection();
		void lock();
		void unlock();
		~CriticalSection();
	private:
		CRITICAL_SECTION _hCS;
	};
}