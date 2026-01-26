#pragma once

namespace DataBaseUtility
{
	enum class UserDBResult
	{
		OK,
		USER_ALREADY_EXIST,
		USER_DOESNT_EXIST,
		WRONG_PASSWORD,
		HASH_ALREADY_EXIST,
		HASH_DOESNT_EXIST,
		INSERT_OK,
		INSERT_ERROR,
		DB_ERROR,
		TOKEN_ALREADY_EXIST,
		TOKEN_DOESNT_EXIST
	};

	struct DataBaseDeleter
	{
		void operator()(sqlite3* db) const
		{
			if (db)
			{
				sqlite3_close(db);
			}
		}
	};
}