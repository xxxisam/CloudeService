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
#include "HashFileTable.h"
#include "DataBaseUtility.hpp"
#include <CryptoPP/sha.h>
#include <CryptoPP/hex.h>
#include <CryptoPP/filters.h>
#include "UserDataBase.h"
#include "TokenDataBase.h"
#include "SessionManager.h"
#include <beast/http/error.hpp>
#include <beast/http/verb.hpp>

#include <CryptoPP/osrng.h>
#include <CryptoPP/hex.h>
#include <CryptoPP/filters.h>
#include <beast/http/fields.hpp>
#include <beast/http/field.hpp>



class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket socket, UserDataBase& uDB, TokenDataBase& tDB) : m_socket(std::move(socket)), fileDB(FileDataBase::getInstance()), hashDB(HashDataBase::getInstance()), userDB(uDB), tokenDB(tDB), m_token()
    {

    }

    void start()
    {
        hashDB.start();
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

    void inizializeToken(SessionManager::Token& token)
    {
        std::cout << "[Session][inizializeToken] !!\n";
        if (tokenDB.takeTokenInfo(m_token))
        {
            std::cout << "[Session][inizializeToken]\n Token " << token.token << " exist!\n";
        }
        else
        {
            std::cout << "[Session][inizializeToken]\n Token " << token.token << " doesnt exist!\n";
        }
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    std::shared_ptr<User::User> m_user;
    std::filesystem::path m_serverFolder;
    std::string m_loginUser;
    SessionManager::Token m_token;

    //DBS
    FileDataBase& fileDB;
    HashDataBase& hashDB;
    UserDataBase& userDB;
    TokenDataBase& tokenDB;

    //boost::beast::flat_buffer m_buffer;
    //boost::beast::http::request<boost::beast::http::string_body> m_request;
    //std::unordered_map<std::string, std::shared_ptr<File::FileMetaInfo>> m_files;
    std::unordered_map<std::string, std::shared_ptr<File::tempFileData>> m_upload_files;
    nlohmann::json m_JSONUploadFiles = nlohmann::json::array();
    //std::vector<std::string> hash_of_uploading_files;
    FileDataBase* filedb;





    void getRequest()
    {
        std::cout << "[Session] Getting request\n";
        std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request = std::make_shared<boost::beast::http::request<boost::beast::http::dynamic_body>>();
        std::shared_ptr<boost::beast::flat_buffer> buffer = std::make_shared<boost::beast::flat_buffer>();
        std::shared_ptr<boost::beast::http::request_parser<boost::beast::http::dynamic_body>> parser = std::make_shared<boost::beast::http::request_parser<boost::beast::http::dynamic_body>>();
        parser->body_limit(10 * 1024 * 1024);
        std::cout << "[Session] Getting request\n";
        boost::beast::http::async_read(
            m_socket,
            *buffer,
            *parser,
            [self = shared_from_this(), request, buffer, parser](const boost::system::error_code& ec, std::size_t)
            {
                if (ec)
                {
                    if (ec != boost::beast::http::error::end_of_stream)
                    {
                        std::cerr << "[Session] async_read error: "  << ec.message() << "\n";
                    }
                    std::cerr << "getRequest async_read Error: " << ec.message() << "\n";
                    return;
                }
                else
                {
                    std::cout << "[Session][getRequest]Ok\n";
                }

                std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request = std::make_shared<boost::beast::http::request<boost::beast::http::dynamic_body>>(parser->get());
                if (request->target() != "/upload")
                {
                    self->printRequest(request);
                }
                //if ((request->target() == "/") && (request->base()["Cookie"] == ""))
                //{
                //    std::cout << "[Session][getRequest] First apperance request\n";
                //    self->createGuestToken(self->m_token);
                //    /*self->m_token.ipAddress = self->m_socket.remote_endpoint().address().to_string();
                //    std::cout << "IP:" << self->m_token.ipAddress << "\n";
                //    
                //    self->m_token.token = self->createToken();
                //    self->m_token.expires = 60 * 60;
                //    self->m_token.isAdmin = false;
                //    self->m_token.isGuest = true;
                //    std::cout << "Ну и что дальше\n";
                //    self->tokenDB.addTokenInTable(self->m_token);
                //    std::cout << self->m_token.token << " is generated token\n";*/


                //    //std::string ip = "unknown";
                //    //try
                //    //{
                //    //    ip = self->m_socket.remote_endpoint().address().to_string();
                //    //}
                //    //catch (...)
                //    //{
                //    //    std::cout << "[Session][LOGIN] Cannot get remote IP\n";
                //    //}

                //    //std::string userAgent = "unknown";
                //    //auto uaIt = request->find(boost::beast::http::field::user_agent);
                //    //if (uaIt != request->end())
                //    //{
                //    //    userAgent = std::string(uaIt->value());
                //    //}

                //    //constexpr int64_t TOKEN_TTL = 60 * 60;

                //    //std::cout << "[Session][LOGIN] IP: " << ip << "\n";
                //    //std::cout << "[Session][LOGIN] User-Agent: " << userAgent << "\n";

                //    //Token token;
                //    //User::User user;
                //    //if (!self->tokenDB.findByTokenIfExistByLogin(token))
                //    //{
                //    //    std::cout << "[Session][LOGIN] token doesn't exist, creating\n";
                //    //    token = SessionManager::getInstance().createToken(
                //    //        user.id,
                //    //        user.login,
                //    //        user.isAdmin,
                //    //        user.login == "" ? true : false,  //is guest
                //    //        ip,
                //    //        userAgent,
                //    //        TOKEN_TTL
                //    //    );
                //    //    std::cout << "Generated token is -- " << token.token << "\n";
                //    //    self->tokenDB.addTokenInTable(token);
                //    //}
                //    //else
                //    //{
                //    //    std::cout << "[Session][LOGIN] token exist, getting\n";
                //    //    self->tokenDB.getTokenByLogin(token);
                //    //}
                //}
                //исрправить [Session][getRequest] session have cookie [null] ничего не вывело
                if (self->checkSession(request))
                {
                    std::cout << "[Session][getRequest] session have cookie\n";
                    self->m_token.token = request->base()[boost::beast::http::field::cookie];
                    std::cout << self->m_token.token << "\n";
                }
                if(self->m_token.token == "")
                {
                    boost::beast::http::fields::iterator cookIt = request->find(boost::beast::http::field::cookie);
                    if (cookIt != request->end())
                    {
                        std::cout << "[Session][getRequest] This request have cookie: " << request->base()[boost::beast::http::field::cookie] << "\n";
                        std::cout << "[Session][GetRequest]Inizialize Token \n";
                        self->m_token.token = request->base()[boost::beast::http::field::cookie];
                        std::cout << "m_token.token - " << self->m_token.token << "\t" << "Request base " << request->base()[boost::beast::http::field::cookie] << "\n";
                        self->inizializeToken(self->m_token);
                        SessionManager::printTokenInfo(self->m_token);
                        
                    }
                    else
                    {
                        std::cout << "[Session][getRequest] This request doesnt have cookie: \n";
                        auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
                        self->createGuestToken(self->m_token);
                        //std::cout << "[Session][getRequest] Установка Cookie \n";
                        //res->set(boost::beast::http::field::set_cookie, self->m_token.token);
                    }
                }


                self->processRequest(request);
            }
        );
    }

    //if session fill true, else false
    bool checkSession(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        boost::beast::http::fields::iterator sessionIt = request->find(boost::beast::http::field::cookie);
        if (sessionIt != request->end())
        {
            std::cout << "[Session][checkSession] session cookie is " << request->base()[boost::beast::http::field::cookie] << "\n";
            return false;
        }
        else
        {
            return true;
        }
    }

    void printRequest(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::cout << "\n========== HTTP REQUEST ==========\n";
        std::cout << "Method       : " << request->method_string() << "\n";
        std::cout << "Target       : " << request->target() << "\n";
        std::cout << "Version      : " << (request->version() == 11 ? "HTTP/1.1" : "HTTP/1.0") << "\n";
        std::cout << "Keep-Alive   : " << (request->keep_alive() ? "yes" : "no") << "\n";
       

        std::cout << "\n--- Headers ---\n";
        //std::string sessionValue = request->find();
        boost::beast::http::fields::iterator Authentification = request->find(boost::beast::http::field::authorization);

        if (Authentification != request->end())
        {
            std::cout << "Authentification output: " << Authentification->name_string() << "\n";
        }

        for (auto const& h : request->base())
        {
            std::cout << h.name_string() << ": " << h.value() << "\n";
        }

        std::string body = boost::beast::buffers_to_string(request->body().data());

        std::cout << "\n--- Body (" << body.size() << " bytes) ---\n";
        if (!body.empty())
            std::cout << body << "\n";
        else
            std::cout << "<empty>\n";

        std::cout << "==================================\n\n";
    }

    void createAuthToken(SessionManager::Token& token, const User::User& user)
    {
        //unsigned int id = 0;
        token.token = createToken();
        //unsigned int userId = 0;
        token.userLogin = user.login;
        //unsigned int expires = 0;     
        token.isGuest = false;
        token.isAdmin = (token.userLogin == "admin" ? true : false);
        //int issuedAt = 0;
        //std::string ipAddress = "";
        //std::string userAgent = "";
        //unsigned int lastUsed = 0;
        tokenDB.addTokenInTable(token);
        std::cout << "Created createAuthToken token " << token.token << "\n";
        std::cout << std::boolalpha << "is admin " << token.isAdmin << "\n";
        std::cout << std::boolalpha << "is guest " << token.isGuest << "\n";
        std::cout << "[Session][createAuthToken]\n";
        SessionManager::printTokenInfo(m_token);
    }

    void createGuestToken(SessionManager::Token& token)
    {
        std::cout << "Creating guest token: \n";
        //unsigned int id = 0;
        token.token = createToken();
        //unsigned int userId = 0;
        token.userLogin = "";
        //unsigned int expires = 0;     // UNIX time
        token.isGuest = true;
        token.isAdmin = false;
        //int issuedAt = 0;
        //std::string ipAddress = "";
        //std::string userAgent = "";
        //unsigned int lastUsed = 0;
        std::cout << "Created guest token " << token.token << "\n";
        tokenDB.addTokenInTable(token);
        std::cout << "[Session][createGuestToken]\n";
        SessionManager::printTokenInfo(m_token);
    }

    std::string createToken()
    {
        std::string generatedToken;
        do
        {
            generatedToken = generateToken();
            std::cout << "[Session] generated token is -- " << generatedToken << "\n";
        } while (tokenDB.isThisTokenExisted(generatedToken));
        
        std::cout << "Generated token returned\n";
        return generatedToken;
    }

    std::string generateToken()
    {
        CryptoPP::AutoSeededRandomPool rng;

        CryptoPP::byte seed[32];
        rng.GenerateBlock(seed, sizeof(seed));

        std::string token;
        CryptoPP::SHA256 hash;

        CryptoPP::StringSource(
            seed,
            sizeof(seed),
            true,
            new CryptoPP::HashFilter(
                hash,
                new CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(token),
                    false
                )
            )
        );

        return token;
    }


    void processRequest(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::string target = std::string(request->target());
        boost::beast::http::verb method = request->method();


        /*if (m_token.token == "")
        {
            boost::beast::http::fields::iterator cookIt = request->find(boost::beast::http::field::cookie);
            if (cookIt != request->end())
            {
                std::cout << "[Session][getRequest] This request have cookie: " << request->base()[boost::beast::http::field::cookie].substr(8) << "\n";
                std::cout << "[Session][GetRequest]Inizialize Token \n";
                m_token.token = request->base()[boost::beast::http::field::cookie].substr(8);
                std::cout << "m_token.token - " << m_token.token << "\t" << "Request base " << request->base()[boost::beast::http::field::cookie].substr(8) << "\n";
                inizializeToken(m_token);
                SessionManager::printTokenInfo(m_token);
            }
            else
            {
                std::cout << "[Session][getRequest] This request doesnt have cookie: \n";
            }
        }*/


        std::cout << "[Session][processRequest] 1. target is " << target << "\n";

        

        if (target.empty())
        {
            target = "/";//Добавить проверку n входа. если нет логинащмпшоалртуцшгпркушгцрпкшуцроо!!!!!!!!!!!!!!!!
           /* Token tok;
            User::User user;
            std::string token = SessionManager::getInstance().generateToken();
            std::cout << "TOken - " << token << "\n";
            res->set(boost::beast::http::field::cookie, token);*/
        }

        std::cout << "[Session][processRequest] 2. target is " << target << "\n";
        if (method == boost::beast::http::verb::get)
        {
            if (target == "/" || target == "LogPage")
            {
                std::cout << "[Session][processRequest] 3. redirect\n";
                //redirect(request, "/logPage.html"); 
                redirect(request, "LogPage.html");
            }
            else if (target == "/logPage")
            {
                std::cout << "[Session][method: GET. target: /logPage] 3. \n";
                loadPage(request, target);
            }
            else if (target == "/MainPage" && m_token.isGuest == false)
            {
                std::cout << "[Session][method: GET. target: /MainPage] 3. \n";
                loadPage(request, target);
            }
            else if (target == "/registration")
            {
                loadPage(request, target);
            }
            else if (target == "/list")
            {
                pressRefreshButton(request);
            }
            else if (target.substr(0, 9) == "/download")
            {
                downloadFile(request);
            }
            else
            {
                std::cout << "[Session][method: GET. target: not identify]  3.\n";
                loadPage(request, target);
            }
        }
        else if (method == boost::beast::http::verb::post)
        {
            if (target == "/upload")
            {
                upload(request);
            }
            else if (target == "/login")
            {
                std::cout << "[Session][method: POST. target: login]  3.\n";
                login(request);
            }
            else if(target == "/registration")
            {
                std::cout << "[Session][method: POST. target: register]!!!!!!!))))____)_)!!!!!!!!!)))_)_!)!\n";
                registration(request);
            }
        }
        else if (method == boost::beast::http::verb::delete_)
        {
            if (target.substr(0, 7) == "/delete")
            {
                std::cout << "[Session]Delete method was !!!!!!!!!!!!!\n";
                deleteFile(request);
            }
        }
        else
        {
            std::cout << "Неверный вид запроса: " << target << "\n";
            std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> res;
            res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(boost::beast::http::status::not_found, request->version());
            res->body() = "Not found";
            res->keep_alive(request->keep_alive());
            res->prepare_payload();

            boost::beast::http::async_write(
                m_socket,
                *res,
                [self = shared_from_this(), res, request](boost::system::error_code ec, std::size_t bytes_transfferd)
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

    void redirect(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request, const std::string targetLocation)
    {
        auto res = std::make_shared<boost::beast::http::response<boost::beast::http::empty_body>>(boost::beast::http::status::found, request->version());

        res->set(boost::beast::http::field::location, targetLocation);
        if (checkSession(request))
        {
            std::cout << "[Session][Redirect] doesn't have cookie\n";
            if (m_token.token != "")
            {
                SessionManager::printTokenInfo(m_token);
                res->set(boost::beast::http::field::set_cookie, m_token.token);
            }
            else
            {
                std::cout << "[Session][Redirect] m_token.token != \"\"\n";
                createGuestToken(m_token);
                res->set(boost::beast::http::field::set_cookie, m_token.token);
            }
        }
        else
        {
            std::cout << "[Session][Redirect] have Cookie\n";
        }
        res->keep_alive(request->keep_alive());
        res->prepare_payload();

        boost::beast::http::async_write(
            m_socket,
            *res,
            [self = shared_from_this(), res](const boost::beast::error_code& ec, size_t bytes_trasnferred)
            {
                if (ec)
                {
                    std::cerr << "[Session][redirect] async_write error: " << ec.message() << "\n";
                    return;
                }

                
                if (res->keep_alive())
                {
                    self->getRequest();
                }
                else
                {
                    
                    boost::system::error_code shutdown_ec;
                    self->m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, shutdown_ec);
                }
            }
        );
    }

    void loadPage(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request, const std::string targetPage)
    {
        const std::string target = std::string(request->target());
        std::cout << "[Session][loadPage] Target for check -> " << targetPage << "\n";

        const std::string full_path = "bin" + targetPage;
        std::cout << "[Session][loadPage] TargetPage for check -> " << full_path << "\n";


        boost::beast::http::file_body::value_type body;
        boost::beast::error_code ec;
        body.open(full_path.c_str(), boost::beast::file_mode::scan, ec);

        if (ec)
        {
            //sendError(boost::beast::http::status::not_found, "File not found: " + target, request);
            return;
        }
        
        auto res = std::make_shared<boost::beast::http::response<boost::beast::http::file_body>>(
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(boost::beast::http::status::ok, request->version())
        );

        /*if (request->base()["Cookie"] == "")
        {
            if (m_token.token == "")
            {
                std::cout << "[Session][loadPage] Cookie and m_token.token = \"\"\n";
                m_token.token = generateToken();
            }

            res->set(boost::beast::http::field::set_cookie, "session=" + m_token.token + "; Path=/; HttpOnly; SameSite=Lax");
            tokenDB.addTokenInTable(m_token);
            tokenDB.printTokenTable();
        }*/
        if (checkSession(request))
        {
            std::cout << "[Session][loadPage] set cookie in loadPage, Cookie doesnt exist\n";
            request->set(boost::beast::http::field::set_cookie, m_token.token);
        }
        res->set(boost::beast::http::field::content_type, checkMimeType(full_path));
        res->content_length(res->body().size());
        res->keep_alive(request->keep_alive());

        boost::beast::http::async_write(
            m_socket,
            *res,
            [self = shared_from_this(), res](const boost::beast::error_code& ec, size_t bytes_trasnferred)
            {
                if (ec)
                {
                    std::cerr << "[Session][loadPage] async_write error: " << ec.message() << "\n";
                    return;
                }

                if (res->keep_alive())
                    self->getRequest();
                else
                {
                    boost::system::error_code shutdown_ec;
                    self->m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, shutdown_ec);
                }
            }
        );
    }

    void login(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        std::cout << "\n\n\n\n\n\n\n\n\n\n--------------------------[Session][LOGIN]-------------------------\n";
        const nlohmann::json userJSON = parseLoginJSON(request);
        User::User user = User::jsonToUser(userJSON);

        std::cout << "[Session][LOGIN] User login: " << user.login << "\n";

        //check in db
        std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> res;
        DataBaseUtility::UserDBResult dbResult = userDB.verification(user);
        if (dbResult == DataBaseUtility::UserDBResult::OK)
        {
            /*std::string ip = "unknown";
            try
            {
                ip = m_socket.remote_endpoint().address().to_string();
            }
            catch (...)
            {
                std::cout << "[Session][LOGIN] Cannot get remote IP\n";
            }

            std::string userAgent = "unknown";
            auto uaIt = request->find(boost::beast::http::field::user_agent);
            if (uaIt != request->end())
            {
                userAgent = std::string(uaIt->value());
            }

            constexpr int64_t TOKEN_TTL = 60 * 60;

            std::cout << "[Session][LOGIN] IP: " << ip << "\n";
            std::cout << "[Session][LOGIN] User-Agent: " << userAgent << "\n";*/

            /*if (request->base()[boost::beast::http::field::cookie].substr(8) == "")
            {*/

            std::cout << "[Session][login] login set cookie!\n";
            SessionManager::Token tempToken = m_token;
            tokenDB.takeTokenInfo(tempToken);
            SessionManager::printTokenInfo(tempToken);
            if (tempToken.isGuest)
            {
                std::cout << "[Session][login] login creating auth token!\n";
                createAuthToken(m_token, user);
                DataBaseUtility::UserDBResult ret = tokenDB.delToken(tempToken);
                if (ret == DataBaseUtility::UserDBResult::OK)
                {
                    std::cout << "[Session][login] successful deletion! OK\n";
                }
                else if(ret == DataBaseUtility::UserDBResult::TOKEN_DOESNT_EXIST)
                {
                    std::cout << "[Session][login] unsuccessful deletion! UserDBResult::TOKEN_DOESNT_EXIST\n";
                }
            }
            //}
            std::cout << "Geiofopfkdsg" << m_token.token << "\n";


            res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::ok,
                request->version()
            );
            res->set(boost::beast::http::field::set_cookie, m_token.token);
            /*if (!tokenDB.findByTokenIfExistByLogin(token))
            {
                std::cout << "[Session][LOGIN] token doesn't exist, creating\n";
                token = SessionToken::ge
                    user.id,
                    user.login,
                    user.isAdmin,
                    user.login != "" ? true : false,
                    ip,
                    userAgent,
                    TOKEN_TTL
                );
                std::cout << "Generated token is -- " << token.token << "\n";
                tokenDB.addTokenInTable(token);
            }
            else
            {
                std::cout << "[Session][LOGIN] token exist, getting\n";
                tokenDB.getTokenByLogin(token);
            }*/
            std::cout << "Finish setting cookorke\n";
            tokenDB.addTokenInTable(m_token);
            tokenDB.printTokenTable();

            if (checkSession(request))
            {
                if (m_token.token == "")
                {
                    m_token.token = generateToken();
                }

                res->set(boost::beast::http::field::set_cookie, m_token.token);

            }

            res->body() = "USER_REGISTRATION_OK";
        }
        else if (dbResult == DataBaseUtility::UserDBResult::USER_DOESNT_EXIST)
        {
            res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::conflict,
                request->version()
            );
            res->body() = "USER_USER_DOESNT_EXIST";
        }
        else if (dbResult == DataBaseUtility::UserDBResult::WRONG_PASSWORD)
        {
            res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::conflict,
                request->version()
            );
            res->body() = "USER_WRONG_INCORRECT_DATA";
        }
        else
        {
            std::cout << "[Session][method: POST. target: register] !!!!!!!! ANOTHER MISTAKE !!!!!!!!!!!!!!!!\n";
            res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::internal_server_error,
                request->version()
            );
            res->body() = "INTERNAL_ERROR";
        }

        res->set(boost::beast::http::field::content_type, "text/plain");
        res->keep_alive(request->keep_alive());
        res->prepare_payload();


        std::cout << "\n\n";
        boost::beast::http::async_write(m_socket, *res, [self = shared_from_this(), res, request](boost::system::error_code ec, std::size_t bytes_transfferd)
            {
                if (!ec && res->keep_alive())
                {
                    self->getRequest();
                }
                else
                {
                    std::cerr << "async_write error: " << ec.message() << "\n";
                }
            });
    }

    void registration(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        const nlohmann::json userJSON = parseLoginJSON(request);
        User::User user = User::jsonToUser(userJSON);

        std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> res;
        if (userDB.addUserInTable(user) == DataBaseUtility::UserDBResult::OK)
        {
            res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::ok,
                request->version()
            );
            res->body() = "USER_REGISTRATION_OK";
        }
        else if (userDB.addUserInTable(user) == DataBaseUtility::UserDBResult::USER_ALREADY_EXIST)
        {
            res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::conflict,
                request->version()
            );
            res->body() = "USER_ALREADY_REGISTERED";
        }
        else
        {
            std::cout << "[Session][method: POST. target: register] !!!!!!!! ANOTHER MISTAKE !!!!!!!!!!!!!!!!\n";
            res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                boost::beast::http::status::internal_server_error,
                request->version()
            );
            res->body() = "INTERNAL_ERROR";
        }


        if (checkSession(request))
        {
            if (m_token.token == "")
            {
                m_token.token = generateToken();
            }
           
            res->set(boost::beast::http::field::set_cookie,  m_token.token);
 
        }
        res->set(boost::beast::http::field::content_type, "text/plain");
        res->keep_alive(request->keep_alive());
        res->prepare_payload();

        boost::beast::http::async_write(m_socket, *res, [self = shared_from_this(), res, request](boost::system::error_code ec, std::size_t bytes_transfferd)
            {
                if (!ec && res->keep_alive())
                {
                    self->getRequest();
                }
                else
                {
                    std::cerr << "async_write error: " << ec.message() << "\n";
                }
            });
    }

    void upload(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
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
                std::cout << "[Session][upload]Canonical path: " << fmi.fileServerPath.string() << "\n";
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "File doesn't exist or error: " << e.what() << "\n";
            }

            //std::cout << "222Canonical path: " << fmi.name << "\n";
            DataBaseUtility::UserDBResult opRes = hashDB.addHash(fmi);
            filedb = &FileDataBase::getInstance();
            if (opRes == DataBaseUtility::UserDBResult::INSERT_OK)
            {
                const int HASH_ID = hashDB.getHashID(fmi);
                SessionManager::printTokenInfo(m_token);
                std::cout << "[Session][upload]Print token before:\n";
                SessionManager::printTokenInfo(m_token);
                if (m_token.userLogin == "")
                {
                    std::cout << "m_token.userLogin doesn't inizialized\n";
                    inizializeToken(m_token);
                    SessionManager::printTokenInfo(m_token);
                }
                if (filedb->addRow(fmi, HASH_ID, m_token.userLogin) == DataBaseUtility::UserDBResult::OK)
                {
                    std::cout << "1.1[Session][upload] success if (opRes == DataBaseUtility::UserDBResult::INSERT_OK)if (opRes == DataBaseUtility::UserDBResult::INSERT_OK)\n \n";
                }
                else
                {
                    std::cout << "1.2[Session][upload]if (opRes == DataBaseUtility::UserDBResult::INSERT_OK) FAILED \n \n";
                }
            }
            else if (opRes == DataBaseUtility::UserDBResult::HASH_ALREADY_EXIST)
            {
                std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                File::printFileMetaInfo(fmi);
                std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                const int HASH_ID = hashDB.getHashID(fmi);
                std::cout << "m_token.userLogin before addrow" << m_token.userLogin << "\n";
                if (m_token.userLogin == "")
                {
                    std::cout << "m_token.userLogin doesn't inizialized\n";
                    inizializeToken(m_token);
                }
                if (filedb->addRow(fmi, HASH_ID, m_token.userLogin) == DataBaseUtility::UserDBResult::OK)
                {
                    std::cout << "2.1[Session][upload] success if (opRes == DataBaseUtility::UserDBResult::INSERT_OK)if (opRes == DataBaseUtility::UserDBResult::INSERT_OK)\n \n";
                }
                else
                {
                    std::cout << "2.2   [Session][upload]if (opRes == DataBaseUtility::UserDBResult::INSERT_OK) FAILED \n \n";
                }
            }
            else
            {

            }
            /*filedb->getRequest(sqlDataBaseRequest::sqlInsert, fmi);
            filedb->getRequest(sqlDataBaseRequest::sqlPrint);*/
            
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

   void getLoginSQL()
   {

   }

   nlohmann::json parseLoginJSON(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
   {
       std::string body = boost::beast::buffers_to_string(
           request->body().data()
       );
       std::cout << "[Session][parseLoginJSON] msg body: " << body << "\n";


       return nlohmann::json::parse(body);

       /*std::string login = j["login"];
       std::string password = j["password"];

       std::cout << "[Session][parseLoginJSON]LOGIN: " << login << "\n";
       std::cout << "[Session][parseLoginJSON]PASSWORD: " << password << "\n";*/
   }

    void pressRefreshButton(std::shared_ptr<boost::beast::http::request<boost::beast::http::dynamic_body>> request)
    {
        nlohmann::json json;

        //fileDB.getRequest(sqlDataBaseRequest::sqlSync, json, m_token);
        if (m_token.userLogin == "")
        {
            std::cout << "[Session][pressRefreshButton]m_token.userLogin doesn't inizialized\n";
            inizializeToken(m_token);
            SessionManager::printTokenInfo(m_token);
        }
        std::cout << "[Session][pressRefreshButton]+returnFilesFromDataBase+\n";
        fileDB.returnFilesFromDataBase(json, m_token.userLogin);
        std::cout << "[Session][pressRefreshButton]-returnFilesFromDataBaseend=\n";
        //std::cout << "pressRefreshButton---filesJson[fileServerPath] JSON: " << json["fileServerPath"] << "\n";
        for (const nlohmann::json& file : json)
        {
            std::cout << "pressRefreshButton__fileServerPath = " << file["name"] << "\n";
            std::cout << "pressRefreshButton__fileServerPath = " << file["uploaderLogin"] << "\n";
        }
        
        auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
            boost::beast::http::status::ok, request->version()
        );


        std::cout << "[Session][pressRefreshButton]FileDB Print\n";
        fileDB.getRequest(sqlDataBaseRequest::sqlPrint);
        std::cout << "[Session][pressRefreshButton]HashDB Print\n";
        hashDB.getRequest(sqlDataBaseRequest::sqlPrint);
        std::cout << "[Session][pressRefreshButton]UserDB Print\n";
        userDB.getRequest(sqlDataBaseRequest::sqlPrint);
        
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
        /*std::string deletedHash = std::string(request->target()).substr(13);
        std::cout << "Deleted Hash: " << deletedHash << "\n";*/
        auto bodyReq = boost::beast::buffers_to_string(request->body().data());
        std::cout << bodyReq <<"\n";
        nlohmann::json json = nlohmann::json::parse(bodyReq);

        File::FileMetaInfo fmi;
        fmi.hash = json["hash"];
        fmi.fullName = json["filename"];
        std::cout << "[Session][deleteFile]HASH deleted file check is " << fmi.hash << "\n";
        std::cout << "[Session][deleteFile]fullName deleted file check is " << fmi.fullName << "\n";

        //fmi.hash = deletedHash;
        fileDB.getRequest(sqlDataBaseRequest::sqlFind, fmi);
        fmi.fileServerPath = defPath::defaultPath::serverFolder / fmi.fullName;
        std::cout << "[Session][deleteFile] fmi servgfoikejgoifjnes: " << fmi.fileServerPath << "\n";

        std::error_code std_ec;
        try
        {
            if (std::filesystem::remove(fmi.fileServerPath))
            {
                std::cout << "[Session][deleteFile] fmi.fileServerPath - delete server path: " << fmi.fileServerPath << "\n";
                fileDB.delUserFile(fmi, m_token);
                if (hashDB.minusRef(fmi))
                {
                    std::cout << "[Session][deleteFile] minusRef successful\n";
                }
                else
                {
                    std::cout << "[Session][deleteFile] minusRef unsuccessful\n";
                }
                if (hashDB.isRefCountZero(fmi))
                {
                    std::cout << "RefCount = 0 \n";
                    hashDB.getRequest(sqlDataBaseRequest::sqlDelete, fmi);
                }
                else
                {
                    std::cout << "RefCount != 0 \n";
                }
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
                std::cout << "[Session][deleteFile]Файл " << fmi.hash << " не найден.\n";
                std::cout << "[Session][deleteFile]Файл " << fmi.fileServerPath << " не найден.\n";
                std::cout << "[Session][deleteFile]Файл " << fmi.fullName << " не найден.\n";

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
        fileDB.getRequest(sqlDataBaseRequest::sqlFind, fmi);
        hashDB.getRequest(sqlDataBaseRequest::sqlGetServerPath, fmi);

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