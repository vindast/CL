#pragma once 
#include <string>
#include <map>

namespace CL
{
	template<class MapKey, class MapObj> std::string uniqueName(const std::string& sStartName, const std::map<MapKey, MapObj>& m)
	{
		std::string sName = sStartName;
		size_t iId = 0;

		while (m.find(sName) != m.end())
		{
			sName = sStartName + "_" + std::to_string(iId++);
		}

		return sName;
	}
};