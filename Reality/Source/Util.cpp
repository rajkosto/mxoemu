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

#include "Common.h"
#include "Util.h"
#include <cstdlib>
#include "ByteBuffer.h"
using namespace std;

vector<string> StrSplit(const string &src, const string &sep)
{
	vector<string> r;
	string s;
	for (string::const_iterator i = src.begin(); i != src.end(); i++)
    {
		if (sep.find(*i) != string::npos)
        {
            if (s.length()) r.push_back(s);
            s = "";
        }
        else
        {
            s += *i;
        }
    }
    if (s.length()) r.push_back(s);
    return r;
}

int random(int low,int high)
{
	return rand() % (high - low + 1) + low;
}

string Bin2Hex(const byte *data,size_t count,uint32 flags)
{
	if (data != NULL && count > 0)
	{
		ostringstream myStream;

		//if no spaces, then only one 0x at the start
		if ( (flags & BIN2HEX_ZEROES) && !(flags & BIN2HEX_SPACES) )
			myStream << "0x";	
			
		for (unsigned int j = 0;j < count;j++)
		{
			byte n = data[j];

			//if spaces, then 0x before every byte
			if ( (flags & BIN2HEX_ZEROES) && (flags & BIN2HEX_SPACES) )
				myStream << "0x";

			if (n <= 15)
				myStream << "0";
			myStream << hex << (int)n;

			//if no zeroes, just use space to separate
			if (flags & BIN2HEX_SPACES && !(flags & BIN2HEX_ZEROES) )
				myStream << " ";
			// if zeroes, use , to separate
			if (flags & BIN2HEX_SPACES && (flags & BIN2HEX_ZEROES) )
				myStream << ",";
		}
		myStream.flush();

		//remove the trailing space/,
		if (flags & BIN2HEX_SPACES)
			return myStream.str().substr(0,myStream.str().length()-1);
		else
			return myStream.str();
	}

	return string();
}

string Bin2Hex(const char *data,size_t count,uint32 flags)
{
	return Bin2Hex((const byte*)data,count,flags);
}

string Bin2Hex(const ByteBuffer &sourceBuf,uint32 flags)
{
	return Bin2Hex((const byte*)sourceBuf.contents(),sourceBuf.size(),flags);
}
string Bin2Hex(const string &sourceStr,uint32 flags)
{
	return Bin2Hex(sourceStr.data(),sourceStr.size(),flags);
}

string XOR(string value,string key)
{
	string retval(value);

    size_t klen=key.length();
    size_t vlen=value.length();
    size_t k=0;
    size_t v=0;
    
    for(v=0;v<vlen;v++)
    {
        retval[v]=value[v]^key[k];
        k=(++k<klen?k:0);
    }
    
    return retval;
}

string ConvertToLower(string input,size_t position)
{
	string output = input;

	for (int j=0; (unsigned int)j<position; ++j)
	{
		output[j]=tolower(input[j]);
	}
	return output;
}

void SetThreadName(const char* format, ...)
{
	// This isn't supported on nix?
	va_list ap;
	va_start(ap, format);

#if PLATFORM == PLATFORM_WIN32 && COMPILER == COMPILER_MICROSOFT

	char thread_name[200];
	vsnprintf(thread_name, 200, format, ap);

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags = 0;
	info.szName = thread_name;

	__try
	{
#ifdef _WIN64
		RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info);
#else
		RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info);
#endif
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{

	}

#endif

	va_end(ap);
}

time_t convTimePeriod ( uint32 dLength, char dType )
{
	time_t rawtime = 0;
	if (dLength == 0)
		return rawtime;
	struct tm * ti = localtime( &rawtime );
	switch(dType)
	{
	case 'h':		// hours
		ti->tm_hour += dLength;
		break;
	case 'd':		// days
		ti->tm_mday += dLength;
		break;
	case 'w':		// weeks
		ti->tm_mday += 7 * dLength;
		break;
	case 'm':		// months
		ti->tm_mon += dLength;
		break;
	case 'y':		// years
		// are leap years considered ? do we care ?
		ti->tm_year += dLength;
		break;
	default:		// minutes
		ti->tm_min += dLength;
		break;
	}
	return mktime(ti);
}
