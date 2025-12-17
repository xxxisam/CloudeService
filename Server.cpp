#include "Server.h"
#include <boost/asio.hpp> 
//#include <iostream>

#include <thread>

Server::Server()
{
    //m_executor_work_guard = std::make_shared<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(m_io));
}

void Server::startThread()
{
    
    m_thread_pool.emplace_back(std::make_shared<std::thread>(
        [this]() 
        { 
            m_io.run(); 
        }));
}

void Server::start()
{
    const unsigned short portNumber = 8080;
    m_server_acceptor.reset(new Acceptor(m_io, portNumber));
    m_server_acceptor->connect();
    startThread();
}

void Server::stop()
{
    const size_t THREAD_POOL_SIZE = m_thread_pool.size();
    for (size_t i = 0; i < THREAD_POOL_SIZE; ++i)
    {
        m_thread_pool.at(i)->join();
    }


    //m_serverAcceptor->stop();
    m_io.stop();
}
