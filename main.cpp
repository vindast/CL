#include <iostream>
#include <CLHashMap.h>
#include "include/CLObjects/CLTimer.h"
#include <unordered_map>
#include <Delegate.h>
#include <CLString.h>
#include <FilesSystem.h>

struct HashMapPair
{
	size_t Key;
	CL::String Value;
};

// #TODO add to the CL::HashMap const iterator and make the CL::HashMap& as constant.
void Validate(std::vector<HashMapPair> mTestValues, const std::unordered_map<size_t, CL::String>& StdMap, CL::HashMap& ClMap)
{
	for (size_t i = 0; i < mTestValues.size(); i++)
	{
		HashMapPair P = mTestValues[i];
		auto it = ClMap.Find(P.Key);
		auto stdit = StdMap.find(P.Key);

		if (!it)
		{
			std::cout << "Key = " << P.Key << " fail to find " << std::endl;
			if (stdit != StdMap.end())
			{
				std::cout << "std = (" << stdit->first << ", " << stdit->second << ")" << std::endl;
			}

			if (stdit != StdMap.end())
			{
				ClMap.Find(P.Key);
			}

			CL_ASSERT(stdit == StdMap.end());
		}
		else
		{
			std::cout << "Key = " << P.Key << ", value = " << P.Value << ", found = (" << it.GetKey() <<", " << it.GetValue() << "), std = (" << stdit->first <<", " << stdit->second << ")" << std::endl;
			CL_ASSERT(P.Key == it.GetKey());
			CL_ASSERT(P.Value == it.GetValue() || stdit->second == it.GetValue());
			CL_ASSERT(stdit != StdMap.end() && stdit->second == it.GetValue());
		}
	}
}

