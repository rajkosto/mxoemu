// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
// ***************************************************************************

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

