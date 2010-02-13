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

#ifndef MXOSIM_FIELD_H
#define MXOSIM_FIELD_H

#if PLATFORM != PLATFORM_WIN32
#include <stdio.h>
#include <stdlib.h>
#endif

class Field
{
public:

	inline void SetValue(char* value) { mValue = value; }

	inline const char *GetString() { return mValue; }
	inline float GetFloat() { return mValue ? static_cast<float>(atof(mValue)) : 0; }
	inline double GetDouble() { return atof(mValue); }
	inline bool GetBool() { return mValue ? atoi(mValue) > 0 : false; }
	inline uint8 GetUInt8() { return mValue ? static_cast<uint8>(atol(mValue)) : 0; }
	inline int8 GetInt8() { return mValue ? static_cast<int8>(atol(mValue)) : 0; }
	inline uint16 GetUInt16() { return mValue ? static_cast<uint16>(atol(mValue)) : 0; }
	inline uint32 GetUInt32() { return mValue ? static_cast<uint32>(atol(mValue)) : 0; }
	inline uint32 GetInt32() { return mValue ? static_cast<int32>(atol(mValue)) : 0; }
	uint64 GetUInt64() 
	{
		if(mValue)
		{
			uint64 value;
#if !defined(WIN32) && defined(X64)
			sscanf(mValue,I64FMTD,(long long unsigned int*)&value);
#else
			sscanf(mValue,I64FMTD,&value);
#endif
			return value;
		}
		else
			return 0;
	}

private:
		char *mValue;
};

#endif
