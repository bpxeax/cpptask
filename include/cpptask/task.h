/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2011, Kyrylo Kolodiazhnyi
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _TASK_H_
#define _TASK_H_

#include "taskthread.h"
#include "mutex.h"
#include "event.h"
#include "alignedalloc.h"

#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <stdlib.h>

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4324 )
#endif

namespace cpptask
{

class alignas(_CPP_TASK_CACHE_LINE_SIZE_) Task
{
public:
    Task()
    {
        waitEvent.Reset();
    }
    virtual ~Task(){}
    virtual void Execute() = 0;

    void SignalDone()
    {
        waitEvent.Signal();
    }

    void Run()
    {
        try
        {
            Execute();
        }
        catch(...)
        {
            lastException = std::current_exception();
        }
    }

    std::exception_ptr GetLastException() const
    {
        return lastException;
    }

    bool CheckFinished()
    {
        if (waitEvent.Check())
        {
            return true;
        }
        return false;
    }

    void Wait()
    {
        waitEvent.Wait();
    }

    Event* GetWaitEvent()
    {
        return &waitEvent;
    }

    void* operator new(size_t size)
    {
        return aligned_alloc(_CPP_TASK_CACHE_LINE_SIZE_, size);
    }

    void operator delete(void* ptr)
    {
        free(ptr);
    }

    void operator delete(void* ptr, size_t)
    {
        free(ptr);
    }

    Task(const Task&) = delete;
    const Task& operator=(const Task&) = delete;
private:
    std::exception_ptr lastException;
    Mutex exceptionGuard;
    Event waitEvent;
};

}

#ifdef _WIN32
#pragma warning( pop )
#endif

#endif
