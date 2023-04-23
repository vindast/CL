#pragma once
#include <map>
#include <string>

namespace CL
{
	template<class ObjType> std::string uniqueName(const std::string& sStartName, const std::map<const std::string, ObjType>& map)
	{
		std::string sName = sStartName;
		size_t iId = 0;

		while (map.find(sName) != map.end())
		{
			sName = sStartName + "_" + std::to_string(iId++);
		}

		return sName;
	}

	template <typename Value> __forceinline Value max(Value v1, Value v2)
	{
		return v1 > v2 ? v1 : v2;
	}

	template <typename Value> __forceinline Value min(Value v1, Value v2)
	{
		return v1 < v2 ? v1 : v2;
	}

	template <typename Value> __forceinline Value abs(Value v)
	{
		return v > Value(0) ? v : -v;
	}

	bool isPowerOf2(int i);
	float RandFZeroOne();
	float RandFInterval(float from, float to);
};