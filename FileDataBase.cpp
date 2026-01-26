#pragma once
#include "FileDataBase.h"
#include "defaultPath.h"

FileDataBase::FileDataBase() : m_fileDataBase(nullptr)
{
    
};

FileDataBase& FileDataBase::getInstance()
{
    static FileDataBase instance;
    return instance;
}

void FileDataBase::start()
{
    openFileSQL();
    if (isSQLTableExist(FileDataBaseTable::tableName))
    {
        printFilesTable();
    }
    else
    {
        if (createFilesTable())
        {
            std::cout << "1.Table is been created!\n";
        }
        if (isSQLTableExist(FileDataBaseTable::tableName))
        {
            std::cout << "2.Table is exist!\n";
        }
    }

}

sqlite3* FileDataBase::getFileDataBase()
{
    return m_fileDataBase.get();
}

// "print", "sync"
void FileDataBase::getRequest(std::string_view request) //DElete
{
    std::cout << "SQL request " << request << "\n";

    if (request == "/print")
    {
        printFilesTable();
    }
    else
    {
        std::cout << "[SQL][File][FileDataBase::getFileDataBase]FileDataBase::getRequest(std::string_view request)__Error request: " << request << "\n";
    }
}

// "/insert", "/get", "/exist", "/delete" with fmi

void FileDataBase::getRequest(std::string_view request, File::FileMetaInfo& fmi)
{
    std::cout << "SQL request " << request << "\n";



    if (request == "/insert")
    {
        /*isThisFileExisted(fmi);
        insertFileInfoInTable(fmi);*/
    }
    else if (request == "/get")
    {
        takeFileInfoFromTable(fmi);
    }
    else if (request == "/find")
    {
        if (checkFileExistence(fmi))
        {
            std::cout << "File exist\n";
        }
        else
        {
            std::cout << "[SQL][File][getRequest]File d exist\n";
        }

    }
    else if (request == "/delete")
    {
        //deleteFileInfoInTableByHash(fmi);
    }
    else
    {
        std::cout << "[SQL][ERROR]FileDataBase::getRequest(std::string_view request, File::FileMetaInfo& fmi)__Error request: " << request << "\n";
    }
}


void FileDataBase::getRequest(std::string_view request, nlohmann::json& json, SessionManager::Token& token)
{
    std::cout << "SQL request json" << request << "\n";

    if (request == "/sync")
    {
        std::cout << "Error sync file no work\n";
        syncFiles(json, token);
    }
    else
    {
        std::cout << "[SQL][File][FileDataBase::getFileDataBaseFileDataBase::getRequest(std::string_view request, nlohmann::json& json)__Error request: " << request << "\n";
    }
}



void FileDataBase::openFileSQL()
{
    sqlite3* sqlOpenPointer = nullptr;

    if (!std::filesystem::exists(defPath::defaultPath::fileDataBase))
    {
        std::cout << "[SQL][File][db doest't exist" << "\n";
    }

    int errorCodeFileDataBase = sqlite3_open(defPath::defaultPath::fileDataBase.string().c_str(), &sqlOpenPointer);

    if (errorCodeFileDataBase != SQLITE_OK)
    {
        std::cerr << "[SQL][File][Ошибка открытия базы: " << sqlite3_errmsg(sqlOpenPointer) << "\n";
        if (m_fileDataBase)
        {
            sqlite3_close(sqlOpenPointer);
        }
        sqlOpenPointer = nullptr;
    }

    m_fileDataBase.reset(sqlOpenPointer);
}

