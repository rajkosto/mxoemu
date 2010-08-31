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

#include "Common.h"

#ifdef WIN32
class StaticTime
{
public:
	StaticTime()
	{
		timeBeginPeriod(1);
		m_timeBase = timeGetTime();
	}
	~StaticTime()
	{
		timeEndPeriod(1);
	}
	uint32 getTimeBase() { return m_timeBase; }
private:
	uint32 m_timeBase;
} staticTimeInst;

float getFloatTime()
{
	return (float)(timeGetTime() - staticTimeInst.getTimeBase()) * (1.0f / 1000.0f);
}
uint32 getMSTime() 
{
	return (timeGetTime() - staticTimeInst.getTimeBase());
}
#else
#include <sys/time.h>
#include <unistd.h>

class StaticTime
{
public:
	StaticTime()
	{
		timeval currTime;
		gettimeofday(&currTime, NULL);
		m_seconds = currTime.tv_sec;
	}
	~StaticTime()
	{
	}
	uint32 getSeconds() { return m_seconds; }
private:
	uint32 m_seconds;
} staticTimeInst;

float getFloatTime()
{
	timeval theTime;
	gettimeofday(&theTime, NULL);
	return (float)(theTime.tv_sec - staticTimeInst.getSeconds() + theTime.tv_usec * 0.000001f);
}
uint32 getMSTime()
{
	timeval theTime;
	gettimeofday(&theTime, NULL);
	return ((theTime.tv_sec - staticTimeInst.getSeconds()) * 1000) + (theTime.tv_usec / 1000);
}
#endif