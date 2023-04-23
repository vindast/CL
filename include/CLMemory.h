#pragma once
#include "CLAssert.h"
#include <CLObjects/CLCriticalSection.h>
#include <unordered_map>

#define CL_MEMORY_LEAK_DEBUG

#ifdef CL_MEMORY_LEAK_DEBUG
#define CL_FRIEND_MEMORY_CONTROLLER() friend class CL::MemoryController;

#define CL_PLACEMENT_NEW(mem, type, ...)\
CL::MemoryController::GetInstance().PlacementNew<type>(mem, CL_FUNCSIG(), __VA_ARGS__)

#define CL_PLACEMENT_DELETE(pObj)\
CL::MemoryController::GetInstance().PlacementDelete(pObj, CL_FUNCSIG())

#define CL_NEW(type, ...)\
CL::MemoryController::GetInstance().New<type>(CL_FUNCSIG(), __VA_ARGS__)

#define CL_DELETE(pObj)\
CL::MemoryController::GetInstance().Delete(pObj, CL_FUNCSIG())

#define CL_NEW_ARR(type, lenght)\
CL::MemoryController::GetInstance().NewArr<type>(lenght, CL_FUNCSIG())

#define CL_DELETE_ARR(pObjs)\
CL::MemoryController::GetInstance().DeleteArr(pObjs, CL_FUNCSIG())

#define CL_MALLOC(size)\
CL::MemoryController::GetInstance().Malloc(size, CL_FUNCSIG())

#define CL_FREE(pMem)\
CL::MemoryController::GetInstance().Free(pMem, CL_FUNCSIG())

#else
#define CL_FRIEND_MEMORY_CONTROLLER()

#define CL_PLACEMENT_NEW(mem, type, ...)\
new(mem) type(__VA_ARGS__)

#define CL_PLACEMENT_DELETE(pObj)\
CL::MemoryController::PlacementDeleteImplementation(pObj)

#define CL_NEW(type, ...)\
new type(__VA_ARGS__)

#define CL_DELETE(pObj)\
delete pObj

#define CL_NEW_ARR(type, lenght)\
new type[lenght]

#define CL_DELETE_ARR(pObjs)\
delete[] pObjs

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
		};

		CL_DELETE_COPY_OPERATORS(MemoryController)
	public:
		template <class ObjType>
		static void PlacementDeleteImplementation(ObjType* pObj)
		{
			pObj->~ObjType();
		}
		static MemoryController& GetInstance();
		template<class ObjType, class... Args>
		ObjType* PlacementNew(void* pMem, const char* pDebugStr, Args&&... args)
		{
			ObjType* pObj = new(pMem) ObjType(args...);
			InsertControllSection(pObj, EMemoryType::mem_placement_new, pDebugStr);
			return pObj;
		}
		template<class ObjType>
		void PlacementDelete(ObjType* pObj, const char* pDebugStr)
		{
			EraseControllSection(pObj, CL::EMemoryType::mem_placement_new, pDebugStr);
			PlacementDeleteImplementation(pObj);
		}
		template<class ObjType, class... Args>
		ObjType* New(const char* pDebugStr, Args&&... args)
		{
			ObjType* pObj = new ObjType(args...);
			InsertControllSection(pObj, EMemoryType::mem_new, pDebugStr);
			return pObj;
		}
		template<class ObjType>
		void Delete(ObjType* pObj, const char* pDebugStr)
		{
			EraseControllSection(pObj, CL::EMemoryType::mem_new, pDebugStr);
			delete pObj;
		}
		template<class ObjType>
		ObjType* NewArr(size_t Length, const char* pDebugStr)
		{
			ObjType* pObjs = new ObjType[Length];
			InsertControllSection(pObjs, EMemoryType::mem_new_array, pDebugStr);
			return pObjs;
		}
		template<class ObjType>
		void DeleteArr(ObjType* pObjs, const char* pDebugStr)
		{
			EraseControllSection(pObjs, CL::EMemoryType::mem_new_array, pDebugStr);
			delete[] pObjs;
		}
		void* Malloc(size_t Size, const char* pDebugStr);
		void Free(void* pMemory, const char* pDebugStr);
		bool IsPlacementType(EMemoryType Type) const { return Type == EMemoryType::mem_placement_new; }
		static const char* MemoryTypeToSTR(EMemoryType Type);
	private:
		MemoryController();
		~MemoryController();

		void InsertControllSection(void* pMemory, EMemoryType Type, const char* pDebugStr);
		void EraseControllSection(void* pMemory, EMemoryType Type, const char* pDebugStr);
		static std::unordered_map<const char*, size_t> FindUniqueLeaks(const std::unordered_map<size_t, MemoryData>& InMap);

		//ptr adr - type
		std::unordered_map<size_t, MemoryData> _MemoryTable;
		std::unordered_map<size_t, MemoryData> _PlacementNewMemoryTable;

		CriticalSection _CS;
	};
}