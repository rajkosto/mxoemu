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

#ifndef MXOEMU_THREADSTARTER_H
#define MXOEMU_THREADSTARTER_H

class ThreadContext
{
public:
	ThreadContext() { m_threadRunning = true; }
	virtual ~ThreadContext() {}
	virtual bool run() = 0;

#if PLATFORM == PLATFORM_WIN32
	HANDLE THREAD_HANDLE;
#else
	pthread_t THREAD_HANDLE;
#endif

	inline void Terminate() { m_threadRunning = false; }
	virtual void OnShutdown() { Terminate(); }

protected:
	bool m_threadRunning;
};

#endif

