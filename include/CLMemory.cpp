#include "CLMemory.h"
#include "Logger/Logger.h"

// #TODO code cleanup

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

		InsertControllSection(pMemory, EMemoryType::mem_malloc, pDebugStr, Size);
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

		auto ByteToMb = [](size_t Byte)->float
		{
			return float(Byte) / (1024.0f * 1024.0f);
		};

		Logger Log("MemoryLeak");
		Log.Write("\nCL::~MemoryController(): Memory leak detected!");
		Log.PushMessageFormated("Total leaks detected : %d\n", NumError);

		size_t TotalLeak = 0;
		std::unordered_map<const char*, MemoryLeakData> UniqueLeaks;

		if (_MemoryTable.size())
		{
			UniqueLeaks = FindUniqueLeaks(_MemoryTable, TotalLeak);
			Log.PushMessageFormated("Total memory leaks: %d, %f MB", _MemoryTable.size(), ByteToMb(TotalLeak));

			for (auto it : UniqueLeaks)
			{
				Log.PushMessageFormated("	Count %d, %f MB, call from: %s ", it.second.Count, ByteToMb(it.second.Size), it.first);
			}
		}

		if (_PlacementNewMemoryTable.size())
		{
			Log.PushMessageFormated("Total placement new / delete mismatch: %d", _PlacementNewMemoryTable.size());
			UniqueLeaks = FindUniqueLeaks(_PlacementNewMemoryTable, TotalLeak);

			for (auto it : UniqueLeaks)
			{
				Log.PushMessageFormated("	Count %d, call from: %s ", it.second.Count, it.first);
			}
		}
	}

	void MemoryController::InsertControllSection(void* pMemory, EMemoryType Type, const char* pDebugStr, size_t Size)
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
		Data.Size = Size;
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

			if (IsPlacementType(Type))
			{
				bool bMemBlockFound = false;

				for (auto It: _MemoryTable)
				{
					const MemoryData& MemData = It.second;

					if (MemData.pMemory <= pMemory && pMemory <= (char*)MemData.pMemory + MemData.Size)
					{
						CL_ASSERT(!bMemBlockFound);
						bMemBlockFound = true;
						Logger::pushMessageFormated("	Source memory block  address %d", size_t(MemData.pMemory));
						Logger::pushMessageFormated("	Memory type is %s", MemoryTypeToSTR(MemData.Type));
						Logger::pushMessageFormated("	Call from %s", MemData.pDebugStr);
					}
				}

				for (auto It : _PlacementNewMemoryTable)
				{
					const MemoryData& MemData = It.second;

					if (MemData.pMemory <= pMemory && pMemory <= (char*)MemData.pMemory + MemData.Size)
					{
						CL_ASSERT(!bMemBlockFound);
						bMemBlockFound = true;
						Logger::pushMessageFormated("	Source memory block  address %d", size_t(MemData.pMemory));
						Logger::pushMessageFormated("	Memory type is %s", MemoryTypeToSTR(MemData.Type));
						Logger::pushMessageFormated("	Call from %s", MemData.pDebugStr);
					}
				}

				if (!bMemBlockFound)
				{
					Logger::write("	Can`t find source memory block location!");
				}
			}

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

	std::unordered_map<const char*, MemoryController::MemoryLeakData> MemoryController::FindUniqueLeaks(const std::unordered_map<size_t, MemoryData>& InMap, size_t& TotalLeak)
	{
		std::unordered_map<const char*, MemoryLeakData> UniqueLeaks;

		for (auto LeakIt : InMap)
		{
			TotalLeak += LeakIt.second.Size;
			auto it = UniqueLeaks.find(LeakIt.second.pDebugStr);

			if (it == UniqueLeaks.end())
			{
				UniqueLeaks.insert(std::make_pair(LeakIt.second.pDebugStr, MemoryLeakData(1, LeakIt.second.Size)));
			}
			else
			{
				it->second.Count++;
				it->second.Size += LeakIt.second.Size;
			}
		}

		return UniqueLeaks;
	}
	
	void MemoryController::MemoryData::Empty()
	{
		PreviousType = Type;
		pPreviousDebugStr = pDebugStr;
		Type = EMemoryType::mem_empty;
	}

	MemoryController::MemoryLeakData::MemoryLeakData(size_t InCount, size_t InSize) :
		Count(InCount), Size(InSize)
	{

	}
}