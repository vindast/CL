#pragma once 
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <iostream>

namespace CL
{
	class Barrier
	{
	public:
		Barrier();

		Barrier(size_t nThread);

		void setThreadCount(size_t nThread);

		size_t getThreadCount() const;

		size_t getThreadWaitingCount() const;

		void wait();

		void destroy();

		size_t getGeneration() const;

	private:
		std::atomic_bool _bAlive = true;
		
	
		size_t _nThread = 0;

		std::atomic_size_t _nGeneration = 0;
		std::atomic_size_t _nThreadWaiting = 0; 
		std::condition_variable _waitVarible;
		std::mutex _mutex;

		Barrier(const Barrier&) = delete;
		Barrier& operator = (const Barrier&) = delete;
	};

	/*class ExternBarrierSynch
	{
	public:
		ExternBarrierSynch(size_t nThread) :
			_nThread(nThread)
		{

		}

		void wait()
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_nWaitingThread++;
			_waitVarible.wait(lock);
		}

		void free()
		{
			while (_nWaitingThread < _nThread - 1)
			{

			}

			_waitVarible.notify_all();
		}

	private:
		std::condition_variable _waitVarible;
		std::mutex _mutex;
		size_t _nThread;
		std::atomic_size_t _nWaitingThread = 0;

		ExternBarrierSynch(const ExternBarrierSynch&) = delete;
		ExternBarrierSynch& operator = (const ExternBarrierSynch&) = delete;
	};*/

	class LastInBarrier
	{
	public:
		LastInBarrier();

		LastInBarrier(size_t nThread);

		void setThreadCount(size_t nThread);

		size_t getThreadCount() const;

		size_t getThreadWaitingCount() const;

		bool wait();

		void free(); 

	private:
		size_t _nThread = 0;
		std::atomic_size_t _nLockThread = 0;

		std::atomic_size_t _nThreadWaiting = 0;
		std::condition_variable _waitVarible;
		std::mutex _mutex;

		LastInBarrier(const LastInBarrier&) = delete;
		LastInBarrier& operator = (const LastInBarrier&) = delete;
	};

	class FirstInBarrier
	{
	public:
		FirstInBarrier();

		FirstInBarrier(size_t nThread);

		void setThreadCount(size_t nThread);

		size_t getThreadCount() const;

		size_t getThreadWaitingCount() const;

		bool wait();

		void free();

	private:
		size_t _nThread = 0;
		std::atomic_size_t _nLockThread = 0;

		std::atomic_size_t _nThreadWaiting = 0;
		std::condition_variable _waitVarible;
		std::mutex _mutex;

		FirstInBarrier(const FirstInBarrier&) = delete;
		FirstInBarrier& operator = (const FirstInBarrier&) = delete;
	};
};