#include "CLMemory.h"
#include <unordered_set>
#include "Logger/Logger.h"
#include <iostream>

namespace CL
{
	MemoryController& MemoryController::GetInstance()
	{
		static MemoryController c;
		return c;
	}

	void* MemoryController::Malloc(size_t Size, const char* pDebugStr)
	{
		CL_ASSERT(Size);
		CL_SCOPE_LOCK_GUARD(_CS);
		void* pMemory = malloc(Size);

		if (!pMemory)
		{
			return nullptr;
		}

		InsertControllSection(pMemory, EMemoryType::mem_malloc, pDebugStr);
		return pMemory;
	}

	void MemoryController::Free(void* pMemory, const char* pDebugStr)
	{
		if (!pMemory)
		{
			return;
		}

		CL_SCOPE_LOCK_GUARD(_CS);
		EraseControllSection(pMemory, CL::EMemoryType::mem_malloc, pDebugStr);
		free(pMemory);
	}

	const char* MemoryController::MemoryTypeToSTR(EMemoryType Type)
	{
		switch (Type)
		{
		case CL::EMemoryType::mem_new:
			return " new ObjType(...)";
			break;
		case CL::EMemoryType::mem_new_array:
			return " new ObjType[...]";
			break;
		case CL::EMemoryType::mem_malloc:
			return "void* malloc(...)";
			break;
		case CL::EMemoryType::mem_placement_new:
			return " new(pMem) ObjType(...)";
			break;
		default:
			break;
		}

		CL_CRASH();
		return "INVALIDE MEMORY TYPE!";
	}

	MemoryController::MemoryController()
	{
	
	}

	MemoryController::~MemoryController()
	{
		size_t NumError = _MemoryTable.size() + _PlacementNewMemoryTable.size();

		if (!NumError)
		{
			return;
		}

		Logger Log("MemoryLeak");
		Log.Write("\nCL::~MemoryController(): Memory leak detected!");

		Log.PushMessageFormated("Total leaks detected : %d\n", NumError);

		std::unordered_map<const char*, size_t> UniqueLeaks;

		if (_MemoryTable.size())
		{
			Log.PushMessageFormated("Total memory leaks: %d", _MemoryTable.size());

			UniqueLeaks = FindUniqueLeaks(_MemoryTable);

			for (auto it : UniqueLeaks)
			{
				Log.PushMessageFormated("	Count %d, call from: %s ", it.second, it.first);
			}
		}

		if (_PlacementNewMemoryTable.size())
		{
			Log.PushMessageFormated("Total placement new / delete mismatch: %d", _PlacementNewMemoryTable.size());

			UniqueLeaks = FindUniqueLeaks(_PlacementNewMemoryTable);

			for (auto it : UniqueLeaks)
			{
				Log.PushMessageFormated("	Count %d, call from: %s ", it.second, it.first);
			}
		}
	}

	void MemoryController::InsertControllSection(void* pMemory, EMemoryType Type, const char* pDebugStr)
	{
		if (pMemory == nullptr)
		{
			return;
		}

		CL_SCOPE_LOCK_GUARD(_CS);

		std::unordered_map<size_t, MemoryData>& Table = IsPlacementType(Type) ? _PlacementNewMemoryTable : _MemoryTable;

		auto it = Table.find(size_t(pMemory));

		if (it != Table.end())
		{
			Logger::write("CL::MemoryController()::InsertControllSection(...): CRITICAL ERROR!");
			Logger::pushMessageFormated("	Initial memory type is %s", MemoryTypeToSTR(it->second.Type));
			Logger::pushMessageFormated("	Initial call from %s", it->second.pDebugStr);
			Logger::pushMessageFormated("	New memory type is %s", MemoryTypeToSTR(Type));
			Logger::pushMessageFormated("	New call from %s", pDebugStr);
			CL_CRASH();
		}

		MemoryData Data;
		Data.pMemory = pMemory;
		Data.pDebugStr = pDebugStr;
		Data.Type = Type;
		Table.insert(std::make_pair(size_t(pMemory), Data));
	}

	void MemoryController::EraseControllSection(void* pMemory, EMemoryType Type, const char* pDebugStr)
	{
		if (pMemory == nullptr)
		{
			return;
		}

		CL_SCOPE_LOCK_GUARD(_CS);

		std::unordered_map<size_t, MemoryData>& Table = IsPlacementType(Type) ? _PlacementNewMemoryTable : _MemoryTable;

		auto it = Table.find(size_t(pMemory));

		if (it == Table.end())
		{
			Logger::write("CL::MemoryController()::EraseControllSection(...): CRITICAL ERROR!");
			Logger::pushMessageFormated("	Fail to find %d memory address!", size_t(pMemory));
			Logger::pushMessageFormated("	Memory type is %s", MemoryTypeToSTR(Type));
			Logger::pushMessageFormated("	Call from %s", pDebugStr);
			CL_CRASH();
		}

		if (it->second.Type != Type)
		{
			Logger::write("CL::MemoryController()::EraseControllSection(...): CRITICAL ERROR!");
			Logger::pushMessageFormated("Memory type mismatch : original type is %s, delete / free action call with %s type", MemoryTypeToSTR(it->second.Type), MemoryTypeToSTR(Type));
			Logger::pushMessageFormated("Original call from %s", it->second.pDebugStr);
			Logger::pushMessageFormated("delete / free call from %s", pDebugStr);
			CL_CRASH();
		}

		Table.erase(it);
	}

	std::unordered_map<const char*, size_t> MemoryController::FindUniqueLeaks(const std::unordered_map<size_t, MemoryData>& InMap)
	{
		std::unordered_map<const char*, size_t> UniqueLeaks;

		for (auto LeakIt : InMap)
		{
			auto it = UniqueLeaks.find(LeakIt.second.pDebugStr);

			if (it == UniqueLeaks.end())
			{
				UniqueLeaks.insert(std::make_pair(LeakIt.second.pDebugStr, 1));
			}
			else
			{
				it->second++;
			}
		}

		return UniqueLeaks;
	}
}