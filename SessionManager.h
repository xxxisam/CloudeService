#pragma once
#include <string>
#include <iostream>

namespace SessionManager {
    struct Token
    {
        unsigned int id = 0;
        std::string token = "";
        unsigned int userId = 0;
        std::string userLogin = "";
        unsigned int expires = 0;     // UNIX time
        bool isAdmin = false;
        bool isGuest = false;
        int issuedAt = 0;
        std::string ipAddress = "";
        std::string userAgent = "";
        unsigned int lastUsed = 0;
    };

    //class SessionManager
    //{
    //public:
    //    static SessionManager& getInstance();
    //
    //    Token createToken(
    //        unsigned userId,
    //        const std::string& login,
    //        bool isAdmin,
    //        bool isGuest,
    //        const std::string& ip,
    //        const std::string& userAgent,
    //        unsigned ttlSeconds
    //    );
    //
    //    std::optional<Token> find(const std::string& token);
    //
    //    void remove(const std::string& token);
    //
    //    void cleanupExpired(unsigned int  now);
    //    std::string generateToken();
    //
    //private:
    //    SessionManager() = default;
    //
    //    //std::string generateToken();
    //
    //private:
    //    std::unordered_map<std::string, Token> m_activeTokens;
    //    std::mutex m_mutex;
    //};

    inline void printTokenInfo(const Token& token)
    {
        std::cout << "\n\n\n[START]-------------------------[TOKEN INFO]-------------------------\n";
        std::cout << "[SessionManager][printTokenInfo] token ID: " << token.id << "\n";
        std::cout << "[SessionManager][printTokenInfo] token string: " << token.token << "\n";
        std::cout << "[SessionManager][printTokenInfo] user ID: " << token.userId << "\n";
        std::cout << "[SessionManager][printTokenInfo] user login: " << token.userLogin << "\n";
        std::cout << "[SessionManager][printTokenInfo] expires: " << token.expires << "\n";
        std::cout << "[SessionManager][printTokenInfo] is admin: " << (token.isAdmin ? "true" : "false") << "\n";
        std::cout << "[SessionManager][printTokenInfo] is guest: " << (token.isGuest ? "true" : "false") << "\n";
        std::cout << "[SessionManager][printTokenInfo] issued at: " << token.issuedAt << "\n";
        std::cout << "[SessionManager][printTokenInfo] IP address: " << token.ipAddress << "\n";
        std::cout << "[SessionManager][printTokenInfo] user agent: " << token.userAgent << "\n";
        std::cout << "[SessionManager][printTokenInfo] last used: " << token.lastUsed << "\n";
        std::cout << "\n[END]-------------------------[TOKEN INFO]-------------------------\n\n\n";
    }
}
