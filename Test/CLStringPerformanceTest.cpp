#include "CLStringPerformanceTest.h"
#include <string>
#include <iostream>
#include <vector>
#include <CLStringUtils.h>
#include <CLObjects/CLTimer.h>

void CL::Test::PrintTimeComparsion(float clTime, float stdTime)
{
	printf("cl time : %6.3fMs, std time : %6.3fMs, std / cl %% = %6.3f%%\n", clTime, stdTime, 100.0f * stdTime / clTime);
}

void CL::Test::TestFindString(float& clTotalTime, float& stdTotalTime, size_t nTestIterationForType, size_t nTestIterationPerValue)
{
	printf("\nfind substring (const char*)\n");

	std::vector<const char*> mStringToTest =
	{
		"Oh shit, I'm sorry",
		"Sorry for what ? Our daddy taught us not to be ashamed of our dicks, especially since they're such a good size and all.",
		"Yeah, I see that.Your daddy gave you good advice.",
		"It gets bigger when I pull on it.",
		"Hmmmm!",
		"Sometimes, I pull on it so hard, I RIP THE SKIN",
		"Well, my daddy taught me a few things too, like, uh, how not to rip the skin by using someone else's mouth, instead of your own hands.",
		"Will you show me ?",
		"I'd be right happy to."
	};

	std::vector<const char*> mStringToFind =
	{
		"I",
		"daddy",
		"that",
		"bigger",
		"m",
		"pull",
		"things",
		"me",
		"be"
	};

	std::vector<char> mCharToFind =
	{
		'I',
		'd',
		't',
		'b',
		'm',
		'p',
		't',
		'e'
	};

	CL::String clStr;
	std::string stdStr;
	CL::Timer testTimer;
	float clLocalTime = 0.0f;
	float stdLocalTime = 0.0f;
	size_t n = 0;

	for (const char* pSrcString : mStringToTest)
	{
		clStr = pSrcString;
		stdStr = pSrcString;

		printf("Source str = \"%s\"\n", pSrcString);

		for (const char* pStringToFind : mStringToFind)
		{
			float clTime = 0.0f;
			float stdTime = 0.0f;

			for (size_t i = 0; i < nTestIterationPerValue; i++)
			{
				{
					CL_SCOPE_TIMER_LOCK(testTimer);

					for (int i = 0; i < nTestIterationForType; i++)
					{
						n += clStr.FindFirst(pStringToFind);
					}
				}

				clTime += testTimer.getLastTimeInMiliSeconds();

				{
					CL_SCOPE_TIMER_LOCK(testTimer);

					for (size_t i = 0; i < nTestIterationForType; i++)
					{
						n += stdStr.find(pStringToFind);
					}
				}

				stdTime += testTimer.getLastTimeInMiliSeconds();
			}

			clLocalTime += clTime;
			stdLocalTime += stdTime;

			printf("String to find = \"%10s\", stdTime = %10.3fms, clTime = %10.3fms, stdTime / clTime = %5.2f%%\n", pStringToFind, stdTime, clTime, stdTime / clTime * 100.0f);
		}

		for (char charToFind : mCharToFind)
		{
			float clTime = 0.0f;
			float stdTime = 0.0f;

			for (size_t i = 0; i < nTestIterationPerValue; i++)
			{
				{
					CL_SCOPE_TIMER_LOCK(testTimer);

					for (int i = 0; i < nTestIterationForType; i++)
					{
						n += clStr.FindFirst(charToFind);
					}
				}

				clTime += testTimer.getLastTimeInMiliSeconds();

				{
					CL_SCOPE_TIMER_LOCK(testTimer);

					for (size_t i = 0; i < nTestIterationForType; i++)
					{
						n += stdStr.find(charToFind);
					}
				}

				stdTime += testTimer.getLastTimeInMiliSeconds();
			}

			clLocalTime += clTime;
			stdLocalTime += stdTime;

			printf("String to find = \"%c\", stdTime = %10.3fms, clTime = %10.3fms, stdTime / clTime = %5.2f%%\n", charToFind, stdTime, clTime, stdTime / clTime * 100.0f);
		}
	}

	PrintTimeComparsion(clLocalTime, stdLocalTime);

	clTotalTime += clLocalTime;
	stdTotalTime += stdLocalTime;
}

