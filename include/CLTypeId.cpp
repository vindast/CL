#include "CLTypeId.h"

size_t CL::TypeId::CreateNewTypeIndex()
{
	static size_t TypeIndex = 0;
	TypeIndex++;
	return TypeIndex;
} 