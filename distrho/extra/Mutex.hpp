/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2022 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DISTRHO_MUTEX_HPP_INCLUDED
#define DISTRHO_MUTEX_HPP_INCLUDED

#include "../DistrhoUtils.hpp"

#ifdef DISTRHO_OS_WINDOWS
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# include <winsock2.h>
# include <windows.h>
#endif

// FIXME make Mutex stop relying on pthread
#ifdef _MSC_VER
#define DISTRHO_OS_WINDOWS__TODO
#pragma NOTE(DPF Mutex implementation is TODO on MSVC)
#else
#include <pthread.h>
#endif


class Signal;

// -----------------------------------------------------------------------
// Mutex class

class Mutex
{
public:
    Mutex(const bool inheritPriority = true) noexcept
       #ifdef DISTRHO_OS_WINDOWS__TODO
        : fSection()
       #else
        : fMutex()
       #endif
    {
       #ifdef DISTRHO_OS_WINDOWS__TODO
        InitializeCriticalSection(&fSection);
       #else
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setprotocol(&attr, inheritPriority ? PTHREAD_PRIO_INHERIT : PTHREAD_PRIO_NONE);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
        pthread_mutex_init(&fMutex, &attr);
        pthread_mutexattr_destroy(&attr);
       #endif
    }

    ~Mutex() noexcept
    {
       #ifdef DISTRHO_OS_WINDOWS__TODO
        DeleteCriticalSection(&fSection);
       #else
        pthread_mutex_destroy(&fMutex);
       #endif
    }

    bool lock() noexcept
    {
       #ifdef DISTRHO_OS_WINDOWS__TODO
        EnterCriticalSection(&fSection);
        return true;
       #else
        return (pthread_mutex_lock(&fMutex) == 0);
       #endif
    }

    bool tryLock() noexcept
    {
       #ifdef DISTRHO_OS_WINDOWS__TODO
        return (TryEnterCriticalSection(&fSection) != FALSE);
       #else
        return (pthread_mutex_trylock(&fMutex) == 0);
       #endif
    }

    void unlock() noexcept
    {
       #ifdef DISTRHO_OS_WINDOWS__TODO
        LeaveCriticalSection(&fSection);
       #else
        pthread_mutex_unlock(&fMutex);
       #endif
    }

private:
   #ifdef DISTRHO_OS_WINDOWS__TODO
    CRITICAL_SECTION fSection;
   #else
    mutable pthread_mutex_t fMutex;
   #endif

    DISTRHO_DECLARE_NON_COPYABLE(Mutex)
};

// -----------------------------------------------------------------------
// RecursiveMutex class

class RecursiveMutex
{
public:
    RecursiveMutex() noexcept
       #ifdef DISTRHO_OS_WINDOWS
        : fSection()
       #else
        : fMutex()
       #endif
    {
       #ifdef DISTRHO_OS_WINDOWS
        InitializeCriticalSection(&fSection);
       #else
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&fMutex, &attr);
        pthread_mutexattr_destroy(&attr);
       #endif
    }

    ~RecursiveMutex() noexcept
    {
       #ifdef DISTRHO_OS_WINDOWS
        DeleteCriticalSection(&fSection);
       #else
        pthread_mutex_destroy(&fMutex);
       #endif
    }

    bool lock() const noexcept
    {
       #ifdef DISTRHO_OS_WINDOWS
        EnterCriticalSection(&fSection);
        return true;
       #else
        return (pthread_mutex_lock(&fMutex) == 0);
       #endif
    }

    bool tryLock() const noexcept
    {
       #ifdef DISTRHO_OS_WINDOWS
        return (TryEnterCriticalSection(&fSection) != FALSE);
       #else
        return (pthread_mutex_trylock(&fMutex) == 0);
       #endif
    }

    void unlock() const noexcept
    {
       #ifdef DISTRHO_OS_WINDOWS
        LeaveCriticalSection(&fSection);
       #else
        pthread_mutex_unlock(&fMutex);
       #endif
    }

private:
   #ifdef DISTRHO_OS_WINDOWS
    mutable CRITICAL_SECTION fSection;
   #else
    mutable pthread_mutex_t fMutex;
   #endif

    DISTRHO_DECLARE_NON_COPYABLE(RecursiveMutex)
};

#ifndef _MSC_VER
// -----------------------------------------------------------------------
// Signal class

