#pragma once

namespace CL::Test
{
	void PrintTimeComparsion(float clTime, float stdTime);

	void TestFindString(float& clTotalTime, float& stdTotalTime, size_t nTestIterationForType, size_t nTestIterationPerValue);
	void TestConvetationSignedIntToStr(float& clTotalTime, float& stdTotalTime, size_t nTestIterationForType, size_t nTestIterationPerValue);
	void TestConvetationUnsignedIntToStr(float& clTotalTime, float& stdTotalTime, size_t nTestIterationForType, size_t nTestIterationPerValue);

	void TestString();
}