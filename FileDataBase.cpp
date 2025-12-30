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
    openSQL();
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

sqlite3* FileDataBase::getDataBase()
{
    return m_fileDataBase.get();
}



// "print", "sync"
void FileDataBase::getRequest(std::string_view request)
{
    std::cout << "SQL request " << request << "\n";

    if (request == "/print")
    {
        printFilesTable();
    }
    else
    {
        std::cout << "FileDataBase::getRequest(std::string_view request)__Error request: " << request << "\n";
    }
}

// "/insert", "/get", "/exist", "/delete" with fmi

void FileDataBase::getRequest(std::string_view request, File::FileMetaInfo& fmi)
{
    std::cout << "SQL request " << request << "\n";



    if (request == "/insert")
    {
        insertFileInfoInTable(fmi);
    }
    else if (request == "/get")
    {
        takeFileInfoFromTable(fmi);
    }
    else if (request == "/find")
    {
        findFileInfoInTableByHash(fmi);
    }
    else if (request == "/delete")
    {
        deleteFileInfoInTableByHash(fmi);
    }
    else
    {
        std::cout << "FileDataBase::getRequest(std::string_view request, File::FileMetaInfo& fmi)__Error request: " << request << "\n";
    }
}


void FileDataBase::getRequest(std::string_view request, nlohmann::json& json)
{
    std::cout << "SQL request json" << request << "\n";

    if (request == "/sync")
    {
        syncFiles(json);
    }
    else
    {
        std::cout << "FileDataBase::getRequest(std::string_view request, nlohmann::json& json)__Error request: " << request << "\n";
    }
}



