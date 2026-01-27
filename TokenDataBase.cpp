#include "TokenDataBase.h"
#include "defaultPath.h"
#include "UserDataBase.h"

TokenDataBase::TokenDataBase() : m_tokenDataBase(nullptr)
{

};

TokenDataBase& TokenDataBase::getInstance()
{
	static TokenDataBase instance;
	return instance;
}

void TokenDataBase::start()
{
    openTokenSQL();
    if (isSQLTableExist(TokenDataBaseTable::tableName))
    {
        std::cout << "[SQL][Token][start] 1.Token Table is exist!\n";
        printTokenTable();
    }
    else
    {
        if (createTokenTable())
        {
            std::cout << "[SQL][Token][start] 1.TokenTable is been created!\n";
        }
        if (isSQLTableExist(TokenDataBaseTable::tableName))
        {
            std::cout << "[SQL][Token][start] 2. Token Table is exist!\n";
        }
    }

}

sqlite3* TokenDataBase::getTokenDataBase()
{
    return m_tokenDataBase.get();
}

bool TokenDataBase::createTokenTable()
{
    sqlite3* db = getTokenDataBase();

    if (!db)
    {
        std::cout << "[SQL][Token][createTokenTable] - No db\n";
        return false;
    }

    char* uErr = nullptr;
    int uRc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &uErr);
    if (uRc != SQLITE_OK)
    {
        std::cerr << "[SQL][Token] Cannot enable foreign keys: " << (uErr ? uErr : "unknown") << "\n";
        sqlite3_free(uErr);
        return false;
    }

    const std::string sqlCreateTokenTable =
        "CREATE TABLE IF NOT EXISTS " + TokenDataBaseTable::tableName + " ( "
        + TokenDataBaseTable::columnID + " INTEGER PRIMARY KEY AUTOINCREMENT, "
        + TokenDataBaseTable::columnToken + " TEXT UNIQUE NOT NULL, "
        + TokenDataBaseTable::columnUserId + " INTEGER NOT NULL, "
        + TokenDataBaseTable::columnUserLogin + " TEXT, "
        + TokenDataBaseTable::columnExpires + " INTEGER NOT NULL, "
        + TokenDataBaseTable::columnIsAdmin + " INTEGER, "
        + TokenDataBaseTable::columnIsGuest + " INTEGER, "
        + TokenDataBaseTable::columnIssuedAt + " INTEGER, "
        + TokenDataBaseTable::columnIPAddress + " TEXT, "
        + TokenDataBaseTable::columnUserAgent + " TEXT, "
        + TokenDataBaseTable::columnLastUsed + " INTEGER, "
        + "FOREIGN KEY(" + TokenDataBaseTable::columnUserId + ") REFERENCES "
        + UserDataBaseTable::tableName + "(" + UserDataBaseTable::columnID + ") "
        + "ON DELETE CASCADE"
        + ");";

    char* fdbec = nullptr;
    int rc = sqlite3_exec(db, sqlCreateTokenTable.c_str(), nullptr, nullptr, &fdbec);
    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][Token] createTokenTable error: "
            << (fdbec ? fdbec : "unknown") << "\n";
        sqlite3_free(fdbec);
        return false;
    }

    return true;
}

DataBaseUtility::UserDBResult TokenDataBase::addTokenInTable(const SessionManager::Token& token)
{
    std::cout << "[SQL][TOKEN][addTokenInTable] isThisTokenExisted() \n";
    if (!isThisTokenExisted(token.token))
    {
        std::cout << "[SQL][TOKEN][addTokenInTable] This token doesnt existed \n";
        std::cout << "[SQL][TOKEN][addTokenInTable] insertTokenInfoInTable(token); \n";
        insertTokenInfoInTable(token);
        return DataBaseUtility::UserDBResult::OK;
    }
    else
    {
        std::cout << "[SQL][TOKEN][addTokenInTable] Tsis token already existed \n";
        return DataBaseUtility::UserDBResult::TOKEN_ALREADY_EXIST;
    }

}

