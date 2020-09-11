#pragma once


namespace CL
{
	template <typename Value> Value max(Value v1, Value v2)
	{
		return v1 > v2 ? v1 : v2;
	}

	template <typename Value> Value min(Value v1, Value v2)
	{
		return v1 < v2 ? v1 : v2;
	}

	bool isPowerOf2(int i);
};