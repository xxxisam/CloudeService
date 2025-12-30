#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include "defaultPath.h"


namespace File {

    struct tempFileData
    {
        std::string temp_name;
        std::string temp_extension;
        std::string temp_fullName;
        std::uintmax_t temp_size;
        std::string temp_hash;
        size_t temp_packetCount; 
        size_t temp_packetID;
        std::map <size_t, std::vector<char>> temp_packets;
        bool isFileReceived = false;
    };

    struct receivedPacket
    {
        std::string u_name;
        std::string u_extension;
        std::string u_fullName;
        std::uintmax_t u_size;
        std::string u_hash;
        std::vector<char> u_packet;
        size_t u_packetCount; 
        size_t u_packetID;
        bool metaParsed = false;
    };

    struct FileMetaInfo
    {
        std::string name;        // имя файла
        std::string extension;   // расширение
        std::string fullName;    // полное имя
        std::uintmax_t size;     // размер
        std::string hash;        // SHA-256     
        bool isEmpty;
        bool isFileComplete;
        std::filesystem::path fileServerPath{};

    };

    inline FileMetaInfo toFileMetaInfo(receivedPacket uploadFile)
    {
        FileMetaInfo fmi;
        fmi.name = uploadFile.u_name;
        fmi.extension = uploadFile.u_extension;
        fmi.fullName = uploadFile.u_fullName;
        fmi.size = uploadFile.u_size;
        fmi.hash = uploadFile.u_hash;
        return fmi;
    }

    inline void FirstPacketToTempFileData(const receivedPacket& rp, tempFileData& tfd)
    {
        tfd.temp_name = rp.u_name;
        tfd.temp_extension = rp.u_extension;
        tfd.temp_fullName = rp.u_fullName;
        tfd.temp_size = rp.u_size;
        tfd.temp_hash = rp.u_hash;
        tfd.temp_packetCount= rp.u_packetCount;

        //appendPacketToTempFileData(rp, tfd);
    }

    inline void appendPacketToTempFileData(const receivedPacket& rp, tempFileData& tfd)
    {
        //if()
        std::cout << "tfd.temp_packets[tfd.temp_packetID] = rp.u_packet: " << rp.u_packetID << "\n";
        tfd.temp_packets[rp.u_packetID] = rp.u_packet;
        std::cout << "Size tfd " << tfd.temp_packets.size() << "\n";
    }

    inline void transformInOneFile(const tempFileData& tfd, std::vector<char>& binFile)
    {
        for (const auto& it : tfd.temp_packets)
        {
            binFile.insert(binFile.end(), it.second.begin(), it.second.end());
        }
    }

    inline void printFileMetaInfo(FileMetaInfo fmi)
    {
        std::cout << "name:" << fmi.name << "\n";
        std::cout << "extension:" << fmi.extension << "\n";
        std::cout << "fullName:" << fmi.fullName << "\n";
        std::cout << "size:" << fmi.size << "\n";
        std::cout << "hash:" << fmi.hash << "\n";
        std::cout << "File Path " << fmi.fileServerPath << "\n";
    }

    //convertion to/from JSON
    inline void to_json(nlohmann::json& j, const FileMetaInfo& f)
    {
        j = nlohmann::json{
            {"name", f.name},
            {"extension", f.extension},
            {"fullName", f.fullName},
            {"size", f.size},
            {"hash", f.hash},
            {"fileServerPath", f.fileServerPath}
        };
    }

    inline void from_json(const nlohmann::json& j, FileMetaInfo& f)
    {
        j.at("name").get_to(f.name);
        j.at("extension").get_to(f.extension);
        j.at("fullName").get_to(f.fullName);
        j.at("size").get_to(f.size);
        j.at("hash").get_to(f.hash);
        j.at("fileServerPath").get_to(f.fileServerPath);
    }

    //write/read JSON File
    inline void writeJSONCompleteFile(nlohmann::json& j)
    {
        std::filesystem::path jsonFile = defPath::defaultPath::jsonFolder / "CompleteFiles.json";

        std::ofstream ofs(jsonFile, std::ios::app);
        ofs << j.dump(4);

        ofs.close();
    }

    inline void readJSONCompleteFile(nlohmann::json& j)
    {
        std::filesystem::path jsonFile = defPath::defaultPath::jsonFolder / "CompleteFiles.json";
        std::ifstream ifs(jsonFile);
        
        ifs >> j;

        ifs.close();
    }

    inline void writeJSONnotCompleteFile(nlohmann::json& j)
    {
        std::filesystem::path jsonFile = defPath::defaultPath::jsonFolder / "notCompleteFiles.json";

        std::ofstream ofs(jsonFile);
        ofs << j.dump(4);

        ofs.close();
    }

    inline void readJSONnotCompleteFile(nlohmann::json& j)
    {
        std::filesystem::path jsonFile = defPath::defaultPath::jsonFolder / "notCompleteFiles.json";
        std::ifstream ifs(jsonFile);

        ifs >> j;

        ifs.close();
    }


    //json Operations
    inline void addJSONToCompleteFile()
    {

    }

    /*inline std::string normalizePath(const std::string& path)
    {
        std::string result = path;
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }*/
}