void FileDataBase::openSQL()
{
    sqlite3* sqlOpenPointer = nullptr;

    if (!std::filesystem::exists(defPath::defaultPath::fileDataBase))
    {
        std::cout << "db doest't exist" << "\n";
    }

    int errorCodeFileDataBase = sqlite3_open(defPath::defaultPath::fileDataBase.string().c_str(), &sqlOpenPointer);

    if (errorCodeFileDataBase != SQLITE_OK)
    {
        std::cerr << "Ошибка открытия базы: " << sqlite3_errmsg(sqlOpenPointer) << "\n";
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
    sqlite3* db = getDataBase();

    if (!db)
    {
        std::cout << "createFilesTable - No db\n";
    }

    char* fdbec = nullptr;
    int rc = sqlite3_exec(db, sqlFilesTable.c_str(), nullptr, nullptr, &fdbec);

    if (rc != SQLITE_OK)
    {
        std::cerr << "createFilesTable - Creatint table error:" << fdbec << "\n";

        sqlite3_free(fdbec);
        return false;
    }

    return true;
}

bool FileDataBase::isSQLTableExist(const std::string& tableName)
{
    sqlite3* db = getDataBase();
    if (!db)
    {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    std::string sqlRequest = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";


    if (sqlite3_prepare_v2(db, sqlRequest.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "isSQLTableExist - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
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
    sqlite3* db = getDataBase();
    const char* sql = "SELECT id, name, fullName, extension, size, hash, isEmpty, isFileComplete, fileServerPath FROM files;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "printFilesTable - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "id | name | fullName | extension | size | hash | isEmpty | isFileComplete | fileServerPath\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* extension = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        sqlite3_int64 size = sqlite3_column_int64(stmt, 4);
        const char* hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        int isEmpty = sqlite3_column_int(stmt, 6);
        int isFileComplete = sqlite3_column_int(stmt, 7);
        const char* fileServerPath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        std::cout << id << " | " << name << " | " << fullName << " | "
            << extension << " | " << size << " | " << hash << " | "
            << isEmpty << " | " << isFileComplete << " | " << fileServerPath << "\n";
    }

    sqlite3_finalize(stmt);
}

void FileDataBase::syncFiles(nlohmann::json& json)
{
    sqlite3* db = getDataBase();
    if (!db) return;

    json = nlohmann::json::array();

    for (const auto& entry : std::filesystem::directory_iterator(defPath::defaultPath::serverFolder))
    {
        //std::cout << "_______NEXTFILE_________________________!_____________\n";
        if (!entry.is_regular_file()) continue;

        auto path = entry.path();
        auto size = entry.file_size();
        std::string name = path.stem().string();
        std::string extension = path.extension().string();
        std::string fullName = path.filename().string();
        std::string serverPath;
        try {
            serverPath = std::filesystem::canonical(path).string().c_str();
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "File doesn't exist or error: " << e.what() << "\n";
        }
        /*std::cout << "Checking path for:\n";
        std::cout << "checkFilesExistance---Server path: " << name << "\n";
        std::cout << "checkFilesExistance---Server path: " << extension << "\n";
        std::cout << "checkFilesExistance---Server path: " << fullName << "\n";
        std::cout << "checkFilesExistance---Server path: " << size << "\n";
        std::cout << "checkFilesExistance---Server path: " << serverPath.c_str() << "\n";*/


        sqlite3_stmt* stmt = nullptr;
        
        if (sqlite3_prepare_v2(db, sqlSelect.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "SQLite prepare error: " << sqlite3_errmsg(db) << "\n";
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
            std::cout << "Doesnt exist\n";
        }

        

        sqlite3_finalize(stmt);

        // Формируем JSON
        nlohmann::json fileJson;
        fileJson["name"] = name;
        fileJson["extension"] = extension;
        fileJson["fullName"] = fullName;
        fileJson["size"] = size;
        fileJson["fileServerPath"] = serverPath;
        fileJson["existsInDB"] = existsInDB;
        fileJson["hash"] = hash;

        json.push_back(fileJson);

        std::cout << "checkFilesExistance---JSON: " << fileJson["fileServerPath"] << "\n";
    }
}

std::string normalizePath(std::string s)
{
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

bool FileDataBase::insertFileInfoInTable(File::FileMetaInfo& fmi)
{
    if (isThisFileExisted(fmi))
    {
        std::cout << "This file already existed\n";
        return false;    // Обработать.
    }

    sqlite3* db = getDataBase(); //вынести за скобки
    if (!db)
    {
        std::cout << "No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlInsertFile.c_str(), -1, &stmt, nullptr))
    {
        std::cerr << "INSERT prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, fmi.name.c_str(), -1, SQLITE_TRANSIENT);                         //name
    sqlite3_bind_text(stmt, 2, fmi.fullName.c_str(), -1, SQLITE_TRANSIENT);                     //fullName
    sqlite3_bind_text(stmt, 3, fmi.extension.c_str(), -1, SQLITE_TRANSIENT);                    //extension
    sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(fmi.size));                          //size
    sqlite3_bind_text(stmt, 5, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);                         //hash
    sqlite3_bind_int(stmt, 6, fmi.isEmpty ? 1 : 0);                                             //isEmpty
    sqlite3_bind_int(stmt, 7, fmi.isFileComplete ? 1 : 0);                                      //isFileComplete
    sqlite3_bind_text(stmt, 8, fmi.fileServerPath.string().c_str(), -1, SQLITE_TRANSIENT);      //fileServerPath

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success)
    {
        std::cerr << "INSERT step error: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

bool FileDataBase::takeFileInfoFromTable(const File::FileMetaInfo& fmi)
{
    sqlite3* db = getDataBase(); //вынести за скобки
    if (!db)
    {
        std::cout << "No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlFindFileByHash, -1, &stmt, nullptr))
    {
        std::cerr << "takeFileInfoFromTable-- Take information prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    return false; //griohjg9uirtehwuighreu9w8ighriuewhgui        repair
}

bool FileDataBase::isThisFileExisted(File::FileMetaInfo& fmi) // const>
{
    return FileDataBase::findFileInfoInTableByHash(fmi);
}

bool FileDataBase::findFileInfoInTableByHash(File::FileMetaInfo& fmi)
{
    sqlite3* db = getDataBase();
    if (!db)
    {
        std::cout << "No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlFindFileByHash, -1, &stmt, nullptr))
    {
        std::cerr << "find prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;

        
        fmi.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        fmi.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        fmi.fileServerPath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        std::cout << "Найден файл:\n";
        //std::cout << "id: " << id << "\n";
        std::cout << "name: " << fmi.name << "\n";
        std::cout << "fullName: " << fmi.fullName << "\n";
        std::cout << "Path: " << fmi.fileServerPath << "\n";

    }

    sqlite3_finalize(stmt);
    return found;
}

bool FileDataBase::deleteFileInfoInTableByHash(const File::FileMetaInfo& fmi)
{
    sqlite3* db = getDataBase();
    if (!db)
    {
        std::cerr << "DB not opened\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sqlDelete, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Delete Prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    // bind hash
    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Delete error: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    // сколько строк удалено
    int deletedRows = sqlite3_changes(db);
    std::cout  << "deleteFileInfoInTableByHash__Deleted :" << deletedRows << "\n";

    sqlite3_finalize(stmt);

    return deletedRows > 0;
}
