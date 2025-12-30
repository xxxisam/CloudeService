#pragma once
#include <memory>
#include <boost/asio.hpp>
#include "User.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <filesystem>
#include <fstream>
#include <string>
//#include <sstream>
#include "File.hpp"
#include "defaultPath.h"

#include "Parse.h"
#include <vector>
#include "FileDataBase.h"
#include "sqlDataBaseRequest.h"


class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket socket) : m_socket(std::move(socket)), fileDB(FileDataBase::getInstance())
    {

    }

    void start()
    {
        fileDB.start();
        try
        {
            std::cout << "getRequest\n";
            getRequest();
        }
        catch (const std::exception& ec)
        {
            std::cerr << "start Session error: " << ec.what() << "\n";
        }
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    std::shared_ptr<User> m_user;
    std::filesystem::path m_serverFolder;
    FileDataBase& fileDB;

    //boost::beast::flat_buffer m_buffer;
    //boost::beast::http::request<boost::beast::http::string_body> m_request;
    std::unordered_map<std::string, std::shared_ptr<File::FileMetaInfo>> m_files;
    std::unordered_map<std::string, std::shared_ptr<File::tempFileData>> m_upload_files;
    nlohmann::json m_JSONUploadFiles = nlohmann::json::array();
    //std::vector<std::string> hash_of_uploading_files;
    FileDataBase* filedb;





    void getRequest()
    {
        std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request = std::make_shared<boost::beast::http::request<boost::beast::http::dynamic_body>>();
        std::shared_ptr<boost::beast::flat_buffer> buffer = std::make_shared<boost::beast::flat_buffer>();
        std::shared_ptr<boost::beast::http::request_parser<boost::beast::http::dynamic_body>> parser = std::make_shared<boost::beast::http::request_parser<boost::beast::http::dynamic_body>>();
        parser->body_limit(10 * 1024 * 1024);

        boost::beast::http::async_read(
            m_socket,
            *buffer,
            *parser,
            [self = shared_from_this(), request, buffer, parser](const boost::system::error_code& ec, std::size_t)
            {
                if (ec)
                {
                    std::cerr << "getRequest async_read Error: " << ec.message() << "\n";
                    return;
                }
                std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request = std::make_shared<boost::beast::http::request<boost::beast::http::dynamic_body>>(parser->get());
                self->processRequest(request);
            }
        );
    }

    void processRequest(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::string target = std::string(request->target());
        boost::beast::http::verb method = request->method();


        std::cout << target << "\n";

        if (target.empty())
        {
            target = "/";
        }

        if (method == boost::beast::http::verb::get)
        {
            if (target == "/list")
            {
                pressRefreshButton(request);
            }
            else if (target.substr(0, 9) == "/download")
            {
                downloadFile(request);
            }
            else
            {
                loadMainPage(request);
            }
        }
        else if (method == boost::beast::http::verb::post)
        {
            if (target == "/upload")
            {
                upload(request);
            }
        }
        else if (method == boost::beast::http::verb::delete_)
        {
            if (target.substr(0, 7) == "/delete")
            {
                deleteFile(request);
            }
        }
        
        /*else if (request->method() == boost::beast::http::verb::post ||
            request->target() == "/upload/meta")
        {
            uploadFileMetaInfo(request);
        }*/
        else
        {
            std::cout << "Неверный вид запроса: " << target << "\n";
            std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::not_found, request->version()
            );
            res->body() = "Not found";
            res->prepare_payload();

            boost::beast::http::async_write(
                m_socket,
                *res,
                [self = shared_from_this(), res, request](boost::system::error_code ec, std::size_t)
                {
                    if (ec)
                    {
                        std::cerr << "async_write error: " << ec.message() << "\n";
                    }
                    else if (res->keep_alive())
                    {
                        self->getRequest();
                    }
                }
            );
        }
    }



    boost::beast::string_view checkMimeType(boost::beast::string_view path)
    {
        using boost::beast::iequals;
        auto const ext = [&path]
            {
                auto const pos = path.rfind(".");
                if (pos == boost::beast::string_view::npos)
                    return boost::beast::string_view{};
                return path.substr(pos);
            }();

        if (iequals(ext, ".htm"))  return "text/html";
        if (iequals(ext, ".html")) return "text/html";
        if (iequals(ext, ".css"))  return "text/css";
        if (iequals(ext, ".txt"))  return "text/plain";
        if (iequals(ext, ".js"))   return "application/javascript";
        if (iequals(ext, ".json")) return "application/json";
        if (iequals(ext, ".xml"))  return "application/xml";
        if (iequals(ext, ".png"))  return "image/png";
        if (iequals(ext, ".jpg"))  return "image/jpeg";
        if (iequals(ext, ".jpeg")) return "image/jpeg";
        if (iequals(ext, ".gif"))  return "image/gif";
        if (iequals(ext, ".bmp"))  return "image/bmp";
        if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
        if (iequals(ext, ".tiff")) return "image/tiff";
        if (iequals(ext, ".tif"))  return "image/tiff";
        if (iequals(ext, ".svg"))  return "image/svg+xml";
        if (iequals(ext, ".svgz")) return "image/svg+xml";

        return "application/octet-stream";
    }

    void loadMainPage(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::string target = std::string(request->target());
        if (target == "/")
            target = "/MainPage.html";

        std::string full_path = "bin" + target;

        boost::beast::http::file_body::value_type body;
        boost::beast::error_code ec;
        body.open(full_path.c_str(), boost::beast::file_mode::scan, ec);

        if (ec)
        {
            std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::not_found, request->version()
            );
            res->set(boost::beast::http::field::content_type, "text/html");
            res->body() = "File not found: " + target;
            res->prepare_payload();

            boost::beast::http::async_write(
                m_socket,
                *res,
                [self = shared_from_this(), res, request](boost::system::error_code ec, std::size_t)
                {
                    if (ec)
                        std::cerr << "async_write Error: " << ec.message() << "\n";
                    else if (res->keep_alive())
                        self->getRequest();
                }
            );
            return;
        }

        unsigned long long size = body.size();
        std::shared_ptr<boost::beast::http::response<boost::beast::http::file_body>> res = std::make_shared<boost::beast::http::response<boost::beast::http::file_body>>(
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(boost::beast::http::status::ok, request->version())
        );

        res->set(boost::beast::http::field::content_type, checkMimeType(full_path));
        res->content_length(size);

        boost::beast::http::async_write(
            m_socket,
            *res,
            [self = shared_from_this(), res, request](boost::system::error_code ec, std::size_t)
            {
                if (ec)
                    std::cerr << "async_write Error: " << ec.message() << "\n";
                else if (res->keep_alive())
                    self->getRequest();
            }
        );
    }

    void upload(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        /*FileMetaInfo file = uploadFileMetaInfo(request);


        std::filesystem::path folder = "D:/S/Server";
        std::filesystem::path filePath = folder / file.fullName;


        auto& body = request->body();
        std::ofstream out(filePath, std::ios::binary);

        for (auto const& buffer : body.data())
            out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        out.close();

        auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
            boost::beast::http::status::ok, request->version()
        );
        res->set(boost::beast::http::field::content_type, "text/plain");
        res->body() = "File saved";
        res->prepare_payload();

        boost::beast::http::async_write(
            m_socket,
            *res,
            [self = shared_from_this(), res, request](boost::system::error_code ec, std::size_t)
            {
                if (ec)
                    std::cerr << "async_write Error: " << ec.message() << "\n";
                else if (res->keep_alive())
                    self->getRequest();
            }
        );*/

        /*std::string boundary = std::string((*request)[boost::beast::http::field::content_type]);
        std::cout << "Boundary -  " << boundary << "\n";*/


        Parse parseObject;
        File::receivedPacket received_packet;
        parseObject.parse(request, received_packet);

        
        std::unordered_map<std::string, std::shared_ptr<File::tempFileData>>::iterator it_SearchFile = m_upload_files.find(received_packet.u_hash);


        if (it_SearchFile != m_upload_files.end())
        {
            std::cout << "Такой пакет уже есть - " << received_packet.u_name << "\n";
            std::cout << it_SearchFile->first << "\n";
            
            
            it_SearchFile->second->temp_packets[received_packet.u_packetID] = received_packet.u_packet;//.insert(uploadFileInfo.u_packet.end(),uploadFileInfo.u_packet.begin(), uploadFileInfo.u_packet.end());

            File::appendPacketToTempFileData(received_packet, *it_SearchFile->second);
            std::cout << "Packet count for test " << received_packet.u_packetCount << "\n";
            std::cout << it_SearchFile->second->temp_packets.size() << "\n";

        }
        else
        {
            File::tempFileData tempFileData;
            std::cout << "Создан файл - " << received_packet.u_name << "\n";
            File::FirstPacketToTempFileData(received_packet,tempFileData);
            
            m_upload_files[tempFileData.temp_hash] = std::make_shared<File::tempFileData>(tempFileData);
            it_SearchFile = m_upload_files.find(received_packet.u_hash);
            File::appendPacketToTempFileData(received_packet, *it_SearchFile->second);

        }

        it_SearchFile->second->isFileReceived = isFullReceive(*it_SearchFile->second, received_packet);
        std::cout << "Проверка: " << it_SearchFile->second->isFileReceived << "\n";
        if (it_SearchFile->second->isFileReceived)
        {
            std::vector<char> completeFile;
            File::transformInOneFile(*it_SearchFile->second, completeFile);
            std::cout << "Файл полностью получен - " << received_packet.u_name << "\n";
            std::cout << "Размер файла: " << completeFile.size() << "\n";

            
            std::filesystem::path filePath = defPath::defaultPath::serverFolder / it_SearchFile->second->temp_fullName;
            std::ofstream out(filePath, std::ios::binary);
            if (!out)
            {
                std::cout << "Error ostream file: \n";
            }
            out.write(completeFile.data(), completeFile.size());


            File::FileMetaInfo fmi = toFileMetaInfo(received_packet);
            try {
                fmi.fileServerPath = std::filesystem::canonical(filePath);
                std::cout << "Canonical path: " << fmi.fileServerPath.string() << "\n";
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "File doesn't exist or error: " << e.what() << "\n";
            }

            //std::cout << "222Canonical path: " << fmi.name << "\n";
            
            filedb = &FileDataBase::getInstance();
            filedb->getRequest(sqlDataBaseRequest::sqlInsert, fmi);
            filedb->getRequest(sqlDataBaseRequest::sqlPrint);
            
            /*nlohmann::json json;
            File::to_json(json, fmi);
            m_JSONUploadFiles.emplace_back(json);
            File::writeJSONCompleteFile(m_JSONUploadFiles);*/


            printFileMetaInfo(fmi);

            m_upload_files.erase(received_packet.u_hash);
        }
        
        std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> res = std::make_shared<
            boost::beast::http::response<boost::beast::http::string_body>
        >(
            boost::beast::http::status::ok,
            request->version()
        );

        res->set(boost::beast::http::field::content_type, "text/plain");
        res->keep_alive(request->keep_alive());
        res->body() = "OK";
        res->prepare_payload();
        std::cout << "_______________________LOG__________________________________\n";
        boost::beast::http::async_write(
            m_socket,
            *res,
            [self = shared_from_this(), res](boost::system::error_code ec, std::size_t)
            {
                if (!ec && res->keep_alive())
                    self->getRequest(); 
            }
        );

    }

   bool isFullReceive(const File::tempFileData& tfd, const File::receivedPacket& rp)
    {
       std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!Проверка вызвана\n";
       std::cout << " Количество пакетов Всего: " << tfd.temp_packetCount << "\n";
       std::cout << " Количество полученных пакетов " << tfd.temp_packets.size() << "\n";
       std::cout << " Размер записываемого файла " << tfd.temp_size << "\n";
       std::cout << " Размер файла " << rp.u_size << "\n";
       std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!Проверка Закончена\n";
       return (tfd.temp_packetCount == tfd.temp_packets.size()) && (tfd.temp_size == rp.u_size);
    }

    void pressRefreshButton(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        nlohmann::json json;

        fileDB.getRequest(sqlDataBaseRequest::sqlSync, json);
        //std::cout << "pressRefreshButton---filesJson[fileServerPath] JSON: " << json["fileServerPath"] << "\n";
        for (const nlohmann::json& file : json)
        {
            std::cout << "pressRefreshButton__fileServerPath = " << file["fileServerPath"] << "\n";
        }
        
        auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
            boost::beast::http::status::ok, request->version()
        );

        fileDB.getRequest(sqlDataBaseRequest::sqlPrint);
        
        res->set(boost::beast::http::field::content_type, "application/json");
        res->body() = json.dump(4);
        res->prepare_payload();

        boost::beast::http::async_write(
            m_socket,
            *res,
            [self = shared_from_this(), res, request](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (ec)
                    std::cerr << "async_write Error: " << ec.message() << "\n";
                else if (res->keep_alive())
                    self->getRequest();
            }
        );
    }

    /*std::string scan_directory_to_json(const std::string& dir_path)
    {
        std::ostringstream json;
        json << "[";
        bool first = true;
        for (const auto& entry : std::filesystem::directory_iterator(defPath::defaultPath::serverFolder))
        {
            if (!entry.is_regular_file()) continue;
            if (!first) json << ",";
            first = false;
            auto path = entry.path();
            auto size = entry.file_size();
            json << "{";
            json << "\"name\":\"" << path.filename().string() << "\",";
            json << "\"size\":" << size << ",";
            json << "\"extension\":\"" << path.extension().string() << "\",";
            json << "\"hash\":\""
                << "71944d7430c461f0cd6e7fd10cee7eb72786352a3678fc7bc0ae3d410f72aece"
                << "\"";
            json << "}";
        }
        json << "]";
        return json.str();
    }*/

    void deleteFile(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::string deletedHash = std::string(request->target()).substr(13);
        std::cout << "Deleted Hash: " << deletedHash << "\n";

        File::FileMetaInfo fmi;
        fmi.hash = deletedHash;
        fileDB.getRequest(sqlDataBaseRequest::sqlFind, fmi);
        std::cout << "fmi servgfoikejgoifjnes: " << fmi.fileServerPath << "\n";

        std::error_code std_ec;
        try
        {
            if (std::filesystem::remove(fmi.fileServerPath))
            {
                fileDB.getRequest(sqlDataBaseRequest::sqlDelete, fmi);
                std::cout << "Файл '" << fmi.fileServerPath << "' успешно удалён.\n";
                auto res = std::make_shared<
                    boost::beast::http::response<boost::beast::http::string_body>
                >(
                    boost::beast::http::status::ok,
                    request->version()
                );

                res->set(boost::beast::http::field::content_type, "text/plain");
                res->keep_alive(request->keep_alive());
                res->body() = "OK";
                res->prepare_payload();
                std::cout << "_______________________LOG__________________________________\n";
                boost::beast::http::async_write(
                    m_socket,
                    *res,
                    [self = shared_from_this(), res](const boost::system::error_code& ec, std::size_t bytes_transferred)
                    {
                        if (!ec && res->keep_alive())
                        {
                            self->getRequest();
                        }
                        else
                        {
                            std::cout << "[Session]__deleteFile -> boost::beast::http::async_write Error\n";
                        }
                    }
                );
            }
            else
            {
                std::cout << "Файл '" << fmi.fileServerPath << "' не найден.\n";

                auto res = std::make_shared<
                    boost::beast::http::response<boost::beast::http::string_body>
                >(boost::beast::http::status::not_found, request->version());

                res->set(boost::beast::http::field::content_type, "text/plain");
                res->keep_alive(request->keep_alive());
                res->body() = "File not found";
                res->prepare_payload();

                boost::beast::http::async_write(
                    m_socket,
                    *res,
                    [self = shared_from_this(), res](boost::system::error_code ec, std::size_t bytes_transferred)
                    {
                        if (!ec && res->keep_alive())
                            self->getRequest();
                    }
                );
            }
        }
        catch (const std::filesystem::filesystem_error& fsec)
        {
            std::cerr << "Ошибка удаления файла" << fmi.fileServerPath << ": " << fsec.what() << "\n";

            auto res = std::make_shared<
                boost::beast::http::response<boost::beast::http::string_body>
            >(boost::beast::http::status::internal_server_error, request->version());

            res->set(boost::beast::http::field::content_type, "text/plain");
            res->keep_alive(request->keep_alive());
            res->body() = fsec.what();
            res->prepare_payload();

            boost::beast::http::async_write(
                m_socket,
                *res,
                [self = shared_from_this(), res](const boost::system::error_code& ec, std::size_t bytes_transferred)
                {
                    if (!ec && res->keep_alive())
                        self->getRequest();
                }
            );
        }


    }

    void downloadFile(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::string downloadHash = std::string(request->target()).substr(15);

        std::cout << "[Session]downloadFile__Download Hash: "  << downloadHash << "\n";


        File::FileMetaInfo fmi;
        fmi.hash = downloadHash;
        fileDB.getRequest( sqlDataBaseRequest::sqlFind, fmi);

        std::cout << "[Session]downloadFile__Server Path:" << fmi.fileServerPath << "\n";

        

        if (!std::filesystem::exists(fmi.fileServerPath))
        {
            /*std::ifstream file(fmi.fileServerPath, std::ios::binary);
            if (!file.is_open())
            {
                res->result(boost::beast::http::status::not_found);
                res->set(boost::beast::http::field::content_type, "text/plain");
                res->body() = "File not found";
                res->prepare_payload();
                return;
            }

            std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            res->result(boost::beast::http::status::ok);
            res->set(boost::beast::http::field::content_type, "application/octet-stream");
            res->set(boost::beast::http::field::content_disposition, "attachment; filename=\"" + fmi.fullName + "\"");
            res->body() = std::string(buffer.begin(), buffer.end());
            res->keep_alive(request->keep_alive());
            res->prepare_payload();*/
            auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(boost::beast::http::status::not_found, request->version());
            res->set(boost::beast::http::field::content_type, "text/plain");
            res->body() = "File not found\n";
            res->prepare_payload();
            boost::beast::http::async_write(
                m_socket,
                *res,
                [self = shared_from_this(), res](const boost::system::error_code& ec, std::size_t bytes_transferred)
                {
                    if (!ec && res->keep_alive())
                    {
                        std::cout << "File not found sent, bytes: " << bytes_transferred << "\n";
                        self->getRequest(); // принимаем новый запрос
                    }
                    else
                    {
                        std::cout << "File writing error: " << ec.message() << "\n";
                        self->getRequest(); // продолжаем слушать даже при ошибке
                    }
                }
            );
            return;
        }
        

        std::ifstream file(fmi.fileServerPath, std::ios::binary);
        if (!file.is_open())
        {
            auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(boost::beast::http::status::not_found, request->version());
            res->set(boost::beast::http::field::content_type, "text/plain");
            res->body() = "File not opened\n";
            res->prepare_payload();
            boost::beast::http::async_write(
                m_socket,
                *res,
                [self = shared_from_this(), res](const boost::system::error_code& ec, std::size_t bytes_transferred)
                {
                    if (!ec && res->keep_alive())
                    {
                        std::cout << "File not found sent, bytes: " << bytes_transferred << "\n";
                        self->getRequest(); // принимаем новый запрос
                    }
                    else
                    {
                        std::cout << "File writing error: " << ec.message() << "\n";
                        self->getRequest(); // продолжаем слушать даже при ошибке
                    }
                }
            );
            return;
        }
     

        /*if (fmi.size != std::filesystem::file_size(fmi.fileServerPath))
        {
            auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(boost::beast::http::status::not_found, request->version());
            res->set(boost::beast::http::field::content_type, "text/plain");
            res->body() = "File size incorrect\n";
            res->prepare_payload();
            return;
        }
        else
        {
            std::cout << "Size\n";
        }*/

        size_t fileRangeStart = 0;
        size_t fileRangeEnd = fmi.size - 1;

        if (request->find(boost::beast::http::field::range) != request->end())
        {
            std::string rangeHeader = std::string((*request)[boost::beast::http::field::range]);
            parseRange(rangeHeader, fileRangeStart, fileRangeEnd, fmi.size);
        }
        else
        {
            std::cout << "[Session]!!(request->find(boost::beast::http::field::range) != request->end() failed\n";
        }

        file.seekg(fileRangeStart);
        const size_t currentPacketFileSize = fileRangeEnd - fileRangeStart + 1;
        //const size_t packetFileSize = 1 * 1024 * 1024;
        //int remainFileSize = packetFileSize - bytes_transferred;
        //std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::vector<char> buffer(currentPacketFileSize);
        file.read(buffer.data(), currentPacketFileSize);
        std::cout << "[Session]Read " << file.gcount() << " bytes from file\n";

        std::shared_ptr<boost::beast::http::response<boost::beast::http::vector_body<char>>> res = std::make_shared<boost::beast::http::response<boost::beast::http::vector_body<char>>>(boost::beast::http::status::ok, request->version());
        res->set(boost::beast::http::field::content_type, "application/octet-stream");
        res->set(boost::beast::http::field::content_disposition, "attachment; filename=\"" + fmi.fullName + "\"");
        res->set("X-File-Hash", fmi.hash);
        res->set("X-File-Size", std::to_string(fmi.size));

        
        res->body().assign(buffer.begin(), buffer.begin() + currentPacketFileSize);
        //*res->body().open(
        //    fmi.fileServerPath.c_str(),
        //    boost::beast::file_mode::scan
        //);
        res->keep_alive(request->keep_alive());
        res->prepare_payload();

        boost::beast::http::async_write(
            m_socket,
            *res,
            [self = shared_from_this(), res](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (!ec && res->keep_alive())
                {
                    
                    std::cout << "Bytes transferred: " << bytes_transferred << "\n";
                    self->getRequest();
                }
                else
                {
                    std::cout << "File writing error: " << ec.what() << "\n";
                    self->getRequest();
                }
            }
        );
    }

    void parseRange(const std::string& header, size_t& start, size_t& end, size_t fileSize) {
        if (header.substr(0, 6) != "bytes=") throw std::runtime_error("Invalid Range");

        auto dashPos = header.find('-');
        if (dashPos == std::string::npos) throw std::runtime_error("Invalid Range");

        start = std::stoull(header.substr(6, dashPos - 6));
        std::string endStr = header.substr(dashPos + 1);

        if (!endStr.empty()) {
            end = std::stoull(endStr);
            if (end >= fileSize) end = fileSize - 1;
        }
        else {
            end = fileSize - 1;
        }

        if (start > end) throw std::runtime_error("Invalid Range");
    }

    //FileMetaInfo uploadFileMetaInfo(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    //{
    //    // Тело запроса → строка
    //    std::string body = boost::beast::buffers_to_string(request->body().data());

    //    FileMetaInfo info;

    //    try
    //    {
    //        // Парсим JSON
    //        nlohmann::json json = nlohmann::json::parse(body);

    //        info = json.get<FileMetaInfo>();

    //        files[info.hash] = std::make_shared<FileMetaInfo>(info);

    //        /*std::cout << "Meta received:\n";
    //        std::cout << "  Name: " << info.name << "\n";
    //        std::cout << "  Size: " << info.size << "\n";
    //        std::cout << "  Hash: " << info.hash << "\n";*/
    //    }
    //    catch (const std::exception& e)
    //    {
    //        std::cerr << "uploadFileMetaInfo error: " << e.what() << "\n";
    //    }


    //    return info;

    //}

};