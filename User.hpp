#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
//#include <boost/asio.hpp>

struct User {
    size_t id = 0;
    std::string username;
    std::string ip;
    std::string email;
    std::string passwordHash;
    std::vector<std::string> files;

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"ip", ip},
            {"username", username},
            {"email", email},
            {"passwordHash", passwordHash},
            {"files", files}
        };
    }

     User fromJson(const nlohmann::json& j) {
        User u;
        u.id = j.at("id").get<size_t>();
        u.ip = j.at("ip").get<std::string>();
        u.username = j.at("username").get<std::string>();
        u.email = j.at("email").get<std::string>();
        u.passwordHash = j.at("passwordHash").get<std::string>();
        u.files = j.at("files").get<std::vector<std::string>>();
        return u;
    }
};