int main(int argc, const char* argv[])
{
	srand(0);
	bool bTestStd = true;

	//CL::HashMap<size_t, CL::String> ClMap;
	CL::HashMap ClMap;
	std::unordered_map<size_t, CL::String> StdMap;
	std::vector<HashMapPair> mTestValues;

	size_t NumTestValues = 256;

	for (size_t i = 0; i < NumTestValues; i++)
	{
		size_t Key = ((rand() << 48) | (rand() << 32)) | (rand() << 16);
		HashMapPair P = { Key, CL::ToString((int)i) };

		ClMap.Insert(CL::HashMapPair(P.Key, CL::String(P.Value)));
		StdMap.insert(std::make_pair(P.Key, CL::String(P.Value)));
		mTestValues.push_back(P);

		//	Validate(mTestValues, StdMap, ClMap);
	}

	Validate(mTestValues, StdMap, ClMap);

	size_t count = 0;
	size_t deletionIndex = 0;
	for (auto it = ClMap.begin(); it != ClMap.end(); )
	{
		std::cout << ++count << ":" << ClMap.GetNumElements() << ":" << (count - deletionIndex) << " Key = " << it.GetKey() << ", Value = " << it.GetValue() << std::endl;

		if (rand() % 2)
		{
			deletionIndex++;

			auto stdit = StdMap.find(it.GetKey());

			CL_ASSERT(stdit != StdMap.end());

			StdMap.erase(stdit);

			it = ClMap.Erase(it);
		}
		else
		{
			it++;
		}
	}

	Validate(mTestValues, StdMap, ClMap);

	struct TestCondition
	{
		TestCondition() = default;
		TestCondition(const size_t InNumElemetns) : NumElements(InNumElemetns)
		{

		}
		void Print() const
		{
			auto PercentageDif = [](float A, float B) -> float
			{
				float Ratio = B / A;
				return -(Ratio - 1.0f) * 100.0f;
			};

			std::cout << "NumElements = " << NumElements << std::endl;
			std::cout << "TotalStdInsertTime = " << TotalStdInsertTime << ", TotalCLInsertTime = " << TotalCLInsertTime << " " << PercentageDif(TotalStdInsertTime, TotalCLInsertTime) << "%" << std::endl;
			std::cout << "TotalStdSearchTime = " << TotalStdSearchTime << ", TotalCLSearchTime = " << TotalCLSearchTime << " " << PercentageDif(TotalStdSearchTime, TotalCLSearchTime) << "%" << std::endl;
			std::cout << "TotalStdEraseTimer = " << TotalStdEraseTimer << ", TotalCLEraseTimer = " << TotalCLEraseTimer << " " << PercentageDif(TotalStdEraseTimer, TotalCLEraseTimer) << "%" << std::endl;
		}
		TestCondition& operator += (const TestCondition& InTest)
		{
			NumElements += InTest.NumElements;
			TotalStdInsertTime += InTest.TotalStdInsertTime;
			TotalStdSearchTime += InTest.TotalStdSearchTime;
			TotalStdEraseTimer += InTest.TotalStdEraseTimer;
			TotalCLInsertTime += InTest.TotalCLInsertTime;
			TotalCLSearchTime += InTest.TotalCLSearchTime;
			TotalCLEraseTimer += InTest.TotalCLEraseTimer;

			return *this;
		}

		size_t NumElements;

		float TotalStdInsertTime = 0.0f;
		float TotalStdSearchTime = 0.0f;
		float TotalStdEraseTimer = 0.0f;
		float TotalCLInsertTime = 0.0f;
		float TotalCLSearchTime = 0.0f;
		float TotalCLEraseTimer = 0.0f;
	};

	CL::Vector<TestCondition> Tests;

	for (size_t i = 0; i < 49; i++)
	{
		Tests.PushBack(TestCondition(20 * (i + 1)));
	}

	for (size_t i = 0; i < 16; i++)
	{
		Tests.PushBack(TestCondition(1000 * (i + 1)));
	}

	const size_t NumIterationPerTests = 16;

	for (TestCondition& Test : Tests)
	{
		for (size_t test = 0; test < NumIterationPerTests; test++)
		{
			CL::HashMap ClMap;
			std::unordered_map<size_t, CL::String> StdMap;

			CL::Timer StdInsertTime, CLInsertTime;
			CL::Timer StdSearchTime, CLSearchTime;
			CL::Timer StdEraseTimer, CLEraseTimer;

			std::vector<int> Keys;
			std::vector<bool> Erases;

			for (size_t i = 0; i < Test.NumElements; i++)
			{
				size_t Key = ((rand() << 48) | (rand() << 32)) | (rand() << 16) + (test > 5 ? i : 0);
				Keys.push_back(Key);
				Erases.push_back(rand() % 2);
			}

			for (size_t i = 0; i < Test.NumElements; i++)
			{
				size_t second = rand() % Test.NumElements;
				std::swap(Keys[i], Keys[second]);
			}

			{
				CL_SCOPE_TIMER_LOCK(CLInsertTime);

				for (size_t i = 0; i < Test.NumElements; i++)
				{
					size_t Key = Keys[i];
					ClMap.Insert(CL::HashMapPair(Key, CL::ToString(int(Key))));
				}
			}

			if (bTestStd)
			{
				CL_SCOPE_TIMER_LOCK(StdInsertTime);

				for (size_t i = 0; i < Test.NumElements; i++)
				{
					size_t Key = Keys[i];
					StdMap.insert(std::make_pair(Key, CL::ToString(int(Key))));
				}
			}

			{
				CL_SCOPE_TIMER_LOCK(CLSearchTime);

				for (size_t i = 0; i < Test.NumElements; i++)
				{
					ClMap.Find(i);
				}
			}

			if (bTestStd)
			{
				CL_SCOPE_TIMER_LOCK(StdSearchTime);

				for (size_t i = 0; i < Test.NumElements; i++)
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

			Test.TotalStdInsertTime += StdInsertTime.getLastTimeInMiliSeconds();
			Test.TotalStdSearchTime += StdSearchTime.getLastTimeInMiliSeconds();
			Test.TotalStdEraseTimer += StdEraseTimer.getLastTimeInMiliSeconds();
			Test.TotalCLInsertTime += CLInsertTime.getLastTimeInMiliSeconds();
			Test.TotalCLSearchTime += CLSearchTime.getLastTimeInMiliSeconds();
			Test.TotalCLEraseTimer += CLEraseTimer.getLastTimeInMiliSeconds();
		}
	}

	TestCondition TotalResult;

	for (const TestCondition& Test : Tests)
	{
		TotalResult += Test;
		Test.Print();
	}

	std::cout << "-------------------------------" << std::endl;
	std::cout << "Total result ( iteration per test " << NumIterationPerTests << "):" << std::endl;
	TotalResult.Print();

	system("pause");

	return 0;
}