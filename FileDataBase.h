#pragma once
#include "sqlite3.h"
#include "File.hpp"
#include <memory>
#include <string>

struct FileDataBaseDeleter
{
	void operator()(sqlite3* db) const
	{
		if (db)
		{
			sqlite3_close(db);
		}
	}
};

struct FileDataBaseTable
{
	inline static const std::string tableName = "files";
	inline static const std::string columnId = "id";
	inline static const std::string columnName = "name";
	inline static const std::string columnExtension = "extension";
	inline static const std::string columnFullName = "fullName";
	inline static const std::string columnSize = "size";
	inline static const std::string columnHash = "hash";
	inline static const std::string columnIsEmpty = "isEmpty";
	inline static const std::string columnIsFileComplete = "isFileComplete";
	inline static const std::string columnFileServerPath = "fileServerPath";
};

class FileDataBase
{
private:
	std::unique_ptr<sqlite3, FileDataBaseDeleter> m_fileDataBase;




	//sqlRqsts
	const std::string sqlFilesTable = "CREATE TABLE IF NOT EXISTS " + FileDataBaseTable::tableName + "("
		+ FileDataBaseTable::columnId + " INTEGER PRIMARY KEY AUTOINCREMENT,"
		+ FileDataBaseTable::columnName + " TEXT NOT NULL,"
		+ FileDataBaseTable::columnFullName + " TEXT NOT NULL, "
		+ FileDataBaseTable::columnExtension + " TEXT, "
		+ FileDataBaseTable::columnSize + " INTEGER, "
		+ FileDataBaseTable::columnHash + " TEXT, "
		+ FileDataBaseTable::columnIsEmpty + " INTEGER,"
		+ FileDataBaseTable::columnIsFileComplete + " INTEGER, "
		+ FileDataBaseTable::columnFileServerPath + " TEXT "
		+ ");";


	const std::string sqlInsertFile = "INSERT INTO " + FileDataBaseTable::tableName
		+ "(name, fullName, extension, size, hash, isEmpty, isFileComplete, fileServerPath)"
		+ "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
	const char* sqlFindFileByHash = "SELECT id, name, fullName, fileServerPath From files WHERE hash = ?";
	const char* sqlGetFile = "SELECT id, name, extension, fullName, size, fileServerPath FROM files;";
	const std::string sqlSelect = "SELECT hash FROM " + FileDataBaseTable::tableName +
		" WHERE name=? AND extension=? AND fullName=? AND size=? AND fileServerPath=?";
	const char* sqlDelete = "DELETE FROM files WHERE hash = ?;";
	
private:
	FileDataBase();

	FileDataBase(FileDataBase &other) = delete;
	FileDataBase& operator=(const FileDataBase &) = delete;
	FileDataBase(FileDataBase&&) = delete;
	FileDataBase& operator=(FileDataBase&&) = delete;

public:
	static FileDataBase& getInstance();


	//
	void start();
	sqlite3* getDataBase();
	void getRequest(std::string_view request);
	void getRequest(std::string_view request, File::FileMetaInfo& fmi);
	void getRequest(std::string_view request, nlohmann::json& json);
	bool createFilesTable();

private: 
	void openSQL();
	bool isSQLTableExist(const std::string& tableName);
	void printFilesTable();
	

//public:	
	//bool checkFile(const File::FileMetaInfo& fmi);


	//operations
	bool insertFileInfoInTable( File::FileMetaInfo& fmi);
	bool takeFileInfoFromTable(const File::FileMetaInfo& fmi);
	bool isThisFileExisted(File::FileMetaInfo& fmi);
	bool findFileInfoInTableByHash(File::FileMetaInfo& fmi);
	bool deleteFileInfoInTableByHash(const File::FileMetaInfo& fmi);
	void syncFiles(nlohmann::json& json);
};

