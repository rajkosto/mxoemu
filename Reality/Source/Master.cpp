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

#define THIS_IS_SPARTA

#include "Common.h"
#include "Master.h"
#include "Log.h"
#include "Config.h"
#include "MersenneTwister.h"
#include "Threading/Threading.h"
#include "Database/Database.h"
#include "AuthRunnable.h"
#include "MarginRunnable.h"
#include "GameRunnable.h"
#include "ConsoleThread.h"

createFileSingleton( Master );

volatile bool Master::m_stopEvent = false;

void Master::_OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
            Master::m_stopEvent = true;
            break;
        case SIGTERM:
            Master::m_stopEvent = true;
            break;
        case SIGABRT:
            Master::m_stopEvent = true;
            break;
#if PLATFORM == PLATFORM_WIN32
        case SIGBREAK:
            Master::m_stopEvent = true;
            break;
#endif
    }

    signal(s, _OnSignal);
}

Master::Master()
{
}


Master::~Master()
{
}

#define REALITY_CONFIG "Reality.conf"

bool Master::Run()
{
	if (!sConfig.SetSource(REALITY_CONFIG))
	{
		CRITICAL_LOG(format("Could not find configuration file %1%.") % REALITY_CONFIG);
		exit(0);
	}

	INFO_LOG(format("Reality v0.01 Alpha %1% bit version started") % (sizeof(int*) * 8));

	if( !_StartDB() )
	{
		CRITICAL_LOG("Error starting database!");

		Database::CleanupLibs();
		return false;
	}

	DEBUG_LOG("Initializing random number generators...");
	uint32 seed = uint32(time(NULL));
	new MTRand(seed);
	srand(seed);

	_HookSignals();

	//init thread manager
	ThreadPool.Startup();
	
	//start server threads
	AuthRunnable *authRun = new AuthRunnable();
	ThreadPool.ExecuteTask(authRun);
	MarginRunnable *marginRun = new MarginRunnable();
	ThreadPool.ExecuteTask(marginRun);
	GameRunnable *gameRun = new GameRunnable();
	ThreadPool.ExecuteTask(gameRun);

	//spawn console thread
	ConsoleThread *consoleRun = new ConsoleThread();
	ThreadPool.ExecuteTask(consoleRun);

	while (!Master::m_stopEvent)
	{
		Sleep(100);
	}

	authRun->Terminate();
	marginRun->Terminate();
	gameRun->Terminate();

	consoleRun->Terminate();
	
	DEBUG_LOG("Exiting...");
	ThreadPool.ShowStats();

	_UnhookSignals();
	_StopDB();

	return 0;
}

// Database defines.
Database* Database_Main;

bool Master::_StartDB()
{
	Database_Main = Database::Create();

	std::string hostname, username, password, database;
	int port = 0;

	bool result = sConfig.GetString("Database.Username", &username);
	sConfig.GetString("Database.Password", &password);
	result = !result ? result : sConfig.GetString("Database.Hostname", &hostname);
	result = !result ? result : sConfig.GetString("Database.Name", &database);
	result = !result ? result : sConfig.GetInt("Database.Port", &port);

 	if(result == false)
	{
		CRITICAL_LOG("sql: One or more parameters were missing from Database directive.");
		return false;
	}

	// Initialize it
	if(!sDatabase.Initialize(hostname.c_str(), (unsigned int)port, username.c_str(),
		password.c_str(), database.c_str(), sConfig.GetIntDefault("Database.ConnectionCount", 5),
		16384))
	{
		CRITICAL_LOG("sql: Main database initialization failed. Exiting.");
		return false;
	}

    return true;
}


void Master::_StopDB()
{
	sDatabase.EndThreads();
	sDatabase.Shutdown();
}

void Master::_HookSignals()
{
    signal(SIGINT, _OnSignal);
    signal(SIGTERM, _OnSignal);
    signal(SIGABRT, _OnSignal);
#if PLATFORM == PLATFORM_WIN32
    signal(SIGBREAK, _OnSignal);
#endif
}


void Master::_UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    signal(SIGABRT, 0);
#if PLATFORM == PLATFORM_WIN32
    signal(SIGBREAK, 0);
#endif

}

#ifdef WIN32

NativeMutex m_crashedMutex;

// Crash Handler
void OnCrash( bool Terminate )
{
	ERROR_LOG( "Advanced crash handler initialized." );

	if( !m_crashedMutex.AttemptAcquire() )
		TerminateThread( GetCurrentThread(), 0 );

	try
	{
		ERROR_LOG( "Waiting for all database queries to finish..." );
		sDatabase.EndThreads();
	}
	catch(...)
	{
		ERROR_LOG( "Threw an exception while attempting to save all data." );
	}

	ERROR_LOG( "Closing." );

	// Terminate Entire Application
	if( Terminate )
	{
		HANDLE pH = OpenProcess( PROCESS_TERMINATE, TRUE, GetCurrentProcessId() );
		TerminateProcess( pH, 1 );
		CloseHandle( pH );
	}
}

#endif