void CL::Test::TestConvetationSignedIntToStr(float& clTotalTime, float& stdTotalTime, size_t nTestIterationForType, size_t nTestIterationPerValue)
{
	printf("\nsigned int(32) -> string test\n");

	CL::String clStr;
	std::string stdStr;
	CL::Timer testTimer;

	std::vector<int> mIntTestValues =
	{
		-2147483647,
		-214748364,
		-21474836,
		-2147483,
		-214748,
		-21474,
		-214,
		-21,
		-2,
		0,
		2,
		21,
		214,
		2147,
		21474,
		214748,
		2147483,
		21474836,
		214748364,
		2147483647
	};

	float clLocalTime = 0.0f;
	float stdLocalTime = 0.0f;

	for (int num : mIntTestValues)
	{
		float clTime = 0.0f;
		float stdTime = 0.0f;

		for (size_t i = 0; i < nTestIterationPerValue; i++)
		{
			{
				CL_SCOPE_TIMER_LOCK(testTimer);

				for (int i = 0; i < nTestIterationForType; i++)
				{
					clStr = CL::ToString(num);
				}
			}

			clTime += testTimer.getLastTimeInMiliSeconds();

			{
				CL_SCOPE_TIMER_LOCK(testTimer);

				for (size_t i = 0; i < nTestIterationForType; i++)
				{
					stdStr = std::to_string(num);
				}
			}

			stdTime += testTimer.getLastTimeInMiliSeconds();
		}

		clLocalTime += clTime;
		stdLocalTime += stdTime;

		printf("num = %-11i, stdTime = %10.3fms, clTime = %10.3fms, stdTime / clTime = %5.2f%%\n", num, stdTime, clTime, stdTime / clTime * 100.0f);
	}

	PrintTimeComparsion(clLocalTime, stdLocalTime);

	clTotalTime  += clLocalTime;
	stdTotalTime += stdLocalTime;
}

void CL::Test::TestConvetationUnsignedIntToStr(float& clTotalTime, float& stdTotalTime, size_t nTestIterationForType, size_t nTestIterationPerValue)
{
	printf("\nunsigned int(32) -> string test\n");

	CL::String clStr;
	std::string stdStr;
	CL::Timer testTimer;

	std::vector<unsigned int> mUnsignedIntTestValues =
	{
		4294967295,
		429496729,
		42949672,
		4294967,
		429496,
		42949,
		4294,
		429,
		42,
		4
	};

	float clLocalTime = 0.0f;
	float stdLocalTime = 0.0f;

	for (unsigned int num : mUnsignedIntTestValues)
	{
		float clTime = 0.0f;
		float stdTime = 0.0f;

		for (size_t i = 0; i < nTestIterationPerValue; i++)
		{
			{
				CL_SCOPE_TIMER_LOCK(testTimer);

				for (int i = 0; i < nTestIterationForType; i++)
				{
					clStr = CL::ToString(num);
				}
			}

			clTime += testTimer.getLastTimeInMiliSeconds();

			{
				CL_SCOPE_TIMER_LOCK(testTimer);

				for (size_t i = 0; i < nTestIterationForType; i++)
				{
					stdStr = std::to_string(num);
				}
			}

			stdTime += testTimer.getLastTimeInMiliSeconds();
		}

		clLocalTime += clTime;
		stdLocalTime += stdTime;

		printf("num = %-11u, stdTime = %10.3fms, clTime = %10.3fms, stdTime / clTime = %5.2f%%\n", num, stdTime, clTime, stdTime / clTime * 100.0f);
	}

	PrintTimeComparsion(clLocalTime, stdLocalTime);

	clTotalTime += clLocalTime;
	stdTotalTime += stdLocalTime;
}

void CL::Test::TestString()
{
	printf("\nbegin <CL::Test::TestString()>...\n");

	const size_t NumTestPerValue = 100;
	const size_t NumTestPerType = 100000;

	float clTotalTime = 0.0f;
	float stdTotalTime = 0.0f;

	TestFindString(clTotalTime, stdTotalTime, NumTestPerType * 100, NumTestPerValue * 100);
	TestConvetationSignedIntToStr(clTotalTime, stdTotalTime, NumTestPerType, NumTestPerValue);
	TestConvetationUnsignedIntToStr(clTotalTime, stdTotalTime, NumTestPerType, NumTestPerValue);




	printf("\nend <CL::Test::TestString()>...\n");
}
