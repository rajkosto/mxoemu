/*
 * InternalMutex.cpp
 *
 *  Created on: Aug 8, 2009
 *      Author: rajkosto
 */

#include "../Common.h"
#include "NativeMutex.h"
#if PLATFORM == PLATFORM_WIN32

/* Windows Critical Section Implementation */
NativeMutex::NativeMutex() { InitializeCriticalSection(&cs); }
NativeMutex::~NativeMutex() { DeleteCriticalSection(&cs); }

#else

/* this is done slightly differently on bsd-variants */
#if defined(__FreeBSD__) ||  defined(__APPLE_CC__) || defined(__OpenBSD__) || defined(__CYGWIN__)
#define recursive_mutex_flag PTHREAD_MUTEX_RECURSIVE
#else
#define recursive_mutex_flag PTHREAD_MUTEX_RECURSIVE_NP
#endif

/* Linux mutex implementation */
bool NativeMutex::attr_initalized = false;
pthread_mutexattr_t NativeMutex::attr;

NativeMutex::NativeMutex()
{
	if(!attr_initalized)
	{
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, recursive_mutex_flag);
		attr_initalized = true;
	}

	pthread_mutex_init(&mutex, &attr);
}

NativeMutex::~NativeMutex() { pthread_mutex_destroy(&mutex); }

#endif

