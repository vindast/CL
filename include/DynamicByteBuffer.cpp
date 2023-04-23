#include "DynamicByteBuffer.h"
#include <CLMemory.h>
#include <iostream>

CL::DynamicByteBuffer::DynamicByteBuffer()
{

}

void CL::DynamicByteBuffer::write(size_t iCursorPosition, size_t iSize, const char* pData)
{
	if (_nSize < iCursorPosition + iSize)
	{
		size_t nAlocatedBytes = iCursorPosition + iSize - _nSize;
		nAlocatedBytes = nAlocatedBytes > _nMinAlocatedSize ? nAlocatedBytes : _nMinAlocatedSize;
		requestSize(_nSize + nAlocatedBytes);
	}

	if (pData)
	{
		memcpy(&_pData[iCursorPosition], pData, iSize);
	}
}

void CL::DynamicByteBuffer::writeAsync(size_t iCursorPosition, size_t iSize, const char* pData)
{
	LockGuard<CriticalSection> lock(_hCS); 
	write(iCursorPosition, iSize, pData);
}

void CL::DynamicByteBuffer::requestSizeAsync(size_t nBytes, bool bCopyOldData)
{
	LockGuard<CriticalSection> lock(_hCS);
	requestSize(nBytes, bCopyOldData);
}

void CL::DynamicByteBuffer::requestSize(size_t nBytes, bool bCopyOldData)
{
	if (_nSize < nBytes)
	{
		size_t nAlocatedBytes = nBytes - _nSize;

		nAlocatedBytes = nAlocatedBytes > _nMinAlocatedSize ? nAlocatedBytes : _nMinAlocatedSize;

		char* pTmpData = (char*)CL_MALLOC(_nSize + nAlocatedBytes);

		if (!pTmpData)
		{
			std::cout << "---------------------CL::Error---------------------------------" << std::endl;
			std::cout << "DynamicByteBuffer(): fail to alocate new buffer" << std::endl;
			std::cout << "nAlocatedBytes = " << nAlocatedBytes << std::endl;
			std::cout << "_nSize         = " << _nSize << std::endl;
			std::cout << "---------------------------------------------------------------" << std::endl;
			throw std::exception("CL::DynamicByteBuffer(): fali to alocate new buffer (out of memory)!");
		}

		if (_pData)
		{
			if (bCopyOldData)
			{
				memcpy(pTmpData, _pData, _nSize);
			}

			CL_FREE(_pData);
		}

		_pData = pTmpData;

		_nSize += nAlocatedBytes;
	}
}

char* CL::DynamicByteBuffer::data()
{
	return _pData;
}

const char* CL::DynamicByteBuffer::data() const
{
	return _pData;
}

size_t CL::DynamicByteBuffer::size() const
{
	return _nSize;
}

CL::DynamicByteBuffer::~DynamicByteBuffer()
{
	if (_pData)
	{
		CL_FREE(_pData);
	}
}
