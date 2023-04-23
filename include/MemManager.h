#pragma once
#include <iostream>
#include "CLMe"
#include "list.h"

namespace CL
{
	struct MemBlock
	{
		size_t blockId = 0;
		size_t blockOffset = 0;
		bool isFree = true;
	};

	struct MemBlockCounter
	{
		size_t blockStartPosition = 0;
		size_t blockCount = 0;
		bool isFree = true;
	};

	struct MemPtr
	{
		char* mem = nullptr;
		CL::ListIterator<MemBlockCounter> it;
	};

	class MemManager
	{
	public:

		MemManager(size_t blockCount, size_t blockSize):
			_nBlock(blockCount),
			_blockSize(blockSize)
		{

			_mBlock = CL_NEW_ARR( MemBlock, blockCount);

			for (size_t i = 0; i < blockCount; i++)
			{
				_mBlock[i].blockId = i;
				_mBlock[i].blockOffset = i * blockSize; 
			}

			MemBlockCounter counter;
			counter.blockCount = blockCount;
			counter.blockStartPosition = 0;
			counter.isFree = true;

			_memBlockCounter.push_back(counter);
		}
		    
		MemManager(const MemManager&) = delete;
		MemManager& operator = (const MemManager&) = delete;
		   
		void print()
		{
			/*std::cout << "--------------Block statistics begin--------------" << std::endl;

			for (size_t i = 0; i < _nBlock; i++)
			{
				std::cout << "id = " << i << ", free: " << _mBlock[i].isFree << std::endl; 
			}

			std::cout << "---------------Block statistics end---------------" << std::endl; */


			std::cout << "--------------Block counter statistics begin--------------" << std::endl;

			std::cout << "num block: " << _memBlockCounter.size() << std:: endl;

			for (auto it = _memBlockCounter.begin(); it != _memBlockCounter.end(); it++)
			{
				std::cout << "blockStartPosition = " << it().blockStartPosition <<", blockCount : " << it().blockCount << ", free: " << it().isFree << std::endl;
			}

			std::cout << "---------------Block counter statistics end---------------" << std::endl;

		}

		MemPtr aloc(char* buffer, size_t nBytes)
		{
			//std::cout << "--------------aloc begin--------------" << std::endl;

			MemPtr memPtr;
			size_t nBlockCount = nBytes / _blockSize + (nBytes % _blockSize ? 1 : 0);

			//std::cout << "reqested memory: " << nBytes << ", blocks count: " << nBlockCount << std::endl;

			for (auto it = _memBlockCounter.begin(); it != _memBlockCounter.end(); it++)
			{
				if (it().isFree && it().blockCount >= nBlockCount)
				{
					memPtr.mem = &buffer[it().blockStartPosition * _blockSize];
					memPtr.it  = it;


					it().isFree = false;

					size_t freeBlockSize = it().blockCount - nBlockCount;

					memPtr.it().blockCount = nBlockCount;
					 
					//std::cout << "freeBlockSize = " << freeBlockSize << std::endl;

					if (freeBlockSize)
					{
						MemBlockCounter counter;
						counter.blockCount = freeBlockSize;
						counter.blockStartPosition = it().blockStartPosition + nBlockCount;

						//std::cout << "New block( count: " << counter.blockCount << ", position:" << counter.blockStartPosition <<")"<< std::endl;

						_memBlockCounter.insert(it, counter);
					}

					break;
				}
			}
			 
		//	std::cout << "---------------aloc end---------------" << std::endl;
			 
			return memPtr; 
		}

		MemPtr aloc(size_t nBytes)
		{
			//std::cout << "--------------aloc begin--------------" << std::endl;

			MemPtr memPtr;
			size_t nBlockCount = nBytes / _blockSize + (nBytes % _blockSize ? 1 : 0);

			//std::cout << "reqested memory: " << nBytes << ", blocks count: " << nBlockCount << std::endl;

			for (auto it = _memBlockCounter.begin(); it != _memBlockCounter.end(); it++)
			{
				if (it().isFree && it().blockCount >= nBlockCount)
				{
					//memPtr.mem = &buffer[it().blockStartPosition * _blockSize];
					memPtr.it = it;


					it().isFree = false;

					size_t freeBlockSize = it().blockCount - nBlockCount;

					memPtr.it().blockCount = nBlockCount;

					//std::cout << "freeBlockSize = " << freeBlockSize << std::endl;

					if (freeBlockSize)
					{
						MemBlockCounter counter;
						counter.blockCount = freeBlockSize;
						counter.blockStartPosition = it().blockStartPosition + nBlockCount;

						//std::cout << "New block( count: " << counter.blockCount << ", position:" << counter.blockStartPosition <<")"<< std::endl;

						_memBlockCounter.insert(it, counter);
					}

					break;
				}
			}

			//	std::cout << "---------------aloc end---------------" << std::endl;

			return memPtr;
		}

		void free(MemPtr ptr)
		{
			//std::cout << "--------------free begin--------------" << std::endl;

			ptr.mem = nullptr;

			ptr.it().isFree = true;
			 
			//std::cout << "current node ( count: " << ptr.it().blockCount << ", position:" << ptr.it().blockStartPosition << ", is free = " << ptr.it().isFree << ")" << std::endl;

			if (ptr.it != _memBlockCounter.begin())
			{
				//std::cout << "Have preivios node..." << std::endl;

				auto previosNode = ptr.it;
				previosNode--;

				//std::cout << "Node ( count: " << previosNode().blockCount << ", position:" << previosNode().blockStartPosition <<", is free = "<< previosNode().isFree << ")" << std::endl;


				if (previosNode().isFree)
				{
					ptr.it().blockCount        += previosNode().blockCount;
					ptr.it().blockStartPosition = previosNode().blockStartPosition;

					//std::cout << "merged node ( count: " << ptr.it().blockCount << ", position:" << ptr.it().blockStartPosition << ", is free = " << ptr.it().isFree << ")" << std::endl;

					_memBlockCounter.erase(previosNode);
				}
			}

			if (ptr.it != _memBlockCounter.rbegin())
			{
				auto nextNode = ptr.it;
				nextNode++;

				//std::cout << "Have next node..." << std::endl;

				//std::cout << "Node ( count: " << nextNode().blockCount << ", position:" << nextNode().blockStartPosition << ", is free = " << nextNode().isFree << ")" << std::endl;

				if (nextNode().isFree)
				{
					ptr.it().blockCount += nextNode().blockCount;

					//std::cout << "merged node ( count: " << ptr.it().blockCount << ", position:" << ptr.it().blockStartPosition << ", is free = " << ptr.it().isFree << ")" << std::endl;

					_memBlockCounter.erase(nextNode);
				}
			}


			//std::cout << "---------------free end---------------" << std::endl;
		}

		void free()
		{
			_memBlockCounter.clear();

			MemBlockCounter counter;
			counter.blockCount = _nBlock;
			counter.blockStartPosition = 0;
			counter.isFree = true;

			_memBlockCounter.push_back(counter);
		}

		~MemManager()
		{
			CL_DELETE_ARR(_mBlock);
		}

		const List<MemBlockCounter>& getBloks() const
		{
			return _memBlockCounter;
		}

		List<MemBlockCounter>& getBloks() 
		{
			return _memBlockCounter;
		}

	private:
		MemBlock* _mBlock = nullptr;
		size_t _nBlock, _blockSize;

		List<MemBlockCounter> _memBlockCounter;

	};

};