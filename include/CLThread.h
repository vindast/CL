#pragma once
#include <CLAssert.h>

namespace CL
{
    unsigned long GetCurrentThreadId();

    class ThreadLowLevel
    {
        typedef unsigned long(__stdcall* ThreadFunctionPtr)(void* pData);
    public:
        ThreadLowLevel();
        ThreadLowLevel(ThreadFunctionPtr function, void* pData, const wchar_t* pThreadName);
        void Launch(ThreadFunctionPtr function, void* pData, const wchar_t* pThreadName);
        void Suspend();
        void Resume();
        void Join();
        bool IsAlive() const { return _hThread; }
        bool IsSuspend() const { return _bIsSuspended; }
        unsigned long GetId() const { return _ThreadId; }
        ~ThreadLowLevel();
    private:
        ThreadLowLevel& operator = (const ThreadLowLevel&) = delete;
        ThreadLowLevel(const ThreadLowLevel&) = delete;

        bool _bIsSuspended;
        void* _hThread;
        unsigned long _ThreadId;
    };
}