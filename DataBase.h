#pragma once
#include "sqlite3.h"

#include <memory>
#include <string>
#include "DataBaseUtility.hpp"


class DataBase
{
private:
	std::unique_ptr<sqlite3, DataBaseUtility::DataBaseDeleter> m_fileDataBase;
	std::unique_ptr<sqlite3, DataBaseUtility::DataBaseDeleter> m_hashDataBase;
	std::unique_ptr<sqlite3, DataBaseUtility::DataBaseDeleter> m_userDataBase;


	bool isFileDBTableOpen = false;
	bool isHashDBTableOpen = false;
	bool isUserDBTableOpen = false;



public:


	//
	virtual void start() = 0;
	sqlite3* getFileDataBase();
	sqlite3* getHashDataBase();
	sqlite3* userHashDataBase();
	//sqlite3* getUserDataBase();
	virtual void getRequest();
	virtual bool createTable() = 0;

private:
	virtual void openSQL();
	virtual bool isSQLTableExist(const std::string& tableName);
	virtual void printFilesTable();

		//operations
	virtual bool insertRowInTable();
	virtual bool takeRowFromTable();
	virtual bool isRowExisted();
	virtual bool findRow();
	virtual bool deleteRowFromTable();
	virtual void syncTable();
};

