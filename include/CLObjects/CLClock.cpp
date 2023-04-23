#include "CLClock.h"

void CL::Clock::setClockTime(float fTimeDelta)
{
	_fTimeDelta = fTimeDelta;
}

bool CL::Clock::onClock(float dT)
{
	bool bClock = false;
	_fTime += dT;

	if (_fTime > _fTimeDelta)
	{
		bClock = true;
		_fTime -= _fTimeDelta;
	}

	return bClock;
}
