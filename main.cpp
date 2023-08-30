#include <iostream>
#include <CLHashMap.h>
#include "include/CLObjects/CLTimer.h"
#include <unordered_map>
#include <Delegate.h>
#include <CLString.h>
#include <FilesSystem.h>

int main(int argc, const char* argv[])
{
	std::cout << CL::GetFileNameFromPath("C:/repos/TEngine/GameProjects/NewEngineFunctionality/GameData/Asset/file.txt", false) << std::endl;
	std::cout << CL::GetFileNameFromPath("C:\\repos\\TEngine\\GameProjects\\NewEngineFunctionality\\Assets\\OldCar.fbx", false) << std::endl;
	return false;

	CL::String std = CL::Forward(CL::String("sda"));

	struct A
	{
		A() {};
		A(int a) : i(a) {}

		int i = 0;
	};

	CL::EmbeddedArray<A, 4> Array;

	Array.PushBack({ 0 });
	Array.PushBack(1);
	Array.PushBack({ 2 });
	Array.PushBack({ 3 });

	for (auto& it : Array)
	{
		std::cout << it.i << std::endl;
	}

	Array.EraseSwap(1);

	for (auto& it : Array)
	{
		std::cout << it.i << std::endl;
	}

	srand(0);
	bool bTestStd = true;

	struct Pair
	{
		size_t Key;
		CL::String Value;
	};

	CL::HashMap<size_t, CL::String> ClMap;
	std::unordered_map<size_t, CL::String> StdMap;
	std::vector<Pair> mTestValues;

	size_t NumTestValues = 25;

	for (size_t i = 0; i < NumTestValues; i++)
	{
		size_t Key = ((rand() << 48) | (rand() << 32)) | (rand() << 16);
		Pair P = { Key, CL::ToString((int)i) };
		ClMap.Insert(P.Key, CL::String(P.Value));
		StdMap.insert(std::make_pair(P.Key, CL::String(P.Value)));
		mTestValues.push_back(P);
	}

	size_t count = 0;
	for (auto it = ClMap.begin(); it != ClMap.end(); )
	{
		std::cout << ++count << " Key = " << it().Key << ", Value = " << it().Value << std::endl;

		if (rand() % 2)
		{
			auto stdit = StdMap.find(it().Key);
			StdMap.erase(stdit);

			it = ClMap.Erase(it);
			std::cout << "---deleted---" << std::endl;
		}
		else
		{
			it++;
		}
	}

	for (size_t i = 0; i < NumTestValues; i++)
	{
		Pair P = mTestValues[i];
		auto it = ClMap.Find(P.Key);
		auto stdit = StdMap.find(P.Key);

		if (!it)
		{
			std::cout << "Key = " << P.Key << " fail to find " << std::endl;
			CL_ASSERT(stdit == StdMap.end());
		}
		else
		{
			std::cout << "Key = " << P.Key << ", value = " << P.Value << ", found = " << it().Value << std::endl;
			CL_ASSERT(stdit != StdMap.end() && stdit->second == it().Value);
		}
	}

	
	for (size_t test = 0; test < 10; test++)
	{
		CL::HashMap<size_t, CL::String> ClMap;
		std::unordered_map<size_t, CL::String> StdMap;

		size_t NumIteration = 10000;

		CL::Timer StdInsertTime, CLInsertTime;
		CL::Timer StdSearchTime, CLSearchTime;
		CL::Timer StdEraseTimer, CLEraseTimer;

		std::vector<int> Keys;
		std::vector<bool> Erases;

		for (size_t i = 0; i < NumIteration; i++)
		{
			size_t Key = ((rand() << 48) | (rand() << 32)) | (rand() << 16) + (test > 5 ? i : 0);
			Keys.push_back(Key);
			Erases.push_back(rand() % 2);
		}

		for (size_t i = 0; i < NumIteration; i++)
		{
			size_t second = rand() % NumIteration;
			std::swap(Keys[i], Keys[second]);
		}

		{
			CL_SCOPE_TIMER_LOCK(CLInsertTime);

			for (size_t i = 0; i < NumIteration; i++)
			{
				size_t Key = Keys[i];
				ClMap.Insert(Key, CL::ToString(int(Key)));
			}
		}

		if(bTestStd)
		{
			CL_SCOPE_TIMER_LOCK(StdInsertTime);

			for (size_t i = 0; i < NumIteration; i++)
			{
				size_t Key = Keys[i];
				StdMap.insert(std::make_pair(Key, CL::ToString(int(Key))));
			}
		}

		{
			CL_SCOPE_TIMER_LOCK(CLSearchTime);

			for (size_t i = 0; i < NumIteration; i++)
			{
				ClMap.Find(i);
			}
		}

		if (bTestStd)
		{
			CL_SCOPE_TIMER_LOCK(StdSearchTime);

			for (size_t i = 0; i < NumIteration; i++)
			{
				StdMap.find(i);
			}
		}

		{
			CL_SCOPE_TIMER_LOCK(CLEraseTimer);

			size_t Index = 0;
			for (auto it = ClMap.begin(); it != ClMap.end();)
			{
				if (Erases[Index++])
				{
					it = ClMap.Erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		if (bTestStd)
		{
			CL_SCOPE_TIMER_LOCK(StdEraseTimer);

			size_t Index = 0;
			for (auto it = StdMap.begin(); it != StdMap.end();)
			{
				if (Erases[Index++])
				{
					it = StdMap.erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		std::cout << "StdInsertTime = " << StdInsertTime.getLastTimeInMiliSeconds() << ", CLInsertTime = " << CLInsertTime.getLastTimeInMiliSeconds() << std::endl;
		std::cout << "StdSearchTime = " << StdSearchTime.getLastTimeInMiliSeconds() << ", CLSearchTime = " << CLSearchTime.getLastTimeInMiliSeconds() << std::endl;
		std::cout << "StdEraseTimer = " << StdEraseTimer.getLastTimeInMiliSeconds() << ", CLEraseTimer = " << CLEraseTimer.getLastTimeInMiliSeconds() << std::endl;
		std::cout << "-----------" << std::endl;
	}

	system("pause");

	return 0;
}