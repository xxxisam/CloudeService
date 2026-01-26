#pragma once
#include "SessionManager.h"

#include <CryptoPP/osrng.h>
#include <CryptoPP/hex.h>
#include <CryptoPP/filters.h>

//SessionManager& SessionManager::getInstance()
//{
//    static SessionManager instance;
//    return instance;
//}
//
//std::string SessionManager::generateToken()
//{
//    CryptoPP::AutoSeededRandomPool asrp;
//    CryptoPP::byte bytes[32];
//    asrp.GenerateBlock(bytes, sizeof(bytes));
//
//    std::string token;
//    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(token));
//    encoder.Put(bytes, sizeof(bytes));
//    encoder.MessageEnd();
//
//    return token;
//}
//
//Token SessionManager::createToken(
//    int64_t userId,
//    const std::string& login,
//    bool isAdmin,
//    bool isGuest,
//    const std::string& ip,
//    const std::string& userAgent,
//    int64_t ttlSeconds)
//{
//    Token token;
//
//    const int64_t now = std::time(nullptr);
//
//    token.id = 0;
//    token.token = generateToken();
//    token.userId = userId;
//    token.userLogin = login;
//    token.isAdmin = isAdmin;
//    token.isAdmin = isGuest;
//    token.issuedAt = now;
//    token.expires = now + ttlSeconds;
//    token.ipAddress = ip;
//    token.userAgent = userAgent;
//    token.lastUsed = now;
//
//    {
//        std::lock_guard<std::mutex> lock(m_mutex);
//        m_activeTokens[token.token] = token;
//    }
//
//    std::cout << "[SessionManager] Token stored in RAM: " << token.token << "\n";
//
//    return token;
//}
//
//void SessionManager::remove(const std::string& token)
//{
//    std::lock_guard lock(m_mutex);
//    m_activeTokens.erase(token);
//}
//
//void SessionManager::cleanupExpired(int64_t now)
//{
//    std::lock_guard lock(m_mutex);
//
//    for (auto it = m_activeTokens.begin(); it != m_activeTokens.end(); )
//    {
//        if (it->second.expires < now)
//            it = m_activeTokens.erase(it);
//        else
//            ++it;
//    }
//}
//
//std::optional<Token> SessionManager::find(const std::string& tokenStr)
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//
//    auto it = m_activeTokens.find(tokenStr);
//    if (it == m_activeTokens.end())
//        return std::nullopt;
//
//    const int64_t now = std::time(nullptr);
//    if (it->second.expires < now)
//    {
//        m_activeTokens.erase(it);
//        return std::nullopt;
//    }
//
//    it->second.lastUsed = now;
//    return it->second;
//}

