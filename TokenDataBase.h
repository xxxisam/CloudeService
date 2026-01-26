#pragma once
#include "sqlite3.h"
#include <memory>
#include <string>
#include "User.hpp"
#include "DataBaseUtility.hpp"
#include "SessionManager.h"

struct TokenDataBaseTable
{
    inline static const std::string tableName = "token";
    inline static const std::string columnID = "id";
    inline static const std::string columnToken = "userToken";
    inline static const std::string columnUserId = "userId";
    inline static const std::string columnUserLogin = "userLogin";
    inline static const std::string columnExpires = "expires";
    inline static const std::string columnIsAdmin = "isAdmin";
	inline static const std::string columnIsGuest = "isGuest";
    inline static const std::string columnIssuedAt = "issuedAt";
    inline static const std::string columnIPAddress = "ipAddress";
    inline static const std::string columnUserAgent = "userAgent";
    inline static const std::string columnLastUsed = "lastUsed";
};

class TokenDataBase
{
private:
	std::unique_ptr<sqlite3, DataBaseUtility::DataBaseDeleter> m_tokenDataBase;




	//sqlRqsts
	const char* sqlGetFile = "SELECT id, name, extension, fullName, size, fileServerPath FROM files;";
	const std::string sqlSelect = "SELECT hash FROM " + TokenDataBaseTable::tableName +
		" WHERE name=? AND extension=? AND fullName=? AND size=? AND fileServerPath=?";
	const char* sqlDelete = "DELETE FROM files WHERE hash = ?;";

private:
	TokenDataBase();

	TokenDataBase(TokenDataBase& other) = delete;
	TokenDataBase& operator=(const TokenDataBase&) = delete;
	TokenDataBase(TokenDataBase&&) = delete;
	TokenDataBase& operator=(TokenDataBase&&) = delete;

public:
	static TokenDataBase& getInstance();


	//
	void start();
	sqlite3* getTokenDataBase();
	void getRequest(std::string_view request);
	void getRequest(std::string_view request, User::User& user);
	//void getRequest(std::string_view request);
	bool createTokenTable();
	DataBaseUtility::UserDBResult addTokenInTable(const SessionManager::Token& token);
	DataBaseUtility::UserDBResult verification(const User::User& user);
	SessionManager::Token getTokenByLogin(SessionManager::Token& token);
	void printTokenTable();
	bool findByTokenIfExistByLogin(const SessionManager::Token& token);
	bool isThisTokenExisted(const std::string& token);
	bool takeTokenInfo(SessionManager::Token& token);
	DataBaseUtility::UserDBResult delToken(const SessionManager::Token& token);

private:
	void openTokenSQL();
	bool isSQLTableExist(const std::string& tableName);
	//void printTokenTable();

	//public:	
		//bool checkFile(const File::FileMetaInfo& fmi);



		//operations
	bool insertTokenInfoInTable(const SessionManager::Token& token);
	bool takeTokenInfoFromTable();
	//bool findByTokenIfExist(const Token& token);
	int getIdByLogin(const SessionManager::Token& token);
	bool verifyPassword(const SessionManager::Token& token);
	//bool deleteUserInfoInTableByHash(const File::FileMetaInfo& fmi);
	//void syncFiles(nlohmann::json& json);
	bool deleteToken(const SessionManager::Token& token);
	bool getLogin(const SessionManager::Token& token);
};

