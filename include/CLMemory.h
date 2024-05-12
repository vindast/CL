#pragma once
#include "CLAssert.h"
#include <CLObjects/CLCriticalSection.h>
#include <unordered_map>

// #TODO CLConfig
// #define CL_MEMORY_LEAK_DEBUG

#ifdef CL_MEMORY_LEAK_DEBUG

#define CL_PLACEMENT_NEW(mem, type, ...)\
CL::MemoryController::GetInstance().PlacementNew<type>(mem, new(mem) type(__VA_ARGS__), CL_FUNCSIG())

#define CL_PLACEMENT_DELETE(type, pObj)\
CL::MemoryController::GetInstance().PlacementDelete(pObj, CL_FUNCSIG())->~type()

#define CL_NEW(type, ...)\
CL::MemoryController::GetInstance().New<type>(new type(__VA_ARGS__), CL_FUNCSIG())

#define CL_DELETE(pObj)\
delete CL::MemoryController::GetInstance().Delete(pObj, CL_FUNCSIG())

#define CL_NEW_ARR(type, lenght)\
CL::MemoryController::GetInstance().NewArr<type>(new type[lenght], lenght, CL_FUNCSIG())

#define CL_DELETE_ARR(pObjs)\
delete[] CL::MemoryController::GetInstance().DeleteArr(pObjs, CL_FUNCSIG())

#define CL_MALLOC(size)\
CL::MemoryController::GetInstance().Malloc(size, CL_FUNCSIG())

#define CL_FREE(pMem)\
CL::MemoryController::GetInstance().Free(pMem, CL_FUNCSIG())

#else

#define CL_PLACEMENT_NEW(mem, type, ...)\
new(mem) type(__VA_ARGS__)

#define CL_PLACEMENT_DELETE(type, pObj)\
(pObj)->~type()

#define CL_NEW(type, ...)\
new type(__VA_ARGS__)

#define CL_DELETE(pObj)\
delete (pObj)

#define CL_NEW_ARR(type, lenght)\
new type[lenght]

#define CL_DELETE_ARR(pObjs)\
delete[] (pObjs)

#define CL_MALLOC(size)\
malloc(size)

#define CL_FREE(pMem)\
free(pMem)

#endif



namespace CL
{
	enum class EMemoryType
	{
		mem_new,
		mem_new_array,
		mem_placement_new,
		mem_malloc
	};

	class MemoryController
	{
		struct MemoryData
		{
			void* pMemory;
			EMemoryType Type;
			const char* pDebugStr;
			size_t Size;
		};

		struct MemoryLeakData
		{
			MemoryLeakData() = default;
			MemoryLeakData(size_t InCount, size_t InSize) :
				Count(InCount), Size(InSize)
			{

			}

			size_t Count = 0;
			size_t Size = 0;
		};

		CL_DELETE_COPY_OPERATORS(MemoryController)
	public:
		static MemoryController& GetInstance();
		template<class ObjType, class... Args>
		ObjType* PlacementNew(void* pMem, ObjType* pObj, const char* pDebugStr)
		{
			InsertControllSection(pObj, EMemoryType::mem_placement_new, pDebugStr, 0);
			return pObj;
		}
		template<class ObjType>
		ObjType* PlacementDelete(ObjType* pObj, const char* pDebugStr)
		{
			EraseControllSection(pObj, CL::EMemoryType::mem_placement_new, pDebugStr);
			return pObj;
		}
		template<class ObjType>
		ObjType* New(ObjType* pObj, const char* pDebugStr)
		{
			InsertControllSection(pObj, EMemoryType::mem_new, pDebugStr, sizeof(ObjType));
			return pObj;
		}
		template<class ObjType>
		ObjType* Delete(ObjType* pObj, const char* pDebugStr)
		{
			EraseControllSection(pObj, CL::EMemoryType::mem_new, pDebugStr);
			return pObj;
		}
		template<class ObjType>
		ObjType* NewArr(ObjType* pObjs, size_t Length, const char* pDebugStr)
		{
			InsertControllSection(pObjs, EMemoryType::mem_new_array, pDebugStr, sizeof(ObjType) * Length);
			return pObjs;
		}
		template<class ObjType>
		ObjType* DeleteArr(ObjType* pObjs, const char* pDebugStr)
		{
			EraseControllSection(pObjs, CL::EMemoryType::mem_new_array, pDebugStr);
			return pObjs;
		}
		void* Malloc(size_t Size, const char* pDebugStr);
		void Free(void* pMemory, const char* pDebugStr);
		bool IsPlacementType(EMemoryType Type) const { return Type == EMemoryType::mem_placement_new; }
		static const char* MemoryTypeToSTR(EMemoryType Type);
	private:
		MemoryController();
		~MemoryController();

		void InsertControllSection(void* pMemory, EMemoryType Type, const char* pDebugStr, size_t Size);
		void EraseControllSection(void* pMemory, EMemoryType Type, const char* pDebugStr);
		static std::unordered_map<const char*, MemoryLeakData> FindUniqueLeaks(const std::unordered_map<size_t, MemoryData>& InMap, size_t& TotalLeak);

		//ptr adr - type
		std::unordered_map<size_t, MemoryData> _MemoryTable;
		std::unordered_map<size_t, MemoryData> _PlacementNewMemoryTable;

		CriticalSection _CS;
	};
}