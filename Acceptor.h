#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "Session.hpp"
#include "SessionManager.h"
#include "UserDataBase.h"
#include "TokenDataBase.h"

class Acceptor
{
public:
	Acceptor(boost::asio::io_context& io, unsigned int portNumber, TokenDataBase& tokenDB, UserDataBase& userDB) : m_io(io), m_acceptor(m_io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), portNumber)), m_aceptanceIsStopped(false), m_tokenDB(tokenDB), m_userDB(userDB)
	{
		std::cout << "Server started at http://localhost:8080\n";
		m_tokenDB.start();
		m_userDB.start();
	}

	void connect()
	{
		std::cout << "[Acceptor] connect\n";
		makeConnect();
	}



private:
	void makeConnect()
	{
		std::cout << "[Acceptor] make connect\n";
		std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io);

		boost::system::error_code ec;

		if (ec)
		{
			std::cout << "[Acceptor]Server socket opening error: " << ec.what() << "\n";
		}
		m_acceptor.async_accept(
			*socket,
			[this, socket](const boost::system::error_code& ec)
			{
				if (!ec)
				{
					std::shared_ptr<std::string> tempmsg(std::make_shared<std::string>("[Acceptor]Successful connection!"));

					std::cout << "[Acceptor]Remote ENDPOINT: - " << socket->remote_endpoint().address().to_string() << "\n";
					std::cout << "[Acceptor]Remote PORT: - " << socket->remote_endpoint().port() << "\n";
					std::cout << "[Acceptor]Local ENDPOINT: - " << socket->local_endpoint().address().to_string() << "\n";
					std::cout << "[Acceptor]Local PORT: - " << socket->local_endpoint().port() << "\n";
					

					std::cout << *tempmsg << "\n";

					/*boost::asio::async_write(*socket, boost::asio::buffer(*tempmsg),
						[this,socket, tempmsg](const boost::system::error_code& ec, const size_t bytes_written)
						{
							if (!ec)
							{
								std::cout << "Successful send message!\n";
								std::cout << "bytesWritten = " << bytes_written << "\n";
							}
							else
							{
								std::cout << "Server Error send message: " << ec.message() << "\n";
							}
						}
					);*/
					try {
						onAccept(ec, socket);
					}
					catch (const std::exception& erc)
					{
						std::cout << "[Acceptor] Session start error: " << erc.what() << "\n";
					} 
				}
				else
				{
					std::cout << "[Acceptor] initConnect - Server Error connection: " << ec.message() << "\n";
				}
				if (!m_aceptanceIsStopped.load())
				{
					std::cout << "[Acceptor] connect();\n";
					makeConnect();
				}
				else
				{
					std::cout << "[Acceptor] close();\n";
					m_acceptor.close();
				}
				//makeConnect();
			}
		);
	}

	void onAccept(const boost::system::error_code& ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
	{
		if (!ec)
		{
			std::cout << "[Acceptor] onAccept start session\n";
			std::shared_ptr<Session> session = std::make_shared<Session>(std::move(*sock), m_userDB, m_tokenDB);
			session->start();
		}
		else
		{
			std::cout << "[Acceptor] onAccept - error occured: Error message " << ec.message() << "\n";
		}
	}

private:
	boost::asio::io_context& m_io; 
	boost::asio::ip::tcp::acceptor m_acceptor;
	//boost::asio::ip::tcp::socket m_socket;
	std::atomic<bool> m_aceptanceIsStopped;
	//SessionManager m_sessionManager;


	TokenDataBase& m_tokenDB;
	UserDataBase& m_userDB;

	
	std::vector<SessionManager::Token> m_users_in_session;
};