#pragma once
#include <atomic>
#include "Buffer.h"

namespace CL
{
	template<class t> class BufferAtomicPtrItarator
	{
	public:
		BufferAtomicPtrItarator(Buffer<t>* buffer)
		{
			_buffer = buffer;
		}

		BufferAtomicPtrItarator(Buffer<t>& buffer)
		{
			_buffer = &buffer;
		}
		 
		void reset()
		{
			_id = 0;
		}

		t* getNext()
		{
			size_t x = _id++;
 
			if (x < _buffer->getSize())
			{
				return &(*_buffer)[x];
			}
			
			return nullptr;
		}

	private:
		Buffer<t>* _buffer = nullptr;
		std::atomic<size_t> _id = 0;

		BufferAtomicPtrItarator(const BufferAtomicPtrItarator&)
		{

		}

		BufferAtomicPtrItarator& operator = (const BufferAtomicPtrItarator&)
		{

		}
	};
}