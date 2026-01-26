#pragma once
#include "sqlite3.h"
#include "File.hpp"
#include <memory>
#include <string>
#include "DataBaseUtility.hpp"
#include "HashFileTable.h"
#include "SessionManager.h"

struct FileDataBaseTable
{
	inline static const std::string tableName = "files";
	inline static const std::string columnID = "id";
	inline static const std::string columnName = "name";
	inline static const std::string columnExtension = "extension";
	inline static const std::string columnFullName = "fullName";
	inline static const std::string columnSize = "size";
	inline static const std::string columnHash = "hash";
	inline static const std::string columnUploaderLogin = "uploaderLogin";
	inline static const std::string columnHashID = "hashID";
};

class FileDataBase
{
private:
	std::unique_ptr<sqlite3, DataBaseUtility::DataBaseDeleter> m_fileDataBase;

	//sqlRqsts
	//FileTable
	/*const std::string sqlFilesTable =
		"CREATE TABLE IF NOT EXISTS files ("
		"id INTEGER PRIMARY KEY, "
		"name TEXT NOT NULL, "
		"fullName TEXT NOT NULL, "
		"extension TEXT, "
		"size INTEGER, "
		"hash TEXT, "
		"hashID INTEGER NOT NULL, "
		"FOREIGN KEY(hashID) REFERENCES hashs(id) "
		"ON DELETE CASCADE"
		");";*/

	
	const std::string sqlSelectName = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
	const char* sqlGetFile = "SELECT id, name, extension, fullName, size FROM files;";
	
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
	sqlite3* getFileDataBase();
	void getRequest(std::string_view request);
	void getRequest(std::string_view request, File::FileMetaInfo& fmi);
	void getRequest(std::string_view request, nlohmann::json& json, SessionManager::Token& token);
	bool createFilesTable();
	DataBaseUtility::UserDBResult addRow(File::FileMetaInfo& fmi, const int& hashID, const std::string login);
	bool delUserFile(File::FileMetaInfo& fmi, const SessionManager::Token& login);
	bool returnFilesFromDataBase(nlohmann::json& json, const std::string login);


private: 
	void openFileSQL();
	bool isSQLTableExist(const std::string& tableName);
	void printFilesTable();

//public:	
	//bool checkFile(const File::FileMetaInfo& fmi);


	//operations
	bool insertFileInfoInTable(File::FileMetaInfo& fmi, const int& hashID, const std::string login);
	bool checkHash();
	bool takeFileInfoFromTable(const File::FileMetaInfo& fmi);
	bool checkFileExistence(File::FileMetaInfo& fmi);
	int getFileIDIfExist(File::FileMetaInfo& fmi);
	bool deleteFileInfoInTableByHash(const File::FileMetaInfo& fmi, const SessionManager::Token& token);
	void syncFiles(nlohmann::json& json, const SessionManager::Token& token);
};

