#include "common.h"

bool CL::isPowerOf2(int i)
{
	return i > 0 && !(i & (i - 1));
}

float CL::RandFZeroOne()
{
	return static_cast<float>(rand() % 10000) / 10000.0f;
}

float CL::RandFInterval(float from, float to)
{
	return from + RandFZeroOne() * (to - from);
}