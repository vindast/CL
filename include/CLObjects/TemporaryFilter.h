#pragma once

namespace CL
{
	class TemporaryFilter
	{
	public:
		TemporaryFilter();
		TemporaryFilter(float fValue, float fInterval);
		void setValue(float fValue);
		void setTimeInterval(float fInterval);
		float getValue() const;
		float getTimeInterval() const;
		void update(float fValue, float dT);
	private:
		float _fValue = 0.0f;
		float _fTimeInterval = 1.0f;
	};
};