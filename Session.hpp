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
#include <sstream>

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket socket) : m_socket(std::move(socket))
    {
       
    }

    void start()
    {
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


    //boost::beast::flat_buffer m_buffer;
    //boost::beast::http::request<boost::beast::http::string_body> m_request;



    void getRequest()
    {
        auto request = std::make_shared<boost::beast::http::request<boost::beast::http::dynamic_body>>();
        auto buffer = std::make_shared<boost::beast::flat_buffer>();

        boost::beast::http::async_read(
            m_socket,
            *buffer,
            *request,
            [self = shared_from_this(), request, buffer](const boost::system::error_code& ec, std::size_t)
            {
                if (ec)
                {
                    std::cerr << "getRequest async_read Error: " << ec.message() << "\n";
                    return;
                }
                self->processRequest(request);
            }
        );
    }

    void processRequest(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::string target = std::string(request->target());
        boost::beast::http::verb method = request->method();

        if (target.empty())
            target = "/";

        if (method == boost::beast::http::verb::get)
        {
            if (target == "/list")
                pressRefreshButton(request);
            else
                loadMainPage(request);
        }
        else if (method == boost::beast::http::verb::post && target == "/upload")
        {
            pressAddButton(request);
        }
        else
        {
            std::cout << "Неверный вид запроса: " << target << "\n";
            auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
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
                        std::cerr << "async_write error: " << ec.message() << "\n";
                    else if (res->keep_alive())
                        self->getRequest();
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
            auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
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

        auto size = body.size();
        auto res = std::make_shared<boost::beast::http::response<boost::beast::http::file_body>>(
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

    void pressAddButton(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        auto& body = request->body();
        std::ofstream out("D:/S/Server/upload.txt", std::ios::binary);

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
        );
    }

    void pressRefreshButton(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::string json = scan_directory_to_json("D:/S/Server");

        auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
            boost::beast::http::status::ok, request->version()
        );
        res->set(boost::beast::http::field::content_type, "application/json");
        res->body() = json;
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
    }

    std::string scan_directory_to_json(const std::string& dir_path)
    {
        std::ostringstream json;
        json << "[";
        bool first = true;
        for (const auto& entry : std::filesystem::directory_iterator(dir_path))
        {
            if (!entry.is_regular_file()) continue;
            if (!first) json << ",";
            first = false;
            auto path = entry.path();
            auto size = entry.file_size();
            json << "{";
            json << "\"name\":\"" << path.filename().string() << "\",";
            json << "\"size\":" << size << ",";
            json << "\"extension\":\"" << path.extension().string() << "\"";
            json << "}";
        }
        json << "]";
        return json.str();
    }
};
