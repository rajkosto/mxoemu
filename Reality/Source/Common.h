// *************************************************************************************************
// --------------------------------------
// Copyright (C) 2006-2010 Rajko Stojadinovic
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// *************************************************************************************************

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
#define I64FMT "%016I64X"
#define I64FMTD "%I64u"
#define SI64FMTD "%I64d"
#define snprintf _snprintf
#define atoll __atoi64
#else
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%llu"
#define SI64FMTD "%lld"
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

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <deque>
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

#if COMPILER == COMPILER_INTEL
#include <ext/hash_map>
#elif COMPILER == COMPILER_GNU && __GNUC__ >= 4
#include <tr1/memory>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#elif COMPILER == COMPILER_GNU && __GNUC__ >= 3
#include <ext/hash_map>
#include <ext/hash_set>
#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1500 && _HAS_TR1   // VC9.0 SP1 and later
#include <memory>
#include <unordered_map>
#include <unordered_set>
#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1500 && !_HAS_TR1
#pragma message ("FATAL ERROR: Please install Service Pack 1 for Visual Studio 2008")
#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER < 1500
#include <boost/tr1/memory.hpp>
#include <boost/tr1/unordered_map.hpp>
#include <boost/tr1/unordered_set.hpp>
#else
#include <memory>
#include <hash_map>
#include <hash_set>
#endif

#ifdef _STLPORT_VERSION
#define HM_NAMESPACE std
using std::hash_map;
using std::hash_set;
#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1500 && _HAS_TR1
using namespace std::tr1;
using std::tr1::shared_ptr;
#undef HM_NAMESPACE
#define HM_NAMESPACE tr1
#define hash_map unordered_map
#define TRHAX 1
#elif COMPILER == COMPILER_MICROSOFT && (_MSC_VER < 1500 || !_HAS_TR1)
using namespace std::tr1;
using std::tr1::shared_ptr;
#undef HM_NAMESPACE
#define HM_NAMESPACE tr1
#define hash_map unordered_map
#define ENABLE_SHITTY_STL_HACKS 1

// hacky stuff for vc++
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#elif COMPILER == COMPILER_INTEL
#define HM_NAMESPACE std
using std::hash_map;
using std::hash_set;
#elif COMPILER == COMPILER_GNU && __GNUC__ >= 4
using namespace std::tr1;
using std::tr1::shared_ptr;
#undef HM_NAMESPACE
#define HM_NAMESPACE tr1
#define shared_ptr std::tr1::shared_ptr
#define hash_map unordered_map
#define TRHAX 1
#elif COMPILER == COMPILER_GNU && __GNUC__ >= 3
#define HM_NAMESPACE __gnu_cxx
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;

namespace __gnu_cxx
{
	template<> struct hash<unsigned long long>
	{
		size_t operator()(const unsigned long long &__x) const { return (size_t)__x; }
	};
	template<typename T> struct hash<T *>
	{
		size_t operator()(T * const &__x) const { return (size_t)__x; }
	};

};
#else
#define HM_NAMESPACE std
using std::hash_map;
#endif
#if COMPILER == COMPILER_GNU && __GNUC__ >=4 && __GNUC_MINOR__ == 1 && __GNUC_PATCHLEVEL__ == 2
//GCC I HATE YOU!
namespace std
{
	namespace tr1
	{
		template<> struct hash<long long unsigned int> : public std::unary_function<long long unsigned int, std::size_t>
		{
			std::size_t operator()(const long long unsigned int val) const
			{
				return static_cast<std::size_t>(val);
			}
		};
	}
}
#endif

namespace std
{
	namespace tr1
	{
		template<typename T> struct hash<shared_ptr<T> >
		{
			size_t operator()(shared_ptr<T> const &__x) const { return (size_t)__x.get(); }
		};
	}
}

using namespace std;

#undef FD_SETSIZE
#define FD_SETSIZE 200  // 200 per thread should be plenty :p

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
#define Sleep(ms) usleep(1000*ms)
#endif

class Database;

#ifndef THIS_IS_SPARTA
extern Database* Database_Main;
#endif

#define sDatabase (*Database_Main)

#endif
