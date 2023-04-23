#pragma once 
#include <chrono>

#define CL_SCOPE_TIMER_LOCK(timer)\
CL::ScopeTimerLock cl_macro_timer_lock_##__LINE__(timer);

namespace CL
{
	class Timer final
	{
	public: 
		void start(); 
		void stop(); 
		float getLastTimeInMiliSeconds() const; 
		double getLastTimeInMiliSecondsD() const; 
		double getLastTimeInSeconds() const; 
	private:
		std::chrono::high_resolution_clock::time_point _startPoint, _endPoint;
		std::chrono::duration<double> _timeIntervalInSeconds = std::chrono::duration<double>(0.0);

		double _lastTimeInSeconds = 0.0, _lastTimeInMiliSeconds = 0.0; 
	};

	class ScopeTimerLock final
	{
	public:
		ScopeTimerLock(Timer& timer);
		~ScopeTimerLock();
	private:
		Timer* _pTimer;

		ScopeTimerLock(const ScopeTimerLock&) = delete;
		ScopeTimerLock& operator = (const ScopeTimerLock&) = delete;
	};
}
