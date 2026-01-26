#pragma once
#include "sqlite3.h"
#include "File.hpp"
#include <memory>
#include <string>
#include "DataBaseUtility.hpp"

struct HashDataBaseTable
{
	inline static const std::string tableName = "hashs";
	inline static const std::string columnHashID = "id";
	inline static const std::string columnHash = "hash";
	inline static const std::string columnIsEmpty = "isEmpty";
	inline static const std::string columnSize = "size";
	inline static const std::string columnServerPath = "fileServerPath";
	inline static const std::string columnIsFileComplete = "isFileComplete";
	inline static const std::string columnRefCount = "fileReferenceCount";
};

class HashDataBase
{
private:
	std::unique_ptr<sqlite3, DataBaseUtility::DataBaseDeleter> m_hashDataBase;

	//HashTable
	const std::string sqlHashTable = "CREATE TABLE IF NOT EXISTS " + HashDataBaseTable::tableName + "("
		+ HashDataBaseTable::columnHashID + " INTEGER PRIMARY KEY,"
		+ HashDataBaseTable::columnHash + " TEXT, "
		+ HashDataBaseTable::columnSize + " INTEGER, "
		+ HashDataBaseTable::columnIsEmpty + " INTEGER,"
		+ HashDataBaseTable::columnServerPath + " TEXT, "
		+ HashDataBaseTable::columnIsFileComplete + " INTEGER, "
		+ HashDataBaseTable::columnRefCount + " INTEGER DEFAULT 0"
		+ ");";


	const char* sqlGetHash = "SELECT id, name, extension, fullName, size, fileServerPath FROM files;";
	const std::string sqlSelectHash = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";

private:
	HashDataBase();

	HashDataBase(HashDataBase& other) = delete;
	HashDataBase& operator=(const HashDataBase&) = delete;
	HashDataBase(HashDataBase&&) = delete;
	HashDataBase& operator=(HashDataBase&&) = delete;

public:
	static HashDataBase& getInstance();


	//
	void start();
	sqlite3* getHashDataBase();
	void getRequest(std::string_view request);
	void getRequest(std::string_view request, File::FileMetaInfo& fmi);
	void getRequest(std::string_view request, nlohmann::json& json);
	bool createHashTable();
	DataBaseUtility::UserDBResult addHash(File::FileMetaInfo& fmi);
	bool getServerPath(File::FileMetaInfo& fmi);
	int getHashID(const File::FileMetaInfo& fmi);
	bool minusRef(const File::FileMetaInfo& fmi);
	bool isRefCountZero(const File::FileMetaInfo& fmi);

private:
	void openHashSQL();
	bool isSQLTableExist(const std::string& tableName);
	void printHashTable();


	//public:	
		//bool checkFile(const File::FileMetaInfo& fmi);


		//operations
	bool insertHashInfoInTable(File::FileMetaInfo& fmi);
	bool checkHash();
	bool takeFileInfoFromTable(const File::FileMetaInfo& fmi);
	bool isThisHashExisted(File::FileMetaInfo& fmi);
	bool checkHashExistence(File::FileMetaInfo& fmi);
	bool deleteFileInfoInTableByHash(const File::FileMetaInfo& fmi);
	void syncFiles(nlohmann::json& json);
	bool addRef(const File::FileMetaInfo& fmi);
};

