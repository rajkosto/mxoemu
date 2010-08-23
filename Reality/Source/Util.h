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

#ifndef MXOSIM_UTIL_H
#define MXOSIM_UTIL_H

#include "Common.h"

///////////////////////////////////////////////////////////////////////////////
// String Functions ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> StrSplit(const std::string &src, const std::string &sep);

int random(int low,int high);

enum
{
	BIN2HEX_ZEROES = 1,
	BIN2HEX_SPACES = 2,
	BIN2HEX_NEWLINES = 4
};

std::string Bin2Hex(const byte *data,size_t count,uint32 flags = BIN2HEX_SPACES | BIN2HEX_NEWLINES);
std::string Bin2Hex(const char *data,size_t count,uint32 flags = BIN2HEX_SPACES | BIN2HEX_NEWLINES);
std::string Bin2Hex(const class ByteBuffer &sourceBuf,uint32 flags = BIN2HEX_SPACES | BIN2HEX_NEWLINES);
std::string Bin2Hex(const std::string &sourceStr,uint32 flags = BIN2HEX_SPACES | BIN2HEX_NEWLINES);

std::string Bin2Str(const byte *data,size_t count);
std::string Bin2Str(const char *data,size_t count);
std::string Bin2Str(const class ByteBuffer &sourceBuf);
std::string Bin2Str(const std::string &sourceStr);

std::string ClientVersionString(uint32 theVersion);


std::string ConvertToLower(std::string input,size_t position);
std::string XOR(std::string value,std::string key);

// This HAS to be called outside the threads __try / __except block!
void SetThreadName(const char* fmt);

time_t convTimePeriod ( uint32 dLength, char dType);

#if PLATFORM == PLATFORM_WIN32
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;
#endif

#endif