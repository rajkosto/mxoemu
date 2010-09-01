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

#ifndef MXOSIM_COMMON_H
#define MXOSIM_COMMON_H

// current platform and compiler
#define PLATFORM_WIN32 0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2

#define UNIX_FLAVOUR_LINUX 1
#define UNIX_FLAVOUR_BSD 2
#define UNIX_FLAVOUR_OTHER 3
#define UNIX_FLAVOUR_OSX 4

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define PLATFORM PLATFORM_WIN32
#elif defined( __INTEL_COMPILER )
#  define PLATFORM PLATFORM_INTEL
#elif defined( __APPLE_CC__ )
#  define PLATFORM PLATFORM_APPLE
#else
#  define PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU	   1
#define COMPILER_BORLAND   2
#define COMPILER_INTEL     3

#ifdef _MSC_VER
#  define COMPILER COMPILER_MICROSOFT
#elif defined( __INTEL_COMPILER )
#  define COMPILER COMPILER_INTEL
#elif defined( __BORLANDC__ )
#  define COMPILER COMPILER_BORLAND
#elif defined( __GNUC__ )
#  define COMPILER COMPILER_GNU
#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

#if (PLATFORM == PLATFORM_WIN32) && (COMPILER == COMPILER_MICROSOFT)
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1
#pragma warning(disable:4251)
#endif

/* Use correct types for x64 platforms, too */
#if COMPILER != COMPILER_GNU
	typedef signed __int64 int64;
	typedef signed __int32 int32;
	typedef signed __int16 int16;
	typedef signed __int8 int8;

	typedef unsigned __int64 uint64;
	typedef unsigned __int32 uint32;
	typedef unsigned __int16 uint16;
	typedef unsigned __int8 uint8;
#else
#include <stdint.h>
	typedef int64_t int64;
	typedef int32_t int32;
	typedef int16_t int16;
	typedef int8_t int8;
	typedef uint64_t uint64;
	typedef uint32_t uint32;
	typedef uint16_t uint16;
	typedef uint8_t uint8;
#endif

#if PLATFORM != PLATFORM_WIN32
	typedef uint32_t DWORD;
#endif

typedef unsigned char byte;

#if COMPILER == COMPILER_MICROSOFT
#ifndef snprintf
#define snprintf sprintf_s
#endif
#define atoll __atoi64
#else
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

// Short for unsigned long
typedef unsigned long ulong;
// Short for 'unsigned int'
typedef unsigned int uint;
// Short for 'unsigned short'
typedef unsigned short ushort;
// Short for 'unsigned char'
typedef unsigned char uchar;

// This is an integer type that has the same size as pointers
typedef long intptr;
typedef unsigned long uintptr;

#if defined (__GNUC__) && (defined (CPU_x86) || defined (CPU_x86_64))
// Optimized routines for x86{_64}
static inline const uint16 swap16 (const uint16 x)
{
	uint16 ret;
	__asm ("xchgb %%al,%%ah" : "=a" (ret) : "a" (x));
	return ret;
}
static inline const uint32 swap32 (const uint32 x)
{
	uint32 ret;
	__asm ("bswapl %%eax" : "=a" (ret) : "a" (x));
	return ret;
}
#if defined (CPU_x86)
static inline const uint64 swap64 (const uint64 x)
{
	uint64 ret;
	__asm ("bswapl %%eax\nbswapl %%edx\nxchgl %%eax,%%edx" : "=A" (ret) : "A" (x));
	return ret;
}
#else
static inline const uint64 swap64 (const uint64 x)
{
	uint64 ret;
	__asm ("bswapq %%rax" : "=A" (ret) : "A" (x));
	return ret;
}
#endif
static inline void xchg32 (void *a, void *b)
{
	__asm ("movl (%0),%%eax\nxchgl (%1),%%eax\nmovl %%eax,(%0)"
		   :: "r" (a), "r" (b) : "eax");
}
#else
static inline const uint16 swap16 (const uint16 x)
{ return (x >> 8) | (x << 8); }
static inline const uint32 swap32 (const uint32 x)
{ return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24); }
static inline const uint64 swap64 (const uint64 x)
{ return ((x >> 56) /*& 0x00000000000000ffULL*/) | ((x >> 40) & 0x000000000000ff00ULL) |
         ((x >> 24) & 0x0000000000ff0000ULL) | ((x >> 8 ) & 0x00000000ff000000ULL) |
         ((x << 8 ) & 0x000000ff00000000ULL) | ((x << 24) & 0x0000ff0000000000ULL) |
	 ((x << 40) & 0x00ff000000000000ULL) | ((x << 56) /*& 0xff00000000000000ULL*/); }
static inline void xchg32 (void *a, void *b)
{
	uint32 tmp = *(uint32 *)a;
	*(uint32 *)a = *(uint32 *)b;
	*(uint32 *)b = tmp;
}
#endif

/// Given a number of bits, return how many bytes are needed to represent that.
#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

#define CALL_METHOD(object,ptrToMember)  ((object).*(ptrToMember))
#define CALL_METHOD_PTR(object,ptrToMember) ((object)->*(ptrToMember))

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <deque>
#include <queue>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <stdarg.h>
#include <signal.h>
#if PLATFORM != PLATFORM_WIN32
#include <unistd.h>
#endif
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <numeric>

using std::cout;
using std::cin;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::stringstream;
using std::ostringstream;
using std::istringstream;
using std::exception;
using std::runtime_error;
using std::invalid_argument;
using std::out_of_range;
using std::vector;
using std::deque;
using std::queue;
using std::list;
using std::map;
using std::string;

#if COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1600 && _HAS_TR1
#include <memory>
using std::tr1::shared_ptr;
using std::tr1::make_shared;
using std::tr1::dynamic_pointer_cast;
#include <unordered_map>
using std::tr1::unordered_map;
#else
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;
using boost::dynamic_pointer_cast;
#include <boost/make_shared.hpp>
using boost::make_shared;
#include <boost/unordered_map.hpp>
using boost::unordered_map;
#endif

#include <boost/scoped_ptr.hpp>
using boost::scoped_ptr;
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
#include <boost/format.hpp>
using boost::format;

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <boost/bind.hpp>
#include <boost/function.hpp>

#if PLATFORM == PLATFORM_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#endif

#if PLATFORM != PLATFORM_WIN32
inline void Sleep(int ms) { usleep(1000*ms); }
#endif

#ifdef min
#undef min
#endif
template<class A, class B>
inline A min(A a, B b) { return ((a < (A)b) ? a : (A)b); }
#ifdef max
#undef max
#endif
template<class A, class B>
inline A max(A a, B b) { return ((a > (A)b) ? a : (A)b); }

class Database;

#ifndef THIS_IS_SPARTA
extern Database* Database_Main;
#endif

#define sDatabase (*Database_Main)

#endif
