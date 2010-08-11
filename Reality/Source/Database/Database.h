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

#ifndef MXOSIM_DATABASE_H
#define MXOSIM_DATABASE_H

#include <string>
#include "../Threading/Queue.h"
#include "../CallBack.h"
#include "../Threading/ThreadStarter.h"
#include "Field.h"
#include <mysql/mysql.h>

class QueryResult;
class QueryThread;
class Database;

struct DatabaseConnection
{
	DatabaseConnection(MYSQL *rawPtr):conn(rawPtr) {}
	~DatabaseConnection() {}
	FastMutex Busy;
	MYSQL *conn;
};

struct AsyncQueryResult
{
	QueryResult * result;
	char * query;
};

class AsyncQuery
{
	friend class Database;
	SQLCallbackBase * func;
	vector<AsyncQueryResult> queries;
	Database * db;
public:
	AsyncQuery(SQLCallbackBase * f) : func(f) {}
	~AsyncQuery();
	void AddQuery(string fmt);
	void AddQuery(format &fmt) { AddQuery(fmt.str()); }
	void Perform();
	inline void SetDB(Database * dbb) { db = dbb; }
};

class QueryBuffer
{
	deque<string> queries;
public:
	friend class Database;
	void AddQuery(string fmt);
	void AddQuery(format &fmt) { AddQuery(fmt.str()); }
};

class Database : public ThreadContext
{
	friend class QueryThread;
	friend class AsyncQuery;

public:
	Database();
	virtual ~Database();

	/************************************************************************/
	/* Thread Stuff                                                         */
	/************************************************************************/
	bool run();
	bool ThreadRunning;

	/************************************************************************/
	/* Virtual Functions                                                    */
	/************************************************************************/
	bool Initialize(const char* Hostname, unsigned int port,
		const char* Username, const char* Password, const char* DatabaseName,
		uint32 ConnectionCount, uint32 BufferSize);

	void Shutdown();

	QueryResult* Query(string QueryString);
	QueryResult* Query(format &fmt) { return Query(fmt.str()); }
	QueryResult * FQuery( string QueryString, DatabaseConnection &con);
	void FWaitExecute( string QueryString, DatabaseConnection &con);
	bool WaitExecute( string QueryString);//Wait For Request Completion
	bool WaitExecute(format &fmt) { return WaitExecute(fmt.str()); }
	bool Execute( string QueryString);
	bool Execute(format &fmt) { return Execute(fmt.str()); }

	inline const string& GetHostName() { return mHostname; }
	inline const string& GetDatabaseName() { return mDatabaseName; }
	inline const uint32 GetQueueSize() { return queries_queue.get_size(); }

	string EscapeString(string Escape);
	void EscapeLongString(const char * str, uint32 len, stringstream& out);
	string EscapeString(const char * esc, DatabaseConnection *con);

	void QueueAsyncQuery(AsyncQuery * query);
	void EndThreads();

	void thread_proc_query();
	void FreeQueryResult(QueryResult * p);

	DatabaseConnection &GetFreeConnection();

	void PerformQueryBuffer(QueryBuffer * b);
	void PerformQueryBuffer(QueryBuffer * b, DatabaseConnection &ccon);
	void AddQueryBuffer(QueryBuffer * b);

	static void CleanupLibs();
	static Database *Create();

	/* database is killed off manually. */
	void OnShutdown() {}

protected:

	// actual query function
	bool _SendQuery(DatabaseConnection &con, const char* Sql, bool Self);
	QueryResult * _StoreQueryResult(DatabaseConnection &con);
	bool _HandleError(DatabaseConnection &conn, uint32 ErrorNumber);
	bool _Reconnect(DatabaseConnection &conn);

	////////////////////////////////
	FQueue<QueryBuffer*> query_buffer;

	////////////////////////////////
	FQueue<string*> queries_queue;
	typedef vector<DatabaseConnection> connectionsList;
	connectionsList m_connections;

	uint32 _counter;
	///////////////////////////////

	// For reconnecting a broken connection
	string mHostname;
	string mUsername;
	string mPassword;
	string mDatabaseName;
	uint32 mPort;

	QueryThread * qt;
};

class QueryResult
{
public:
	QueryResult(MYSQL_RES *res, uint32 fields, uint32 rows);
	~QueryResult();

	bool NextRow();
	void Delete() { delete this; }

	inline Field* Fetch() { return mCurrentRow; }
	inline uint32 GetFieldCount() const { return mFieldCount; }
	inline uint32 GetRowCount() const { return mRowCount; }

protected:
	uint32 mFieldCount;
	uint32 mRowCount;
	Field *mCurrentRow;
	MYSQL_RES *mResult;
};

class QueryThread : public ThreadContext
{
	friend class Database;
	Database * db;
public:
	QueryThread(Database * d) : ThreadContext(), db(d) {}
	~QueryThread();
	bool run();
};

#endif
