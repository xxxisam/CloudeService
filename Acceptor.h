#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "Session.hpp"


class Acceptor
{
public:
	Acceptor(boost::asio::io_context& io, unsigned int portNumber) : m_io(io), m_acceptor(m_io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), portNumber)) //m_socket(m_io)
	{
		std::atomic<bool> m_aceptanceIsStopped = false;
	}

	void connect()
	{
		makeConnect();
	}



private:
	void makeConnect()
	{
		std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io);

		boost::system::error_code ec;

		if (ec)
		{
			std::cout << "Server socket opening error: " << ec.what() << "\n";
		}
		m_acceptor.async_accept(
			*socket,
			[this, socket](const boost::system::error_code& ec)
			{
				if (!ec)
				{
					std::shared_ptr<std::string> tempmsg(std::make_shared<std::string>("Successful connection!"));
					std::cout << "Server started at http://localhost:8080\n";

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
					onAccept(ec, socket);
				}
				else
				{
					std::cout << "initConnect - Server Error connection: " << ec.message() << "\n";
				}

			}
		);
	}

	void onAccept(const boost::system::error_code& ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
	{
		if (!ec)
		{
			std::cout << "start\n";
			std::make_shared<Session>(std::move(*sock))->start();
		}
		else
		{
			std::cout << "onAccept - error occured: Error message " << ec.message() << "\n";
		}


		if (!m_aceptanceIsStopped.load())
		{
			connect();
		}
		else
		{
			m_acceptor.close();
		}
	}

private:
	boost::asio::io_context& m_io;
	boost::asio::ip::tcp::acceptor m_acceptor;
	//boost::asio::ip::tcp::socket m_socket;
	std::atomic<bool> m_aceptanceIsStopped;

	
	//std::vector<Session> m_users_in_session;
	boost::beast::flat_buffer m_beastBuf;
	boost::beast::http::request<boost::beast::http::string_body> m_request;
};

