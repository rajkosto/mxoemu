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

#ifndef MXOSIM_LOCKED_QUEUE_H
#define MXOSIM_LOCKED_QUEUE_H

#include "NativeMutex.h"
#include <deque>

template<class TYPE>
class LockedQueue
{
public:
	~LockedQueue()
	{

	}

	inline void add(const TYPE& element)
	{
		mutex.Acquire();
		queue.push_back(element);
		mutex.Release();
	}

	inline TYPE next()
	{
		mutex.Acquire();
		assert(queue.size() > 0);
		TYPE t = queue.front();
		queue.pop_front();
		mutex.Release();
		return t;
	}

	inline size_t size()
	{
		mutex.Acquire();
		size_t c = queue.size();
		mutex.Release();
		return c;
	}

	inline bool empty()
	{	// return true only if sequence is empty
		mutex.Acquire();
		bool isEmpty = queue.empty();
		mutex.Release();
		return isEmpty;
	}

	inline TYPE get_first_element()
	{
		mutex.Acquire();
		TYPE t; 
		if(queue.size() == 0)
			t = reinterpret_cast<TYPE>(0);
		else
			t = queue.front();
		mutex.Release();
		return t;			
	}

	inline void pop()
	{
		mutex.Acquire();
		ASSERT(queue.size() > 0);
		queue.pop_front();
		mutex.Release();
	}

protected:
	std::deque<TYPE> queue;
	NativeMutex mutex;
};

#endif