class Signal
{
public:
    /*
     * Constructor.
     */
    Signal() noexcept
        : fCondition(),
          fMutex(),
          fTriggered(false)
    {
        pthread_condattr_t cattr;
        pthread_condattr_init(&cattr);
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_PRIVATE);
        pthread_cond_init(&fCondition, &cattr);
        pthread_condattr_destroy(&cattr);

        pthread_mutexattr_t mattr;
        pthread_mutexattr_init(&mattr);
        pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT);
        pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_NORMAL);
        pthread_mutex_init(&fMutex, &mattr);
        pthread_mutexattr_destroy(&mattr);
    }

    /*
     * Destructor.
     */
    ~Signal() noexcept
    {
        pthread_cond_destroy(&fCondition);
        pthread_mutex_destroy(&fMutex);
    }

    /*
     * Wait for a signal.
     */
    void wait() noexcept
    {
        pthread_mutex_lock(&fMutex);

        while (! fTriggered)
        {
            try {
                pthread_cond_wait(&fCondition, &fMutex);
            } DISTRHO_SAFE_EXCEPTION("pthread_cond_wait");
        }

        fTriggered = false;

        pthread_mutex_unlock(&fMutex);
    }

    /*
     * Wake up all waiting threads.
     */
    void signal() noexcept
    {
        pthread_mutex_lock(&fMutex);

        if (! fTriggered)
        {
            fTriggered = true;
            pthread_cond_broadcast(&fCondition);
        }

        pthread_mutex_unlock(&fMutex);
    }

private:
    pthread_cond_t  fCondition;
    pthread_mutex_t fMutex;
    volatile bool   fTriggered;

    DISTRHO_PREVENT_HEAP_ALLOCATION
    DISTRHO_DECLARE_NON_COPYABLE(Signal)
};
#endif // _MSC_VER

// -----------------------------------------------------------------------
// Helper class to lock&unlock a mutex during a function scope.

template <class Mutex>
class ScopeLocker
{
public:
    ScopeLocker(const Mutex& mutex) noexcept
        : fMutex(mutex)
    {
        fMutex.lock();
    }

    ~ScopeLocker() noexcept
    {
        fMutex.unlock();
    }

private:
    const Mutex& fMutex;

    DISTRHO_PREVENT_HEAP_ALLOCATION
    DISTRHO_DECLARE_NON_COPYABLE(ScopeLocker)
};

// -----------------------------------------------------------------------
// Helper class to try-lock&unlock a mutex during a function scope.

template <class Mutex>
class ScopeTryLocker
{
public:
    ScopeTryLocker(const Mutex& mutex) noexcept
        : fMutex(mutex),
          fLocked(mutex.tryLock()) {}

    ScopeTryLocker(const Mutex& mutex, const bool forceLock) noexcept
        : fMutex(mutex),
          fLocked(forceLock ? mutex.lock() : mutex.tryLock()) {}

    ~ScopeTryLocker() noexcept
    {
        if (fLocked)
            fMutex.unlock();
    }

    bool wasLocked() const noexcept
    {
        return fLocked;
    }

    bool wasNotLocked() const noexcept
    {
        return !fLocked;
    }

private:
    const Mutex& fMutex;
    const bool   fLocked;

    DISTRHO_PREVENT_HEAP_ALLOCATION
    DISTRHO_DECLARE_NON_COPYABLE(ScopeTryLocker)
};

// -----------------------------------------------------------------------
// Helper class to unlock&lock a mutex during a function scope.

template <class Mutex>
class ScopeUnlocker
{
public:
    ScopeUnlocker(const Mutex& mutex) noexcept
        : fMutex(mutex)
    {
        fMutex.unlock();
    }

    ~ScopeUnlocker() noexcept
    {
        fMutex.lock();
    }

private:
    const Mutex& fMutex;

    DISTRHO_PREVENT_HEAP_ALLOCATION
    DISTRHO_DECLARE_NON_COPYABLE(ScopeUnlocker)
};

// -----------------------------------------------------------------------
// Define types

typedef ScopeLocker<Mutex>          MutexLocker;
typedef ScopeLocker<RecursiveMutex> RecursiveMutexLocker;

typedef ScopeTryLocker<Mutex>          MutexTryLocker;
typedef ScopeTryLocker<RecursiveMutex> RecursiveMutexTryLocker;

typedef ScopeUnlocker<Mutex>          MutexUnlocker;
typedef ScopeUnlocker<RecursiveMutex> RecursiveMutexUnlocker;

// -----------------------------------------------------------------------


#endif // DISTRHO_MUTEX_HPP_INCLUDED
