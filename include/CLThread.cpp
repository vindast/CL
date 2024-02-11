#include "CLThread.h"
#include <Windows.h>
#include <processthreadsapi.h>

typedef HRESULT(WINAPI* TSetThreadDescription)(HANDLE, PCWSTR);

namespace CL
{
    unsigned long GetCurrentThreadId()
    {
        return ::GetCurrentThreadId();
    }

    ThreadLowLevel::ThreadLowLevel() : _hThread(NULL), _bIsSuspended(false)
    {

    }

    ThreadLowLevel::ThreadLowLevel(ThreadFunctionPtr function, void* pData, const wchar_t* pThreadName) : ThreadLowLevel()
    {
        Launch(function, pData, pThreadName);
    }

    void ThreadLowLevel::Launch(ThreadFunctionPtr function, void* pData, const wchar_t* pThreadName)
    {
        CL_ASSERT(!_hThread);

        _hThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            function,       // thread function name
            pData,          // argument to thread function 
            0,                      // use default creation flags 
            &_ThreadId);   // returns the thread identifier 

        SetThreadPriority(_hThread, THREAD_PRIORITY_TIME_CRITICAL);
        TSetThreadDescription gpSetThreadDescription = nullptr;
        HMODULE hKernel32 = GetModuleHandleA("Kernel32.dll");
        gpSetThreadDescription = reinterpret_cast<TSetThreadDescription>(
            GetProcAddress(hKernel32, "SetThreadDescription"));

        gpSetThreadDescription(_hThread, pThreadName);
    }

    void ThreadLowLevel::Suspend()
    {
        if (!_bIsSuspended && IsAlive())
        {
            SuspendThread(_hThread);
            _bIsSuspended = true;
        }
    }

    void ThreadLowLevel::Resume()
    {
        if (_bIsSuspended && IsAlive())
        {
            ResumeThread(_hThread);
            _bIsSuspended = false;
        }
    }

    void ThreadLowLevel::Join()
    {
        if (_hThread)
        {
            WaitForMultipleObjects(1, &_hThread, TRUE, INFINITE);
            CloseHandle(_hThread);
            _hThread = NULL;
        }
    }

    ThreadLowLevel::~ThreadLowLevel()
    {
        Join();
    }
}


