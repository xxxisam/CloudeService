#pragma once

#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

struct FileMetaInfo
{
    std::string name;        // имя файла
    std::string extension;   // расширение
    std::string fullName;    // полное имя
    std::uintmax_t size;     // размер
    std::string hash;        // SHA-256     


};

inline void to_json(nlohmann::json& j, const FileMetaInfo& f)
{
    j = nlohmann::json{
        {"name", f.name},
        {"extension", f.extension},
        {"fullName", f.fullName},
        {"size", f.size},
        {"hash", f.hash}
    };
}

inline void from_json(const nlohmann::json& j, FileMetaInfo& f)
{
    j.at("name").get_to(f.name);
    j.at("extension").get_to(f.extension);
    j.at("fullName").get_to(f.fullName);
    j.at("size").get_to(f.size);
    j.at("hash").get_to(f.hash);
}