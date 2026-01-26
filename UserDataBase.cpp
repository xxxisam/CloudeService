#include "UserDataBase.h"
#include <iostream>
#include "defaultPath.h"
#include "sqlDataBaseRequest.h"

UserDataBase::UserDataBase() : m_userDataBase(nullptr)
{

};

UserDataBase& UserDataBase::getInstance()
{
    static UserDataBase instance;
    return instance;
}

void UserDataBase::start()
{
    openUserSQL();
    if (isSQLTableExist(UserDataBaseTable::tableName))
    {
        "[SQL][User][start] 1.User Table is exist!\n";
        printUsersTable();
    }
    else
    {
        if (createUsersTable())
        {
            std::cout << "[SQL][User][start] 1.User Table is been created!\n";
        }
        if (isSQLTableExist(UserDataBaseTable::tableName))
        {
            std::cout << "[SQL][User][start] 2. User Table is exist!\n";
        }
    }

}

sqlite3* UserDataBase::getUserDataBase()
{
    return m_userDataBase.get();
}

void UserDataBase::getRequest(std::string_view request)
{

}

void UserDataBase::getRequest(std::string_view request, User::User& user)
{
    std::cout << "[SQL][User][getRequest] request is - " << request << "\n";
    if (request == sqlDataBaseRequest::sqlInsert)
    {
        bool success = insertUserInfoInTable(user);
        if (!success)
        {
            std::cout << "[SQL][User][getRequest] - insert unsuccess\n";
        }
    }
}

void UserDataBase::openUserSQL()
{
    std::cout << "[SQL][User][openUserSQL] opening sql User table\n";
    sqlite3* sqlOpenPointer = nullptr;

    if (!std::filesystem::exists(defPath::defaultPath::userDataBase))
    {
        std::cout << "[SQL][User][openUserSQL] db doest't exist" << "\n";
    }

    int errorCodeUserDataBase = sqlite3_open(defPath::defaultPath::userDataBase.string().c_str(), &sqlOpenPointer);
    std::cout << "[SQL][User][openUserSQL] DB path: " << defPath::defaultPath::userDataBase << "\n";

    if (errorCodeUserDataBase != SQLITE_OK)
    {
        std::cerr << "[SQL][User][openUserSQL] Ошибка открытия базы: " << sqlite3_errmsg(sqlOpenPointer) << "\n";
        if (m_userDataBase)
        {
            sqlite3_close(sqlOpenPointer);
        }
        sqlOpenPointer = nullptr;
    }

    m_userDataBase.reset(sqlOpenPointer);
}