bool FileDataBase::createFilesTable()
{
    sqlite3* db = getFileDataBase();

    if (!db)
    {
        std::cout << "[SQL][File][createFilesTable - No db\n";
        return false;
    }

    char* fkErr = nullptr;
    int fkRc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &fkErr);
    if (fkRc != SQLITE_OK)
    {
        std::cerr << "[SQL][File] Cannot enable foreign keys: " << (fkErr ? fkErr : "unknown") << "\n";
        sqlite3_free(fkErr);
        return false;
    }

    const std::string sqlFilesTable =
        "CREATE TABLE IF NOT EXISTS " + FileDataBaseTable::tableName + " ( "
        + FileDataBaseTable::columnID + " INTEGER PRIMARY KEY, "
        + FileDataBaseTable::columnName + " TEXT NOT NULL, "
        + FileDataBaseTable::columnFullName + " TEXT NOT NULL, "
        + FileDataBaseTable::columnExtension + " TEXT, "
        + FileDataBaseTable::columnSize + " INTEGER, "
        + FileDataBaseTable::columnHash + " TEXT, "
        + FileDataBaseTable::columnHashID + " INTEGER NOT NULL, "
        + FileDataBaseTable::columnUploaderLogin + " TEXT, "
        + "FOREIGN KEY(" + FileDataBaseTable::columnHashID + ") REFERENCES "
        + HashDataBaseTable::tableName + "(" + HashDataBaseTable::columnHashID + ") "
        + "ON DELETE CASCADE"
        + ");";

    char* fdbec = nullptr;
    int rc = sqlite3_exec(db, sqlFilesTable.c_str(), nullptr, nullptr, &fdbec);
    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][Files] createFilesTable error: "
            << (fdbec ? fdbec : "unknown") << "\n";
        sqlite3_free(fdbec);
        return false;
    }

    return true;
}

