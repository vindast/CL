#pragma once
#include "CLCriticalSection.h"

namespace CL
{
	class ConditionVariable
	{
		CL_DELETE_COPY_OPERATORS(ConditionVariable)
	public:
		ConditionVariable();
		void Wait(CL::CriticalSection& CS, unsigned long WaitTimeMS = 0xFFFFFFFF);
		void NotifyOne();
		void NotifyAll();
		~ConditionVariable();
	private:
		CONDITION_VARIABLE _hCV;
	};
}