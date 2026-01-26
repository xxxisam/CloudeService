#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <iostream>
//#include <boost/asio.hpp>

namespace User
{
    struct User {
        size_t id = 0;
        std::string userName = "";
        //std::string ip;
        std::string login = "";
        std::string passwordHash = "";
        bool isAdmin = false;
        std::vector<std::string> files;

        /* nlohmann::json toJson() const {
             return {
                 {"id", id},
                 {"ip", ip},
                 {"username", userName},
                 {"passwordHash", passwordHash},
                 {"files", files}
            };
         }

          User fromJson(const nlohmann::json& j) {
             User u;
             u.id = j.at("id").get<size_t>();
             u.ip = j.at("ip").get<std::string>();
             u.userName = j.at("username").get<std::string>();
             u.email = j.at("email").get<std::string>();
             u.passwordHash = j.at("passwordHash").get<std::string>();
             u.files = j.at("files").get<std::vector<std::string>>();
             return u;
         }*/
    };

    inline User jsonToUser(const nlohmann::json& json)
    {
        User user;
        user.login = json.at("login").get<std::string>();
        user.userName = user.login;
        user.passwordHash = json.at("password").get<std::string>();
        user.isAdmin = (user.login == "admin");

        std::cout << "login " << user.login << "\n";
        std::cout << "PASSWORD " << user.passwordHash << "\n";
        std::cout << "ADMIN " << user.isAdmin << "\n";

        return user;
    }
}