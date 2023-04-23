#include "Barrier.h" 

CL::Barrier::Barrier()
{

}

CL::Barrier::Barrier(size_t nThread) :
	_nThread(nThread)
{

}

void CL::Barrier::setThreadCount(size_t nThread)
{
	_nThread = nThread;
}

size_t CL::Barrier::getThreadCount() const
{
	return _nThread;
}

size_t CL::Barrier::getThreadWaitingCount() const
{
	return _nThreadWaiting;
}

void CL::Barrier::wait()
{
	if (_bAlive)
	{
		std::unique_lock<std::mutex> lock(_mutex);

		size_t nGen = _nGeneration;

		if (_nThreadWaiting++ == (_nThread - 1))
		{
			_nGeneration++;
			 
			_nThreadWaiting = 0;
			 
			_waitVarible.notify_all(); 
		} 
		else
		{
			while (nGen == _nGeneration && _bAlive)
			{
				_waitVarible.wait(lock);
			}
		} 
	} 
}

void CL::Barrier::destroy()
{
	_bAlive = false;
	_waitVarible.notify_all();
}

size_t CL::Barrier::getGeneration() const
{
	return _nGeneration;
}

CL::LastInBarrier::LastInBarrier()
{

}

CL::LastInBarrier::LastInBarrier(size_t nThread) :
	_nThread(nThread)
{

}

void CL::LastInBarrier::setThreadCount(size_t nThread)
{
	_nThread = nThread;
}

size_t CL::LastInBarrier::getThreadCount() const
{
	return _nThread;
}

size_t CL::LastInBarrier::getThreadWaitingCount() const
{
	return _nThreadWaiting;
}

bool CL::LastInBarrier::wait()
{
	std::unique_lock<std::mutex> lock(_mutex);
	size_t nGen = _nGen;
	bool bResult = _nThreadWaiting++ == _nThread - 1;

	if (!bResult)
	{
		while (nGen == _nGen)
		{
			_waitVarible.wait(lock);
		}
	}

	return bResult;
}

void CL::LastInBarrier::free()
{
	_nGen++;
	_nThreadWaiting = 0;
	_waitVarible.notify_all();
}

CL::FirstInBarrier::FirstInBarrier()
{

}

CL::FirstInBarrier::FirstInBarrier(size_t nThread) :
	_nThread(nThread)
{

}

void CL::FirstInBarrier::setThreadCount(size_t nThread)
{
	_nThread = nThread;
}

size_t CL::FirstInBarrier::getThreadCount() const
{
	return _nThread;
}

size_t CL::FirstInBarrier::getThreadWaitingCount() const
{
	return _nThreadWaiting;
}

bool CL::FirstInBarrier::wait()
{
	std::unique_lock<std::mutex> lock(_mutex);

	if (!_nThread)
	{
		throw std::exception("_nThread = 0 ");
	}

	bool bResult = _nThreadWaiting++;

	if (bResult)
	{
		_nLockThread++;
		_waitVarible.wait(lock);
		_nLockThread--;
	}

	return !bResult;
} 

void CL::FirstInBarrier::free()
{ 
	while (_nThreadWaiting != _nThread || _nLockThread != _nThread - 1)
	{
	//	std::cout << "_nThread = " << _nThread << ", _nThreadWaiting = " << _nThreadWaiting << std::endl;
	} 
	 
	_nThreadWaiting = 0;

	while (_nLockThread)
	{ 
		_waitVarible.notify_one();
	}
}

void CL::ExternalBarrier::free()
{
	_nGen++;
	_waitVarible.notify_all();
}

void CL::ExternalBarrier::wait()
{
	std::unique_lock<std::mutex> lock(_mutex);
	size_t nGen = _nGen;
	_onWait = true;

	while (nGen == _nGen)
	{
		_waitVarible.wait(lock);
	}

	_onWait = false;
}

bool CL::ExternalBarrier::onWait() const
{
	return _onWait;
}