void TokenDataBase::openTokenSQL()
{
    std::cout << "[SQL][Token][openTokenSQL] opening sql Token table\n";
    sqlite3* sqlOpenPointer = nullptr;

    if (!std::filesystem::exists(defPath::defaultPath::tokenDataBase))
    {
        std::cout << "[SQL][Token][openTokenSQL] db doest't exist" << "\n";
    }

    int errorCodeUserDataBase = sqlite3_open(defPath::defaultPath::tokenDataBase.string().c_str(), &sqlOpenPointer);
    std::cout << "[SQL][Token][openTokenSQL] DB path: " << defPath::defaultPath::tokenDataBase << "\n";

    if (errorCodeUserDataBase != SQLITE_OK)
    {
        std::cerr << "[SQL][Token][openTokenSQL] Ошибка открытия базы: " << sqlite3_errmsg(sqlOpenPointer) << "\n";
        if (m_tokenDataBase)
        {
            sqlite3_close(sqlOpenPointer);
        }
        sqlOpenPointer = nullptr;
    }

    m_tokenDataBase.reset(sqlOpenPointer);
}

bool TokenDataBase::isSQLTableExist(const std::string& tableName)
{
    sqlite3* db = getTokenDataBase();
    if (!db)
    {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const std::string sqlSelectName = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";

    if (sqlite3_prepare_v2(db, sqlSelectName.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][Token][isSQLTableExist] - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
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

SessionManager::Token TokenDataBase::getTokenByLogin(SessionManager::Token& token)
{

    SessionManager::Token result; // id == 0 → не найден

    sqlite3* db = getTokenDataBase();
    if (!db)
    {
        std::cout << "[SQL][Token][getTokenByToken] No DB\n";
        return result;
    }

    const std::string sqlGetToken =
        "SELECT "
        + TokenDataBaseTable::columnID + ", "
        + TokenDataBaseTable::columnToken + ", "
        + TokenDataBaseTable::columnUserId + ", "
        + TokenDataBaseTable::columnUserLogin + ", "
        + TokenDataBaseTable::columnExpires + ", "
        + TokenDataBaseTable::columnIsAdmin + ", "
        + TokenDataBaseTable::columnIsGuest + ", "
        + TokenDataBaseTable::columnIssuedAt + ", "
        + TokenDataBaseTable::columnIPAddress + ", "
        + TokenDataBaseTable::columnUserAgent + ", "
        + TokenDataBaseTable::columnLastUsed +
        " FROM " + TokenDataBaseTable::tableName +
        " WHERE " + TokenDataBaseTable::columnUserLogin + " = ? "
        " LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlGetToken.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cout << "[SQL][Token][getTokenByToken] Prepare error: "
            << sqlite3_errmsg(db) << "\n";
        return result;
    }

    sqlite3_bind_text(
        stmt,
        1,
        token.token.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        result.id = sqlite3_column_int64(stmt, 0);
        result.token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        result.userId = sqlite3_column_int64(stmt, 2);
        result.userLogin = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        result.expires = sqlite3_column_int64(stmt, 4);
        result.isAdmin = sqlite3_column_int(stmt, 5) != 0;
        result.issuedAt = sqlite3_column_int64(stmt, 6);

        const char* ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        if (ip) result.ipAddress = ip;

        const char* ua = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        if (ua) result.userAgent = ua;

        result.lastUsed = sqlite3_column_int64(stmt, 9);

        std::cout << "[SQL][Token][getTokenByToken] Token loaded: "
            << result.token << "\n";
    }

    sqlite3_finalize(stmt);
    return result;
}

void TokenDataBase::printTokenTable()
{
    sqlite3* db = getTokenDataBase();

    if (!db) { std::cout << "[SQL][TOKEN][printTokenTable] db No such table!"; }

    sqlite3_stmt* stmt = nullptr;

    std::string sqlPrint =
        "SELECT "
        + TokenDataBaseTable::columnID + ", "
        + TokenDataBaseTable::columnToken + ", "
        + TokenDataBaseTable::columnUserId + ", "
        + TokenDataBaseTable::columnUserLogin + ", "
        + TokenDataBaseTable::columnExpires + ", "
        + TokenDataBaseTable::columnIsAdmin + ", "
        + TokenDataBaseTable::columnIsGuest 
        + " FROM " + TokenDataBaseTable::tableName + ";";

    if (sqlite3_prepare_v2(db, sqlPrint.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][TOKEN][printTokenTable] - Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "[TOKEN]|id | token | userID | userLogin | expires | isAdmin | isGuest |" << "\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char* token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int userId = sqlite3_column_int(stmt, 2);
        const char* userLogin = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        sqlite3_int64 expires = sqlite3_column_int64(stmt, 4);
        bool isAdmin = sqlite3_column_int(stmt, 5);
        bool isGuest = sqlite3_column_int(stmt, 6);

        std::cout
            << std::boolalpha << id << " | "
            << (token ? token : "NULL") << " | "
            << userId << " | "
            << (userLogin ? userLogin : "NULL") << " | "
            << expires << " | "
            << isAdmin << " | "
            << isGuest << "\n";
    }

    std::cout << "_____________________________________________________________________________________\n";

    sqlite3_finalize(stmt);
}

bool TokenDataBase::insertTokenInfoInTable(const SessionManager::Token& token)
{
    sqlite3* db = getTokenDataBase();
    if (!db) return false;

    const std::string sqlInsetToken =
        "INSERT INTO " + TokenDataBaseTable::tableName + " ("
        + TokenDataBaseTable::columnToken + ", "
        + TokenDataBaseTable::columnUserId + ", "
        + TokenDataBaseTable::columnUserLogin + ", "
        + TokenDataBaseTable::columnExpires + ", "
        + TokenDataBaseTable::columnIsAdmin + ", "
        + TokenDataBaseTable::columnIsGuest + ", "
        + TokenDataBaseTable::columnIssuedAt + ", "
        + TokenDataBaseTable::columnIPAddress + ", "
        + TokenDataBaseTable::columnUserAgent + ", "
        + TokenDataBaseTable::columnLastUsed +
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlInsetToken.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[SQL][Token] prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, token.token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, token.userId);
    sqlite3_bind_text(stmt, 3, token.userLogin.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, token.expires);
    sqlite3_bind_int(stmt, 5, int(token.isAdmin));
    sqlite3_bind_int(stmt, 6, int(token.isGuest));
    sqlite3_bind_int64(stmt, 7, token.issuedAt);
    sqlite3_bind_text(stmt, 8, token.ipAddress.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, token.userAgent.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 10, token.lastUsed);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    std::cout << "[SQL][Token] Token inserted into DB\n";
    return ok;
}

bool TokenDataBase::deleteToken(const SessionManager::Token& token)
{
    sqlite3* db = getTokenDataBase();

    if (!db)
    {
        std::cout << "[SQL][Token][deleteToken] db doesnt exist\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
  
    std::string sqlDeleteToken = "DELETE FROM " + TokenDataBaseTable::tableName + " WHERE " + TokenDataBaseTable::columnToken + " = ?;";

    int rc = sqlite3_prepare_v2(db, sqlDeleteToken.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "[SQL][Token][Delete Prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, token.token.c_str(), -1, SQLITE_TRANSIENT);

    int brc = sqlite3_step(stmt);

    if (brc != SQLITE_DONE)
    {
        std::cerr << "[SQL][Token][Delete error: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    int deletedRows = sqlite3_changes(db);
    std::cout << "[SQL][Token][deleteToken]__Deleted rows :" << deletedRows << "\n";
    sqlite3_finalize(stmt);


    return deletedRows >  0;
}

bool TokenDataBase::getLogin(const SessionManager::Token& token)
{

    sqlite3* db = getTokenDataBase();
    if (!db)
    {
        std::cout << "[SQL][Token][getLogin] db doesnt exist\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    std::string sqlGetLogin = "SELECT " + TokenDataBaseTable::columnUserLogin + " WHERE " + TokenDataBaseTable::columnToken + "=?;";

    int prerr = sqlite3_prepare_v2(db, sqlGetLogin.c_str(), 1, &stmt, nullptr);
    if (prerr != SQLITE_OK)
    {
        std::cerr << "[SQL][Token][getLogin] Prepare error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    int lerr = sqlite3_bind_text(stmt, 1, token.token.c_str(), -1, SQLITE_TRANSIENT);

    if (lerr != SQLITE_ROW)
    {
        std::cerr << "[SQL][Token][getLogin] error: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool TokenDataBase::isThisTokenExisted(const std::string& token)
{
    sqlite3* db = getTokenDataBase();
    if (!db)
    {
        std::cout << "[SQL][Token][TokenDataBase]No such db!\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    std::string sqlIsTokenExist = "SELECT 1 FROM " + TokenDataBaseTable::tableName + " WHERE " + TokenDataBaseTable::columnToken + " = ? LIMIT 1";

    int stmterr = sqlite3_prepare_v2(db, sqlIsTokenExist.c_str(), -1, &stmt, nullptr);

    if (stmterr != SQLITE_OK)
    {
        std::cerr << "[SQL][Token][isThisTokenExisted] error: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    std::cout << "[SQL][Token][isThisTokenExisted] exist: " << std::boolalpha << exists << "\n";

    sqlite3_finalize(stmt);
    return exists;
}

bool TokenDataBase::takeTokenInfo(SessionManager::Token& token)
{
    std::cout << "[SQL][Token][takeTokenInfo]Searched token" << token.token << "\n";
    sqlite3* db = getTokenDataBase();
    if (!db)
    {
        std::cout << "[SQL][Token][takeTokenInfo]No such db!\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    std::string sqltakeTokenInfo = "SELECT " + TokenDataBaseTable::columnUserLogin + ", " + TokenDataBaseTable::columnIsAdmin + ", " + TokenDataBaseTable::columnIsGuest + " FROM " + TokenDataBaseTable::tableName + " WHERE " + TokenDataBaseTable::columnToken + " = ? LIMIT 1";

    int prepErr = sqlite3_prepare_v2(db, sqltakeTokenInfo.c_str(), -1, &stmt, nullptr);
    if (prepErr != SQLITE_OK)
    {
        std::cerr << "[SQL][Token][takeTokenInfo] error prepare: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, token.token.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE)
    {
        std::cerr << "[SQL][Token][takeTokenInfo] error token not found\n";
        sqlite3_finalize(stmt);
        return false;
    }
    else if (rc != SQLITE_ROW)
    {
        std::cerr << "[SQL][Token][takeTokenInfo] error bind: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    const unsigned char* login = sqlite3_column_text(stmt, 0);
    if (login)
    {
        token.userLogin = std::string(reinterpret_cast<const char*>(login));
    }
    else
    {
        token.userLogin = "";
    }
    token.isAdmin = sqlite3_column_int(stmt, 1);
    token.isGuest = sqlite3_column_int(stmt, 2);

    std::cout << "[SQL][Token][takeTokenInfo] ADMIN" << std::boolalpha << token.isAdmin << "\n";
    std::cout << "[SQL][Token][takeTokenInfo] GUEST" << std::boolalpha << token.isGuest << "\n";
    SessionManager::printTokenInfo(token);
    sqlite3_finalize(stmt);
    return true;
}

DataBaseUtility::UserDBResult TokenDataBase::delToken(const SessionManager::Token& token)
{
    std::cout << "[SQL][TOKEN][delToken] findByLoginIfExist() \n";
    if (isThisTokenExisted(token.token))
    {
        std::cout << "[SQL][TOKEN][delToken] This token exist\n";
        std::cout << "[SQL][TOKEN][delToken]deleteToken(token); \n";
        deleteToken(token);
        return DataBaseUtility::UserDBResult::OK;
    }
    else
    {
        std::cout << "[SQL][TOKEN][delToken] TOKEN_DOESNT_EXIST \n";
        return DataBaseUtility::UserDBResult::TOKEN_DOESNT_EXIST;
    }
}

bool TokenDataBase::findByTokenIfExistByLogin(const SessionManager::Token& token)
{
    sqlite3* db = getTokenDataBase();
    if (!db)
    {
        std::cout << "[SQL][Token][findByTokenIfExist] No DB\n";
        return false;
    }

    sqlite3_stmt* stmt = nullptr;

    const std::string sql =
        "SELECT 1 FROM " + TokenDataBaseTable::tableName +
        " WHERE " + TokenDataBaseTable::columnUserLogin + " = ? LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cout << "[SQL][Token][findByTokenIfExist] Prepare error: "
            << sqlite3_errmsg(db) << "\n";
        return false;
    }

    // bind token string
    sqlite3_bind_text(stmt,1,token.userLogin.c_str(), -1, SQLITE_TRANSIENT );

    bool exists = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        exists = true;
    }

    sqlite3_finalize(stmt);

    std::cout << "[SQL][Token][findByTokenIfExist] token=" << token.token << " exists=" << exists << "\n";

    return exists;
}
