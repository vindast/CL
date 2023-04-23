#include "CLConditionVariable.h"

namespace CL
{
	ConditionVariable::ConditionVariable()
	{
		InitializeConditionVariable(&_hCV);
	}

	void ConditionVariable::Wait(CL::CriticalSection& CS, unsigned long WaitTimeMS)
	{
		SleepConditionVariableCS(&_hCV, &CS._hCS, WaitTimeMS);
	}

	void ConditionVariable::NotifyOne()
	{
		WakeConditionVariable(&_hCV);
	}

	void ConditionVariable::NotifyAll()
	{
		WakeAllConditionVariable(&_hCV);
	}

	ConditionVariable::~ConditionVariable()
	{
		NotifyAll();
	}
}