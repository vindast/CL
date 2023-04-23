#include "TemporaryFilter.h"

CL::TemporaryFilter::TemporaryFilter()
{

}

CL::TemporaryFilter::TemporaryFilter(float fValue, float fInterval) :
	_fValue(fValue), _fTimeInterval(fInterval)
{

}

void CL::TemporaryFilter::setValue(float fValue)
{
	_fValue = fValue;
}

void CL::TemporaryFilter::setTimeInterval(float fInterval)
{
	_fTimeInterval = fInterval;
}

float CL::TemporaryFilter::getValue() const
{
	return _fValue;
}

float CL::TemporaryFilter::getTimeInterval() const
{
	return _fTimeInterval;
}

void CL::TemporaryFilter::update(float fValue, float dT)
{
	dT = dT > _fTimeInterval ? _fTimeInterval : dT;
	_fValue = (_fValue * (_fTimeInterval - dT) + dT * fValue) / _fTimeInterval;
}

