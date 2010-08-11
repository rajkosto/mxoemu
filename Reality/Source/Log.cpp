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
		cout << outputMe << std::endl;
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
			LogFile << outputMe << std::endl;
			fileMutex.Release();
		}
	}
}

void Log::Output( LogLevel level,const string &str )
{
	OutputConsole(level,str);
	OutputFile(level,str);
}

void Log::Critical( boost::format &fmt )
{
	int ConsoleLogLevel = sConfig.GetIntDefault("Log.ConsoleLogLevel",LOGLEVEL_INFO);
	int FileLogLevel = sConfig.GetIntDefault("Log.FileLogLevel",LOGLEVEL_WARNING);

	if (ConsoleLogLevel >= LOGLEVEL_CRITICAL || FileLogLevel >= LOGLEVEL_CRITICAL)
	{
		Critical(fmt.str());
	}
}

void Log::Critical( string fmt )
{
	Output(LOGLEVEL_CRITICAL,fmt);
}

void Log::Error( boost::format &fmt )
{
	int ConsoleLogLevel = sConfig.GetIntDefault("Log.ConsoleLogLevel",LOGLEVEL_INFO);
	int FileLogLevel = sConfig.GetIntDefault("Log.FileLogLevel",LOGLEVEL_WARNING);

	if (ConsoleLogLevel >= LOGLEVEL_ERROR || FileLogLevel >= LOGLEVEL_ERROR)
	{
		Error(fmt.str());
	}
}

void Log::Error( string fmt )
{
	Output(LOGLEVEL_ERROR,fmt);
}

void Log::Warning( boost::format &fmt )
{
	int ConsoleLogLevel = sConfig.GetIntDefault("Log.ConsoleLogLevel",LOGLEVEL_INFO);
	int FileLogLevel = sConfig.GetIntDefault("Log.FileLogLevel",LOGLEVEL_WARNING);

	if (ConsoleLogLevel >= LOGLEVEL_WARNING || FileLogLevel >= LOGLEVEL_WARNING)
	{
		Warning(fmt.str());
	}
}

void Log::Warning( string fmt )
{
	Output(LOGLEVEL_WARNING,fmt);
}

void Log::Info( boost::format &fmt )
{
	int ConsoleLogLevel = sConfig.GetIntDefault("Log.ConsoleLogLevel",LOGLEVEL_INFO);
	int FileLogLevel = sConfig.GetIntDefault("Log.FileLogLevel",LOGLEVEL_WARNING);

	if (ConsoleLogLevel >= LOGLEVEL_INFO || FileLogLevel >= LOGLEVEL_INFO)
	{
		Info(fmt.str());
	}
}

void Log::Info( string fmt )
{
	Output(LOGLEVEL_INFO,fmt);
}

void Log::Debug( boost::format &fmt )
{
	int ConsoleLogLevel = sConfig.GetIntDefault("Log.ConsoleLogLevel",LOGLEVEL_INFO);
	int FileLogLevel = sConfig.GetIntDefault("Log.FileLogLevel",LOGLEVEL_WARNING);

	if (ConsoleLogLevel >= LOGLEVEL_DEBUG || FileLogLevel >= LOGLEVEL_DEBUG)
	{
		Debug(fmt.str());
	}
}

void Log::Debug( string fmt )
{
	Output(LOGLEVEL_DEBUG,fmt);
}

