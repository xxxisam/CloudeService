#pragma once
#include <iostream>
#include "MultipartParser.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "File.hpp"

class Parse
{
private:
    static void
        onPartBegin(const char* buffer, size_t start, size_t end, void* userData) {
        printf("onPartBegin\n");
    }

    static void
        onHeaderField(const char* buffer, size_t start, size_t end, void* userData) {
        printf("onHeaderField: (%s)\n", std::string(buffer + start, end - start).c_str());
    }

    static void
        onHeaderValue(const char* buffer, size_t start, size_t end, void* userData) {
        printf("onHeaderValue: (%s)\n", std::string(buffer + start, end - start).c_str());
    }

    static void
        onPartData(const char* buffer, size_t start, size_t end, void* userData) {
        File::receivedPacket* fileInfo = static_cast<File::receivedPacket*>(userData);
        std::string part(buffer + start, end - start);

        if (!fileInfo->metaParsed) {
            try {
                auto j = nlohmann::json::parse(part);
                fileInfo->u_name = j.at("name").get<std::string>();
                fileInfo->u_fullName = j.at("fullName").get<std::string>();
                fileInfo->u_extension = j.at("extension").get<std::string>();
                fileInfo->u_size = j.at("size").get<std::uintmax_t>();
                fileInfo->u_hash = j.at("hash").get<std::string>();
                fileInfo->u_packetCount = j.at("packetCount").get<size_t>();
                fileInfo->u_packetID = j.at("packetID").get<size_t>();
                fileInfo->metaParsed = true;
                
            }
            catch (std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        }
        else {
            fileInfo->u_packet.insert(fileInfo->u_packet.end(), buffer + start, buffer + end);
        }
    }

    static void
        onPartEnd(const char* buffer, size_t start, size_t end, void* userData) {
        printf("onPartEnd\n");
    }


    static void
        onEnd(const char* buffer, size_t start, size_t end, void* userData) {
        printf("onEnd\n");
    }

    static void onHeaderEnd(const char* buffer, size_t start, size_t end, void* userData) {
        std::cout << "Header end" << std::endl;
    }
public:
    void parse(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request, File::receivedPacket& fileInfo)
    {
        fileInfo.metaParsed = false;

        //parser.onPartBegin = Parse::onPartBegin;
        //parser.onHeaderField = Parse::onHeaderField;
        //parser.onHeaderValue = Parse::onPartData;
        //parser.onPartData = Parse::onPartEnd;
        //parser.onPartEnd = Parse::onEnd;
        //parser.onEnd = Parse::onHeaderEnd;
        //parser.userData = &parser;

        MultipartParser parser;
        parser.onPartData = Parse::onPartData;
        parser.userData = &fileInfo;

        std::string type = std::string((*request)[boost::beast::http::field::content_type]);
        auto pos = type.find("boundary=");
        if (pos != std::string::npos) {
            std::string boundary = type.substr(pos + 9);
            parser.setBoundary(boundary);
        }

        auto& body = request->body();
        std::string bodyStr = boost::beast::buffers_to_string(body.data());
        parser.feed(bodyStr.c_str(), bodyStr.size());

        /*std::cout << "File name: " << fileInfo.u_name << "\n";
        std::cout << "File size: " << fileInfo.u_size << "\n";
        std::cout << "Binary size: " << fileInfo.u_packet.size() << "\n";
        std::cout << "Binary hash: " << fileInfo.u_hash << "\n";
        std::cout << "Count of packets: " << fileInfo.u_packetCount << "\n";
        std::cout << "Packet ID: " << fileInfo.u_packetID << "\n";*/

        

        //// boundary из запроса
        //std::string boundary = std::string((*request)[boost::beast::http::field::content_type]);
        //
        //
        //auto pos = boundary.find("boundary=");
        //if (pos != std::string::npos) {
        //    std::string boundary = boundary.substr(pos + 9);
        //    parser.setBoundary(boundary);
        //}
    }
};

