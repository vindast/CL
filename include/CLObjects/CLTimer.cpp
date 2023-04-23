#include "CLTimer.h"


void CL::Timer::start()
{
	_startPoint = std::chrono::high_resolution_clock::now();
}

void CL::Timer::stop()
{ 
	_endPoint = std::chrono::high_resolution_clock::now();
	_timeIntervalInSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(_endPoint - _startPoint);

	_lastTimeInSeconds = _timeIntervalInSeconds.count();
	_lastTimeInMiliSeconds = 1000.0 * _lastTimeInSeconds;
}

float CL::Timer::getLastTimeInMiliSeconds() const
{
	return float(_lastTimeInMiliSeconds);
}

double CL::Timer::getLastTimeInMiliSecondsD() const
{
	return (_lastTimeInMiliSeconds);
}

double CL::Timer::getLastTimeInSeconds() const
{
	return _lastTimeInSeconds;
}

CL::ScopeTimerLock::ScopeTimerLock(Timer& timer) :
	_pTimer(&timer)
{
	_pTimer->start();
}

CL::ScopeTimerLock::~ScopeTimerLock()
{
	_pTimer->stop();
}
