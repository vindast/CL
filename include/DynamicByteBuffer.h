#pragma once
#include <cstdlib>
#include <iostream>
#include "CLObjects/CLCriticalSection.h"
 
namespace CL
{
	class DynamicByteBuffer
	{
	public:
		DynamicByteBuffer();
		void write(size_t iCursorPosition, size_t iSize, const char* pData);
		void writeAsync(size_t iCursorPosition, size_t iSize, const char* pData);
		void requestSizeAsync(size_t nBytes, bool bCopyOldData = true);
		void requestSize(size_t nBytes, bool bCopyOldData = true);
		char* data();
		const char* data() const;
		size_t size() const;
		~DynamicByteBuffer();
	private:
		const size_t _nMinAlocatedSize = 1024;
		CriticalSection _hCS;
		size_t _nSize = 0;
		char* _pData = nullptr;

		DynamicByteBuffer(const DynamicByteBuffer&) = delete;
		DynamicByteBuffer& operator = (const DynamicByteBuffer&) = delete;
	};
};