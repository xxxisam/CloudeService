#include "HashFileTable.h"
#include "defaultPath.h"

HashDataBase::HashDataBase() : m_hashDataBase(nullptr)
{

};

HashDataBase& HashDataBase::getInstance()
{
	static HashDataBase instance;
	return instance;
}

void HashDataBase::start()
{
    openHashSQL();
    if (isSQLTableExist(HashDataBaseTable::tableName))
    {
        printHashTable();
    }
    else
    {
        if (createHashTable())
        {
            std::cout << "[SQL.HASH]HashDataBase::start__1.Table is been created!\n";
        }
        if (isSQLTableExist(HashDataBaseTable::tableName))
        {
            std::cout << "[SQL.HASH]HashDataBase::start__[SQL.HASH]2.Table is exist!\n";
        }
    }
}

sqlite3* HashDataBase::getHashDataBase()
{
    return m_hashDataBase.get();
}

void HashDataBase::getRequest(std::string_view request)
{
    std::cout << "SQL request " << request << "\n";

    if (request == "/print")
    {
        printHashTable();
    }
    else
    {
        std::cout << "[SQL][File][FileDataBase::getFileDataBase]FileDataBase::getRequest(std::string_view request)__Error request: " << request << "\n";
    }
}

void HashDataBase::getRequest(std::string_view request, File::FileMetaInfo& fmi)
{
    std::cout << "SQL request " << request << "\n";

    if (request == "/delete")
    {
        deleteFileInfoInTableByHash(fmi);
    }
    else if (request == "/getFileServerPath")
    {
        getServerPath(fmi);
    }
    else
    {
        std::cout << "[SQL][File][FileDataBase::getFileDataBase]FileDataBase::getRequest(std::string_view request)__Error request: " << request << "\n";
    }
}

//void HashDataBase::getRequest(std::string_view request, File::FileMetaInfo& fmi)
//{
//    std::cout << "[SQL][HASH]getRequest - " << request << "\n";
//
//
//    if (request == "/insert")
//    {
//        addHash(fmi);
//    }
//}

bool HashDataBase::createHashTable()
{
    sqlite3* db = getHashDataBase();

    if (!db)
    {
        std::cout << "[SQL.HASH]createHashTable - No db\n";
    }

    char* fdbec = nullptr;
    int rc = sqlite3_exec(db, sqlHashTable.c_str(), nullptr, nullptr, &fdbec);

    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL.HASH]createHashTable - Creatint table error:" << fdbec << "\n";

        sqlite3_free(fdbec);
        return false;
    }

    return true;
}

DataBaseUtility::UserDBResult HashDataBase::addHash(File::FileMetaInfo& fmi)
{
    std::cout << "[SQL][HashFileTable][addHash] addHash Вызвана\n";
    //File::printFileMetaInfo(fmi);
    if (checkHashExistence(fmi))
    {
        std::cout << "[SQL][HashFileTable][addHash] Hash " << fmi.hash << " is existed!\n";
        if (addRef(fmi))
        {
            std::cout << "1[SQL][HashFileTable][addHash] add ref to " << fmi.hash << " is succsss!\n";
        }
        else
        {
            std::cout << "1[SQL][HashFileTable][addHash] add ref to " << fmi.hash << " is failed!\n";
        }
        return DataBaseUtility::UserDBResult::HASH_ALREADY_EXIST;
    }
    else
    {
        std::cout << "[SQL][HashFileTable][addHash] Hash " << fmi.hash << " doesnt existed!\n";
        if (insertHashInfoInTable(fmi))
        {
            if (addRef(fmi))
            {
                std::cout << "2[SQL][HashFileTable][addHash] add ref to " << fmi.hash << " is succsss!\n";
            }
            else
            {
                std::cout << "2[SQL][HashFileTable][addHash] add ref to " << fmi.hash << " is failed!\n";
            }
        }
        return DataBaseUtility::UserDBResult::INSERT_OK;
    }
}

bool HashDataBase::getServerPath(File::FileMetaInfo& fmi)
{

    sqlite3* db = getHashDataBase();

    if (!db)
    {
        std::cout << "[SQL][HASH][getServerPath] error opening dataBase\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const std::string sqlGetHash = "SELECT fileServerPath FROM " + HashDataBaseTable::tableName + " WHERE " + HashDataBaseTable::columnHash + " = ?;";

    int errorCodeHashDataBase = sqlite3_prepare(db, sqlGetHash.c_str(), -1, &stmt, nullptr);
    if (errorCodeHashDataBase != SQLITE_OK)
    {
        std::cout << "[SQL][HASH][getServerPath] error prepare - " << errorCodeHashDataBase << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;

        
        fmi.fileServerPath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        std::cout << "Найден файл:\n";
       
        std::cout << "fullName: " << fmi.fileServerPath << "\n";


    }

    sqlite3_finalize(stmt);
    return found;
}

void HashDataBase::openHashSQL()
{
    sqlite3* sqlOpenPointer = nullptr;

    if (!std::filesystem::exists(defPath::defaultPath::hashDataBase))
    {
        std::cout << "[SQL][HASH]db doest't exist" << "\n";
    }

    int errorCodeHashDataBase = sqlite3_open(defPath::defaultPath::hashDataBase.string().c_str(), &sqlOpenPointer);

    if (errorCodeHashDataBase != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH][HashDataBase::openHashSQL]Ошибка открытия базы: " << sqlite3_errmsg(sqlOpenPointer) << "\n";
        if (m_hashDataBase)
        {
            sqlite3_close(sqlOpenPointer);
        }
        sqlOpenPointer = nullptr;
    }

    m_hashDataBase.reset(sqlOpenPointer);
}

