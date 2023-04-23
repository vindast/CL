#pragma once

namespace CL
{
	class Clock
	{
	public:
		void setClockTime(float fTimeDelta);
		bool onClock(float dT);
	private:
		float _fTime = 0.0f;
		float _fTimeDelta = 1.0f;
	};
};