bool FileDataBase::isSQLTableExist(const std::string& tableName)
{
    sqlite3* db = getFileDataBase();
    if (!db)
    {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;


    if (sqlite3_prepare_v2(db, sqlSelectName.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][File][isSQLTableExist - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, tableName.c_str(), -1, SQLITE_TRANSIENT);

    bool exist = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        exist = true;
    }

    sqlite3_finalize(stmt);
    return exist;
}

void FileDataBase::printFilesTable()
{
    sqlite3* db = getFileDataBase();
    if (!db) { std::cout << "[SQL][File][printFilesTable] db No such table!"; }
    sqlite3_stmt* stmt = nullptr;

    const char* sqlPrint = "SELECT id, name, extension, fullName, size, hash, hashID, uploaderLogin FROM files;";

    if (sqlite3_prepare_v2(db, sqlPrint, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][File][printFilesTable] - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "id | name | fullName | extension | size | hash | isEmpty | isFileComplete | fileServerPath | login |\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* extension = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        sqlite3_int64 size = sqlite3_column_int64(stmt, 4);
        const char* hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* uploaderLogin = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        const char* hashID = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
       

       std::cout << id << " | " << name << " | " << fullName << " | " << extension << " | " << size  << " | " << hash << " | " << hashID << " | " << uploaderLogin << " | \n";
    }

    std::cout << "_____________________________________________________________________________________\n";

    sqlite3_finalize(stmt);
}

DataBaseUtility::UserDBResult FileDataBase::addRow(File::FileMetaInfo& fmi, const int& hashID, const std::string login)
{
    std::cout << "Login: ___________" << login << "\n";
    if (!checkFileExistence(fmi))
    {
        std::cout << "[SQL][File][addRow] file doesnt exist!\n";
        if (insertFileInfoInTable(fmi, hashID, login))
        {
            std::cout << "[SQL][File][addRow] Success insertion\n";
            return DataBaseUtility::UserDBResult::INSERT_OK;
        }
        else
        {
            std::cout << "[SQL][File][addRow] FAILED insertion\n";
            return DataBaseUtility::UserDBResult::INSERT_ERROR;
        }
    }
    else
    {
        std::cout << "[SQL][File][addRow] file exist!\n";
        insertFileInfoInTable(fmi, hashID, login);
        return DataBaseUtility::UserDBResult::OK; 
    }
}

bool FileDataBase::delUserFile(File::FileMetaInfo& fmi, const SessionManager::Token& token)
{
    std::cout << "\n\n[SQL][File][delUserFile]\n";
    //if()

    return deleteFileInfoInTableByHash(fmi, token);
}

void FileDataBase::syncFiles(nlohmann::json& json, const SessionManager::Token& token)
{
    sqlite3* db = getFileDataBase();
    if (!db) return;

    json = nlohmann::json::array();

    for (const auto& entry : std::filesystem::directory_iterator(defPath::defaultPath::serverFolder))
    {
        //std::cout << "_______NEXTFILE_________________________!_____________\n";
        if (!entry.is_regular_file()) continue;

        std::filesystem::path path = entry.path();
        const int size = entry.file_size();
        std::string name = path.stem().string();
        std::string extension = path.extension().string();
        std::string fullName = path.filename().string();
        std::string serverPath;
        try {
            serverPath = std::filesystem::canonical(path).string().c_str();
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "[SQL][File][File doesn't exist or error: " << e.what() << "\n";
        }
        /*std::cout << "Checking path for:\n";
        std::cout << "checkFilesExistance---Server path: " << name << "\n";
        std::cout << "checkFilesExistance---Server path: " << extension << "\n";
        std::cout << "checkFilesExistance---Server path: " << fullName << "\n";
        std::cout << "checkFilesExistance---Server path: " << size << "\n";
        std::cout << "checkFilesExistance---Server path: " << serverPath.c_str() << "\n";*/


        sqlite3_stmt* stmt = nullptr;
        
        const std::string sqlSelectFiles = "SELECT hash FROM " + FileDataBaseTable::tableName +
            " WHERE name=? AND extension=? AND fullName=? AND size=?";

        if (sqlite3_prepare_v2(db, sqlSelectFiles.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[SQL][File][syncFiles][SQLite prepare error: " << sqlite3_errmsg(db) << "\n";
            continue;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, extension.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, fullName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(size));
        sqlite3_bind_text(stmt, 5, serverPath.c_str(), -1, SQLITE_TRANSIENT);

        int stepResult = sqlite3_step(stmt);
        bool existsInDB = (stepResult == SQLITE_ROW);
        std::string hash;
        if (existsInDB)
        {
            const unsigned char* text = sqlite3_column_text(stmt, 0); // hash
            if (text)
                hash = reinterpret_cast<const char*>(text);
        }
        else
        {
            std::cout << "[SQL][File][syncFiles]Doesnt exist\n";
        }

        

        sqlite3_finalize(stmt);

    
        nlohmann::json fileJson;
        fileJson["name"] = name;
        fileJson["extension"] = extension;
        fileJson["fullName"] = fullName;
        fileJson["size"] = size;
        fileJson["fileServerPath"] = serverPath;
        fileJson["existsInDB"] = existsInDB;
        fileJson["hash"] = hash;

        json.push_back(fileJson);

        std::cout << "[SQL][File][checkFilesExistance---JSON1: " << fileJson["fileServerPath"] << "\n";
    }
}

std::string normalizePath(std::string s)
{
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

bool FileDataBase::returnFilesFromDataBase(nlohmann::json& json, const std::string login)
{
    sqlite3* db = getFileDataBase();
    if (!db) return false;

    /*json = nlohmann::json::array();*/

    //for (const auto& entry : std::filesystem::directory_iterator(defPath::defaultPath::serverFolder))
    //{
    //    //std::cout << "_______NEXTFILE_________________________!_____________\n";
    //    if (!entry.is_regular_file()) continue;

    //    std::filesystem::path path = entry.path();
    //    const int size = entry.file_size();
    //    std::string name = path.stem().string();
    //    std::string extension = path.extension().string();
    //    std::string fullName = path.filename().string();
    //    std::string serverPath;
    //    try {
    //        serverPath = std::filesystem::canonical(path).string().c_str();
    //    }
    //    catch (const std::filesystem::filesystem_error& e) {
    //        std::cerr << "[SQL][File][File doesn't exist or error: " << e.what() << "\n";
    //    }
    //    /*std::cout << "Checking path for:\n";
    //    std::cout << "checkFilesExistance---Server path: " << name << "\n";
    //    std::cout << "checkFilesExistance---Server path: " << extension << "\n";
    //    std::cout << "checkFilesExistance---Server path: " << fullName << "\n";
    //    std::cout << "checkFilesExistance---Server path: " << size << "\n";
    //    std::cout << "checkFilesExistance---Server path: " << serverPath.c_str() << "\n";*/


        sqlite3_stmt* stmt = nullptr;

        const std::string sqlSelectFiles = "SELECT name, extension, fullName, size, hash, uploaderLogin "
            "FROM " + FileDataBaseTable::tableName +
            " WHERE uploaderLogin = ?";;

        if (sqlite3_prepare_v2(db, sqlSelectFiles.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[SQL][File][returnFilesFromDataBase][SQLite prepare error: " << sqlite3_errmsg(db) << "\n";
            return false;
        }

        sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_TRANSIENT);
        std::cout << "\n\n\nJSONBAG\n\n\n";
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            nlohmann::json fileJson;

            fileJson["name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            fileJson["extension"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            fileJson["fullName"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            fileJson["size"] = sqlite3_column_int64(stmt, 3);
            fileJson["hash"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            fileJson["uploaderLogin"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

            json.push_back(fileJson);
        }

        sqlite3_finalize(stmt);
        return true;
}

bool FileDataBase::insertFileInfoInTable(File::FileMetaInfo& fmi,const int& hashID, const std::string login)
{
    std::cout << "[SQL][File][insertFileInfoInTable] check insertion login:" << login <<"\n";

    sqlite3* db = getFileDataBase(); 
    if (!db)
    {
        std::cout << "[SQL][File][No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    

    const std::string sqlInsertFile = "INSERT INTO " + FileDataBaseTable::tableName + " ( "
        //+ "name, fullName, extension, size, hash, isEmpty, isFileComplete, fileServerPath)"
        + FileDataBaseTable::columnName + ", "
        + FileDataBaseTable::columnExtension + ", "
        + FileDataBaseTable::columnFullName + ", "
        + FileDataBaseTable::columnSize + ", "
        + FileDataBaseTable::columnHash + ", "
        + FileDataBaseTable::columnHashID + ", "
        + FileDataBaseTable::columnUploaderLogin + " ) "
        + "VALUES ( ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sqlInsertFile.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][File][INSERT] prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
   
    sqlite3_bind_text(stmt, 1, fmi.name.c_str(), -1, SQLITE_TRANSIENT);                         //name
    sqlite3_bind_text(stmt, 2, fmi.extension.c_str(), -1, SQLITE_TRANSIENT);                    //extension
    sqlite3_bind_text(stmt, 3, fmi.fullName.c_str(), -1, SQLITE_TRANSIENT);                     //fullName
    sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(fmi.size));                          //size
    sqlite3_bind_text(stmt, 5, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);                          //size
    sqlite3_bind_int(stmt, 6, hashID);                                                             //login
    sqlite3_bind_text(stmt, 7, login.c_str(), -1, SQLITE_TRANSIENT);                                                      //hashID                          

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success)
    {
        std::cerr << "[SQL][File][INSERT step error: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

bool FileDataBase::takeFileInfoFromTable(const File::FileMetaInfo& fmi)
{
    sqlite3* db = getFileDataBase(); //вынести за скобки
    if (!db)
    {
        std::cout << "[SQL][File][No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const char* sqlFindFileByHash = "SELECT id, name, fullName From files WHERE hash = ?";

    if (sqlite3_prepare_v2(db, sqlFindFileByHash, -1, &stmt, nullptr))
    {
        std::cerr << "[SQL][File][takeFileInfoFromTable-- Take information prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    return false; //griohjg9uirtehwuighreu9w8ighriuewhgui        repair
}

int FileDataBase::getFileIDIfExist(File::FileMetaInfo& fmi) // const>
{
    //return FileDataBase::findFileInfoInTableByHash(fmi);
    sqlite3* db = getFileDataBase();
    if (!db) return -1;

    const std::string sqlGetFileID = "SELECT " + FileDataBaseTable::columnHashID +
        " FROM " + FileDataBaseTable::tableName +
        " WHERE " + FileDataBaseTable::columnHash + " = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sqlGetFileID.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][File] getFileID prepare error: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    int hashID = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        hashID = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return hashID;
}

bool FileDataBase::checkFileExistence(File::FileMetaInfo& fmi)
{
    sqlite3* db = getFileDataBase();
    if (!db)
    {
        std::cout << "[SQL][File][No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const char* sqlFindFileByHash = "SELECT id, name, fullName From files WHERE hash = ? AND uploaderLogin = ?;";

    if (sqlite3_prepare_v2(db, sqlFindFileByHash, -1, &stmt, nullptr))
    {
        std::cerr << "[SQL][File][find prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

bool FileDataBase::deleteFileInfoInTableByHash(const File::FileMetaInfo& fmi, const SessionManager::Token& token)
{
    sqlite3* db = getFileDataBase();
    if (!db)
    {
        std::cerr << "[SQL][File][DB not opened\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const char* sqlDelete = "DELETE FROM files WHERE hash = ? AND fullName = ? AND uploaderLogin = ?;";

    int rc = sqlite3_prepare_v2(db, sqlDelete, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][File][Delete Prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }


    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, fmi.fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, token.userLogin.c_str(), -1, SQLITE_TRANSIENT);
    

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "[SQL][File][Delete error: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    
    int deletedRows = sqlite3_changes(db);
    std::cout  << "[SQL][File][deleteFileInfoInTableByHash__Deleted :" << deletedRows << "\n";

    sqlite3_finalize(stmt);

    return deletedRows > 0;
}
