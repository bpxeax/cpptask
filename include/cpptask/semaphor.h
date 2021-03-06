/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2012, Kyrylo Kolodiazhnyi
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

#ifndef _CPP_TASK_SEMAPHORE_H_
#define _CPP_TASK_SEMAPHORE_H_

#include "taskmanager.h"

#include <condition_variable>

namespace cpptask
{
    class semaphore
    {
    public:
        explicit semaphore(int n = 0, int max = -1)
            : maxCount { max }
            , count{ n }
        {}

        semaphore(const semaphore&) = delete;

        semaphore& operator=(const semaphore&) = delete;

        void unlock()
        {
            std::lock_guard<std::mutex> lock{ guard };
            if ((maxCount > 0 && count < maxCount) ||
                 maxCount <= 0)
            {
                ++count;
                cv.notify_one();
            }
        }

        void lock()
        {
            std::unique_lock<std::mutex> lock{ guard };
            cv.wait(lock, [&] { return count > 0; });
            --count;
        }

        bool try_lock()
        {
            std::unique_lock<std::mutex> lock{ guard };
            if (cv.wait_for(lock, std::chrono::milliseconds(0), [&] { return count > 0; }))
            {
                --count;
                return true;
            }
            return false;
        }

    private:
        int maxCount;
        int count;
        std::condition_variable cv;
        std::mutex guard;
    };

    //use this class only if you have corresponding process_lock object
    class lockable_semaphore
    {
    public:
        typedef internal::EventManager EventManagerType;

        explicit lockable_semaphore(int n = 0, int max = -1) noexcept
            : sem(n, max)
        {}

        ~lockable_semaphore()
        {}

        lockable_semaphore(const lockable_semaphore&) = delete;
        const lockable_semaphore& operator=(const lockable_semaphore&) = delete;

        void lock()
        {
            sem.lock();
        }

        void unlock()
        {
            sem.unlock();
            internal::TaskManager::GetCurrent().GetEventManager().notify(internal::EventId::CustomEvent);
        }

        bool try_lock()
        {
            return sem.try_lock();
        }

    private:
        semaphore sem;
    };
}
#endif