bool UserDataBase::isSQLTableExist(const std::string& tableName)
{
    sqlite3* db = getUserDataBase();
    if (!db)
    {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const std::string sqlSelectName = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";

    if (sqlite3_prepare_v2(db, sqlSelectName.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][User][UserDataBase] - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
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

void UserDataBase::printUsersTable()
{
    sqlite3* db = getUserDataBase();

    if (!db) { std::cout << "[SQL][User][printUsersTable] db No such table!"; }

    sqlite3_stmt* stmt = nullptr;

    std::string sqlPrint = "SELECT id, userName, userLogin, userPassword, isAdmin, maxStorageSize FROM " + UserDataBaseTable::tableName + ";";

    if (sqlite3_prepare_v2(db, sqlPrint.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][User][printFilesTable] - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "[USER]|id | userName | userID | userLogin | userPassword | isAdmin | maxStorageSize |" << "\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char* userName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* userLogin = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* userPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int isAdmin = sqlite3_column_int(stmt, 4);
        int maxStorageSize = sqlite3_column_int(stmt, 5);

        std::cout
            << id << " | "
            << userName << " | "
            << userLogin << " | "
            << userPassword << " | "
            << isAdmin << " | "
            << maxStorageSize << "\n";
    }

    std::cout << "_____________________________________________________________________________________\n";

    sqlite3_finalize(stmt);
}

bool UserDataBase::insertUserInfoInTable(const User::User& user)
{
    sqlite3* db = getUserDataBase();
    if (!db)
    {
        std::cerr << "[SQL][User] no db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const std::string sqlInsertUser =
        "INSERT INTO " + UserDataBaseTable::tableName +
        "("
        + UserDataBaseTable::columnUserName + ", "
        + UserDataBaseTable::columnUserLogin + ", "
        + UserDataBaseTable::columnUserPassword + ", "
        + UserDataBaseTable::columnIsAdmin + ", "
        + UserDataBaseTable::columnMaxStorageSize +
        ") VALUES (?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sqlInsertUser.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][User] prepare failed: "
            << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, user.userName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.login.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, user.isAdmin ? 1 : 0);
    sqlite3_bind_int64(stmt, 5, 500);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "[SQL][User] insert failed\n";
        std::cerr << "[SQL][User] rc = " << rc << "\n";
        std::cerr << "[SQL][User] error = "
            << sqlite3_errmsg(db) << "\n";

        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool UserDataBase::findByLoginIfExist(const User::User& user)
{
    sqlite3* db = getUserDataBase();
    if (!db)
    {
        std::cout << "[SQL][User][findByLoginIfExist] db dostnt exist!\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    std::string sqlFindFileByUserName = "SELECT 1 FROM " + UserDataBaseTable::tableName +
        " WHERE " + UserDataBaseTable::columnUserLogin + " = ? LIMIT 1;";

    int rc = sqlite3_prepare_v2(db, sqlFindFileByUserName.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cout << "[SQL][User][findByLoginIfExist] prepare error\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, user.login.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    if (exists)
        std::cout << "[SQL][User][findByLoginIfExist] login '" << user.login << "' exists\n";
    else
        std::cout << "[SQL][User][findByLoginIfExist] login '" << user.login << "' NOT exists\n";

    sqlite3_finalize(stmt);
    return exists;
}

int UserDataBase::getIdByLogin(const User::User& user)
{
    sqlite3* db = getUserDataBase();
    if (!db)
    {
        std::cout << "[SQL][User][getIdByLogin] db dostnt exist!\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    std::string sqlFindFileByUserName = "SELECT " + UserDataBaseTable::columnID +
        " FROM " + UserDataBaseTable::tableName +
        " WHERE " + UserDataBaseTable::columnUserLogin + " = ? LIMIT 1;";;

    int rc = sqlite3_prepare_v2(db, sqlFindFileByUserName.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cout << "[SQL][User][getIdByLogin] prepare error\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, user.login.c_str(), -1, SQLITE_TRANSIENT);

    int userID = -1;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::cout << "[SQL][User][getIdByLogin]" << user.login << " This login exist\n";
        userID = sqlite3_column_int(stmt, 0);
    }
    else
    {
        std::cout << "[SQL][User][getIdByLogin]" << user.login << " DOesnt exist this login login exist\n";
    }

    sqlite3_finalize(stmt);
    return userID;
}

bool UserDataBase::createUsersTable()
{
    sqlite3* db = getUserDataBase();

    if (!db)
    {
        std::cout << "[SQL][User][createUserTable] - No db\n";
        return false;
    }

    char* uErr = nullptr;
    int uRc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &uErr);
    if (uRc != SQLITE_OK)
    {
        std::cerr << "[SQL][User] Cannot enable foreign keys: " << (uErr ? uErr : "unknown") << "\n";
        sqlite3_free(uErr);
        return false;
    }

    const std::string sqlUserTable =
        "CREATE TABLE IF NOT EXISTS " + UserDataBaseTable::tableName + " ( "
        + UserDataBaseTable::columnID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
        + UserDataBaseTable::columnUserName + " TEXT NOT NULL,"
        + UserDataBaseTable::columnUserLogin + " TEXT, "
        + UserDataBaseTable::columnUserPassword + " TEXT, "
        + UserDataBaseTable::columnIsAdmin + " INTEGER, "
        + UserDataBaseTable::columnMaxStorageSize + " INTEGER "
        /*+ "FOREIGN KEY(" + FileDataBaseTable::columnHashID + ") REFERENCES "
        + HashDataBaseTable::tableName + "(" + HashDataBaseTable::columnHashID + ") "
        + "ON DELETE CASCADE"*/
        //+ ");";
        + ");";

    char* fdbec = nullptr;
    int rc = sqlite3_exec(db, sqlUserTable.c_str(), nullptr, nullptr, &fdbec);
    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][User] createFilesTable error: "
            << (fdbec ? fdbec : "unknown") << "\n";
        sqlite3_free(fdbec);
        return false;
    }

    return true;
}

DataBaseUtility::UserDBResult UserDataBase::addUserInTable(const User::User& user)
{
    std::cout << "[SQL][User][addUserInTable] findByLoginIfExist() \n";
    if (!findByLoginIfExist(user))
    {
        std::cout << "[SQL][User][addUserInTable] This user doesnt existed \n";
        std::cout << "[SQL][User][addUserInTable] insertUserInfoInTable(user); \n";
        insertUserInfoInTable(user);
        return DataBaseUtility::UserDBResult::OK;
    }
    else
    {
        std::cout << "[SQL][User][addUserInTable] Tsis user already existed \n";
        return DataBaseUtility::UserDBResult::USER_ALREADY_EXIST;
    }
}

DataBaseUtility::UserDBResult UserDataBase::verification(const User::User& user)
{
    std::cout << "[SQL][User][verification] verify() \n";

    if (!findByLoginIfExist(user))
    {
        std::cout << "[SQL][User][verification] User Doesnt exist \n";
        return DataBaseUtility::UserDBResult::USER_DOESNT_EXIST;
    }

    if (!verifyPassword(user))
    {
        std::cout << "[SQL][User][verification] Password of user - " << user.login <<"  - WRONG \n";
        return DataBaseUtility::UserDBResult::WRONG_PASSWORD;
    }

    return DataBaseUtility::UserDBResult::OK;
}

//std::string UserDataBase::getLogin(const User::User& user)
//{
//    sqlite3* db = getUserDataBase();
//    if (!db) return {};
//
//    sqlite3_stmt* stmt = nullptr;
//
//    const char* sql =
//        "SELECT userLogin FROM users WHERE id = ?;";
//
//    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
//        return {};
//
//    sqlite3_bind_int(stmt, 1, userID);
//
//    std::string login;
//
//    if (sqlite3_step(stmt) == SQLITE_ROW)
//    {
//        const unsigned char* txt = sqlite3_column_text(stmt, 0);
//        if (txt)
//            login = reinterpret_cast<const char*>(txt);
//    }
//
//    sqlite3_finalize(stmt);
//    return login;
//}

bool UserDataBase::verifyPassword(const User::User& user)
{
    sqlite3* db = getUserDataBase();
    if (!db)
    {
        std::cout << "[SQL][User][verification] - No db\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    const std::string sqlVerify = "SELECT "
        + UserDataBaseTable::columnUserPassword
        + " FROM "
        + UserDataBaseTable::tableName
        + " WHERE "
        + UserDataBaseTable::columnUserLogin
        + " = ?;"
        ;

    int rc = sqlite3_prepare_v2(db, sqlVerify.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cout << "[SQL][User][verifyPassword] prepare error: "<< sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(
        stmt,
        1,
        user.login.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    bool ok = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* dbPassword =
            sqlite3_column_text(stmt, 0);

        if (dbPassword)
        {
            std::string dbPasswordStr =
                reinterpret_cast<const char*>(dbPassword);

            ok = (dbPasswordStr == user.passwordHash);
        }
    }
    else
    {
        std::cout << "[SQL][User][verifyPassword] user not found\n";
    }

    sqlite3_finalize(stmt);
    return ok;
}