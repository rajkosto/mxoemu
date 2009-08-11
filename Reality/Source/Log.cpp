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
#include "Singleton.h"
#include "Log.h"
#include "Config.h"

createFileSingleton( Log );

Log::Log()
{
	OpenLogFile("Reality.log");
}

Log::~Log()
{
	if (LogFile.is_open() == true)
	{
		LogFile.close();
	}
}


void Log::OpenLogFile( const char *logFileName )
{
	fileMutex.Acquire();

	if (LogFile.is_open() == true)
		LogFile.close();

	LogFile.open(logFileName,ios::out | ios::app);

	fileMutex.Release();
}

string Log::ProcessString( LogLevel level,const string &str,bool forFile)
{
	ostringstream returnings;

	//Timestamp
	time_t curr=time(0);
	tm local;
	local=*(localtime(&curr));

	ostringstream Day,Month,Year;
	ostringstream Hour,Minute,Second;

	if(local.tm_mday < 10)
		Day << "0";
	Day << local.tm_mday;

	if ((local.tm_mon + 1) < 10)
		Month << "0";
	Month << (local.tm_mon + 1);

	Year << (local.tm_year + 1900);

	if (local.tm_hour < 10)
		Hour << "0";
	Hour << local.tm_hour;

	if (local.tm_min < 10)
		Minute << "0";
	Minute << local.tm_min;

	if (local.tm_sec < 10)
		Second << "0";
	Second << local.tm_sec;

	string LogLevelStr;

	switch (level)
	{
	default:
		LogLevelStr = "UNKNOWN";
		break;
	case LOGLEVEL_CRITICAL:
		LogLevelStr = "CRITICAL";
		break;
	case LOGLEVEL_ERROR:
		LogLevelStr = "ERROR";
		break;
	case LOGLEVEL_WARNING:
		LogLevelStr = "WARNING";
		break;
	case LOGLEVEL_INFO:
		LogLevelStr = "INFO";
		break;
	case LOGLEVEL_DEBUG:
		LogLevelStr = "DEBUG";
		break;
	}

	returnings << "[";
	if (forFile == true)
	{
		returnings << Day.str() << "." << Month.str() << "." << Year.str().c_str() << " ";
	}
	returnings << Hour.str() << ":" << Minute.str() << ":" << Second.str() << "]" << " " << LogLevelStr << ": " << str;

	return returnings.str();
}

void Log::OutputConsole( LogLevel level,const string &str )
{
	int ConsoleLogLevel = sConfig.GetIntDefault("Log.ConsoleLogLevel",LOGLEVEL_INFO);
	if (ConsoleLogLevel < LOGLEVEL_CRITICAL || ConsoleLogLevel > LOGLEVEL_DEBUG)
	{
		ConsoleLogLevel = LOGLEVEL_INFO;
	}
	if (ConsoleLogLevel >= level)
	{
		string outputMe = ProcessString(level,str,false);
		printMutex.Acquire();
		cout << outputMe << endl;
		printMutex.Release();
	}
}

void Log::OutputFile( LogLevel level,const string &str )
{
	int FileLogLevel = sConfig.GetIntDefault("Log.FileLogLevel",LOGLEVEL_WARNING);
	if (FileLogLevel < LOGLEVEL_CRITICAL || FileLogLevel > LOGLEVEL_DEBUG)
	{
		FileLogLevel = LOGLEVEL_WARNING;
	}
	if (FileLogLevel >= level)
	{
		string outputMe = ProcessString(level,str,true);
		if (LogFile.is_open() == true)
		{
			fileMutex.Acquire();
			LogFile << outputMe << endl;
			fileMutex.Release();
		}
	}
}

void Log::Output( LogLevel level,const string &str )
{
	OutputConsole(level,str);
	OutputFile(level,str);
}

void Log::Critical( const char *str, ... )
{
	if( str == NULL )
		return;

	char outstring[65536];
	va_list ap;
	va_start(ap, str);
	vsprintf(outstring,str,ap);
	va_end(ap);

	Output(LOGLEVEL_CRITICAL,string(outstring));
}

void Log::Error( const char *str, ... )
{
	if( str == NULL )
		return;

	char outstring[65536];
	va_list ap;
	va_start(ap, str);
	vsprintf(outstring,str,ap);
	va_end(ap);

	Output(LOGLEVEL_ERROR,string(outstring));
}

void Log::Warning( const char *str, ... )
{
	if( str == NULL )
		return;

	char outstring[65536];
	va_list ap;
	va_start(ap, str);
	vsprintf(outstring,str,ap);
	va_end(ap);

	Output(LOGLEVEL_WARNING,string(outstring));
}

void Log::Info( const char *str, ... )
{
	if( str == NULL )
		return;

	char outstring[65536];
	va_list ap;
	va_start(ap, str);
	vsprintf(outstring,str,ap);
	va_end(ap);

	Output(LOGLEVEL_INFO,string(outstring));
}

void Log::Debug( const char *str, ... )
{
	if( str == NULL )
		return;

	char outstring[65536];
	va_list ap;
	va_start(ap, str);
	vsprintf(outstring,str,ap);
	va_end(ap);

	Output(LOGLEVEL_DEBUG,string(outstring));
}
