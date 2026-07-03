#include "TGMTsqlite.h"
#include "TGMTdebugger.h"

static int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName) {
	for (int i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TGMTsqlite::TGMTsqlite()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TGMTsqlite::~TGMTsqlite()
{
	sqlite3_close(db);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTsqlite::OpenDB(std::string dbName)
{
	rc = sqlite3_open(dbName.c_str(), &db);
	if (rc != SQLITE_OK)
	{
		PrintError("Can't open database: %s\n", sqlite3_errmsg(db));
		m_isOpenDBsuccess = false;
	}
	else 
	{
		PrintSuccess("Opened database successfully\n");
		m_isOpenDBsuccess = true;
	}

	return m_isOpenDBsuccess;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTsqlite::ExecQuery(std::string sql)
{
	if (!m_isOpenDBsuccess)
	{
		PrintError("DB can not open, so that can not exec query");
		return false;
	}

	char *zErrMsg = 0;
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql.c_str(), sqlite_callback, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		PrintError("SQLite error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	return true;
}