bool HashDataBase::isSQLTableExist(const std::string& tableName)
{
    sqlite3* db = getHashDataBase();
    if (!db)
    {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;


    if (sqlite3_prepare_v2(db, sqlSelectHash.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH]isSQLTableExist - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
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

void HashDataBase::printHashTable()
{
    sqlite3* db = getHashDataBase();
    sqlite3_stmt* stmt = nullptr;

    const std::string sqlHashPrint = "SELECT "
        //id, name, fullName, extension, size, hash, isEmpty, isFileComplete, fileServerPath 
        + HashDataBaseTable::columnHashID + ", "
        + HashDataBaseTable::columnHash + ", "
        + HashDataBaseTable::columnSize + ", "
        + HashDataBaseTable::columnIsEmpty + ", "
        + HashDataBaseTable::columnServerPath + ", "
        + HashDataBaseTable::columnIsFileComplete + ", "
        + HashDataBaseTable::columnRefCount
        + " FROM "
        + HashDataBaseTable::tableName
        + " ;";

    if (sqlite3_prepare_v2(db, sqlHashPrint.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH]printHashTable - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "[HASH]id | size | hash | isEmpty | isFileComplete | fileServerPath | ref count\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char* hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sqlite3_int64 size = sqlite3_column_int64(stmt, 2);
        int isEmpty = sqlite3_column_int(stmt, 3);
        const char* fileServerPath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        int isFileComplete = sqlite3_column_int(stmt, 5);
        int refCount = sqlite3_column_int(stmt, 6);

        std::cout
            << id << " | "
            << (hash ? hash : "NULL") << " | "
            << size << " | "
            << isEmpty << " | "
            << isFileComplete << " | "
            << (fileServerPath ? fileServerPath : "NULL") << " | "
            << refCount << "\n";
    }

    std::cout << "_____________________________________________________________________________________\n";
    sqlite3_finalize(stmt);
}

bool HashDataBase::insertHashInfoInTable(File::FileMetaInfo& fmi)
{
    //second check
    if (checkHashExistence(fmi))
    {
        std::cout << "[SQL][HASH]This file already existed\n";
        return false;    
    }

    sqlite3* db = getHashDataBase(); 
    if (!db)
    {
        std::cout << "[SQL][HASH]No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const std::string sqlInsertHash = "INSERT INTO " + HashDataBaseTable::tableName + "("
        //"(hash, size, isEmpty, fileServerPath, isFileComplete)"
        + HashDataBaseTable::columnHash + ", "
        + HashDataBaseTable::columnSize + ", "
        + HashDataBaseTable::columnIsEmpty + ", "
        + HashDataBaseTable::columnServerPath + ", "
        + HashDataBaseTable::columnIsFileComplete + ") "
        + "VALUES (?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sqlInsertHash.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH]INSERT prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    //extension
    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);                         //hash
    sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(fmi.size));                          //size
    sqlite3_bind_int(stmt, 3, fmi.isEmpty ? 1 : 0);                                             //isEmpty
    sqlite3_bind_text(stmt, 4, fmi.fileServerPath.string().c_str(), -1, SQLITE_TRANSIENT);      //isFileComplete
    sqlite3_bind_int(stmt, 5, fmi.isFileComplete ? 1 : 0);                                      //fileServerPath
    //refCount

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success)
    {
        std::cerr << "[SQL][HASH]INSERT step error: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

bool HashDataBase::checkHashExistence(File::FileMetaInfo& fmi)
{
    sqlite3* db = getHashDataBase();
    if (!db)
    {
        std::cout << "[SQL][HASH][checkHashExistence] No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    const std::string sqlFindFileByHash = "SELECT "
        //hash
        + HashDataBaseTable::columnHash
        + " From "
        + HashDataBaseTable::tableName
        + " WHERE "
        + HashDataBaseTable::columnHash
        + " = ?; ";

    if (sqlite3_prepare_v2(db, sqlFindFileByHash.c_str(), -1, &stmt, nullptr))
    {
        std::cerr << "[SQL][HASH][checkHashExistence] find prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
        //stmt finalize
    }

    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::cout << "[SQL][HASH][checkHashExistence] This file exist\n";
        found = true;


       // //fmi.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)); //В другую функцию
       // //fmi.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
       // fmi.fileServerPath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

       // std::cout << "Найден файл:\n";
       // //std::cout << "id: " << id << "\n";
       //// std::cout << "name: " << fmi.name << "\n";
       // //std::cout << "fullName: " << fmi.fullName << "\n";
       // std::cout << "fullName: " << fmi.fileServerPath << "\n";


    }

    sqlite3_finalize(stmt);
    return found;
}

int HashDataBase::getHashID(const File::FileMetaInfo& fmi)
{
    sqlite3* db = getHashDataBase();
    if (!db) return -1;

    const std::string sqlGetHashID = "SELECT " + HashDataBaseTable::columnHashID +
        " FROM " + HashDataBaseTable::tableName +
        " WHERE " + HashDataBaseTable::columnHash + " = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sqlGetHashID.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH] getHashID prepare error: " << sqlite3_errmsg(db) << "\n";
        return -1;
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

bool HashDataBase::deleteFileInfoInTableByHash(const File::FileMetaInfo& fmi)
{
    std::cout << "[SQL][Hash][deleteFileInfoInTableByHash]\n";
    sqlite3* db = getHashDataBase();
    if (!db)
    {
        std::cerr << "[SQL][HASH]DB not opened\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const std::string sqlDeleteHash = "DELETE FROM "
        + HashDataBaseTable::tableName
        + " WHERE "
        + HashDataBaseTable::columnHash
        + " = ?;";

    int rc = sqlite3_prepare_v2(db, sqlDeleteHash.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH]Delete Prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }


    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "[SQL][HASH]Delete error: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }


    int deletedRows = sqlite3_changes(db);
    std::cout << "[SQL][HASH]HashDataBase::deleteFileInfoInTableByHash deleted rows :" << deletedRows << "\n";

    sqlite3_finalize(stmt);

    return deletedRows > 0;
}

void HashDataBase::syncFiles(nlohmann::json& json)
{
    std::cout << "[SQL][Hash][syncFiles]Doesn't work\n";
    sqlite3* db = getHashDataBase();
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

        /*const std::string sqlSelectFiles = "SELECT hash FROM " + HashDataBaseTable::tableName +
            " WHERE name=? AND extension=? AND fullName=? AND size=?";*/
        const std::string sqlSelectFiles = "SELECT hash FROM " + HashDataBaseTable::tableName +
            " WHERE hash=?";

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

bool HashDataBase::addRef(const File::FileMetaInfo& fmi)
{
    sqlite3* db = getHashDataBase();
    if (!db)
    {
        std::cerr << "[SQL][HASH][addRef]DB not opened\n";
        return false;
    }
    sqlite3_stmt* stmt = nullptr;

    const std::string sqlDeleteHash = "UPDATE " + HashDataBaseTable::tableName 
        + " SET "+ HashDataBaseTable::columnRefCount + " = " + HashDataBaseTable::columnRefCount + " + 1"
        + " WHERE " + HashDataBaseTable::columnHash + " = ?;";

    int rc = sqlite3_prepare_v2(db, sqlDeleteHash.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH][addRef]Delete Prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "[SQL][HASH][addRef]Delete error: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool HashDataBase::minusRef(const File::FileMetaInfo& fmi)
{
    sqlite3* db = getHashDataBase();
    if (!db)
    {
        std::cerr << "[SQL][HASH][minusRef]DB not opened\n";
        return false;
    }
    sqlite3_stmt* stmt = nullptr;

    const std::string sqlDeleteHash = "UPDATE " + HashDataBaseTable::tableName
        + " SET " + HashDataBaseTable::columnRefCount + " = " + HashDataBaseTable::columnRefCount + " - 1"
        + " WHERE " + HashDataBaseTable::columnHash + " = ?;";

    int rc = sqlite3_prepare_v2(db, sqlDeleteHash.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH][minusRef]Delete Prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "[SQL][HASH][minusRef]Delete error: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool HashDataBase::isRefCountZero(const File::FileMetaInfo& fmi)
{
    "[SQL][HASH][isRefCountZero] !_!_!_!_!__!_!_!_!_!__!_!_!_!_\n";
    sqlite3* db = getHashDataBase();
    if (!db)
    {
        std::cerr << "[SQL][HASH][isRefCountZero] DB not opened\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const std::string sql =
        "SELECT " + HashDataBaseTable::columnRefCount +
        " FROM " + HashDataBaseTable::tableName +
        " WHERE " + HashDataBaseTable::columnHash + " = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][HASH][isRefCountZero] Prepare error: "
            << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, fmi.hash.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {
        int refCount = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return refCount == 0;
    }

    if (rc == SQLITE_DONE)
    {
        // Запись не найдена — реши, что это значит в твоей логике
        sqlite3_finalize(stmt);
        return true; // или false
    }

    std::cerr << "[SQL][HASH][isRefCountZero] Step error: "
        << sqlite3_errmsg(db) << "\n";

    sqlite3_finalize(stmt);
    return false;
}


