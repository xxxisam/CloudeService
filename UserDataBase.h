#pragma once
#include "sqlite3.h"
#include <memory>
#include <string>
#include "User.hpp"
#include "DataBaseUtility.hpp"

struct UserDataBaseDeleter
{
	void operator()(sqlite3* db) const
	{
		if (db)
		{
			sqlite3_close(db);
		}
	}
};

struct UserDataBaseTable
{
	inline static const std::string tableName = "user";
	inline static const std::string columnID = "id";
	inline static const std::string columnUserName = "userName";
	inline static const std::string columnUserID = "userID";
	inline static const std::string columnUserLogin = "userLogin";
	inline static const std::string columnUserPassword = "userPassword";
	inline static const std::string columnIsAdmin = "isAdmin";
	inline static const std::string columnMaxStorageSize = "maxStorageSize";
};

class UserDataBase
{
private:
	std::unique_ptr<sqlite3, UserDataBaseDeleter> m_userDataBase;




	//sqlRqsts
	const char* sqlGetFile = "SELECT id, name, extension, fullName, size, fileServerPath FROM files;";
	const std::string sqlSelect = "SELECT hash FROM " + UserDataBaseTable::tableName +
		" WHERE name=? AND extension=? AND fullName=? AND size=? AND fileServerPath=?";
	const char* sqlDelete = "DELETE FROM files WHERE hash = ?;";

private:
	UserDataBase();

	UserDataBase(UserDataBase& other) = delete;
	UserDataBase& operator=(const UserDataBase&) = delete;
	UserDataBase(UserDataBase&&) = delete;
	UserDataBase& operator=(UserDataBase&&) = delete;

public:
	static UserDataBase& getInstance();


	//
	void start();
	sqlite3* getUserDataBase();
	void getRequest(std::string_view request);
	void getRequest(std::string_view request, User::User& user);
	//void getRequest(std::string_view request);
	bool createUsersTable();
	DataBaseUtility::UserDBResult addUserInTable(const User::User& user);
	DataBaseUtility::UserDBResult verification(const User::User& user);
	std::string getLogin(const User::User& user);

private:
	void openUserSQL();
	bool isSQLTableExist(const std::string& tableName);
	void printUsersTable();

	//public:	
		//bool checkFile(const File::FileMetaInfo& fmi);



		//operations
	bool insertUserInfoInTable(const User::User& user);
	bool takeUserInfoFromTable();
	bool isThisUserExisted();
	bool findByLoginIfExist(const User::User& user);
	int getIdByLogin(const User::User& user);
	bool verifyPassword(const User::User& user);
	//bool deleteUserInfoInTableByHash(const File::FileMetaInfo& fmi);
	//void syncFiles(nlohmann::json& json);
};

