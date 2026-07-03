#pragma once
#include <string>
#include <sqlite3.h> 

class TGMTsqlite
{
	sqlite3 *db;
	int rc;
	bool m_isOpenDBsuccess;
public:
	TGMTsqlite();
	~TGMTsqlite();

	bool OpenDB(std::string dbName);
	bool ExecQuery(std::string sql);
};

