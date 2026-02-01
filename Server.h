#pragma once

#include "Acceptor.h"

class Server : std::enable_shared_from_this<Server>
{
public:
	Server();


	void start();
	void startThread();
	void stop();


private:
	boost::asio::io_context m_io;
	boost::asio::ssl::context m_ctx{ boost::asio::ssl::context::tlsv12 };
	std::shared_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_executor_work_guard;
	//std::shared_ptr<Acceptor> m_serverAcceptor;
	std::unique_ptr<Acceptor> m_server_acceptor;

	std::vector<std::shared_ptr<std::thread>> m_thread_pool;

};

