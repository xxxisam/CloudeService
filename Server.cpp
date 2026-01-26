#include "Server.h"
#include <boost/asio.hpp> 
#include "TokenDataBase.h"
#include "UserDataBase.h"
//#include <iostream>

#include <thread>

Server::Server()
{
    std::cout << "[Server] Server starting\n";
    //m_executor_work_guard = std::make_shared<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(m_io));
}

void Server::startThread()
{
    std::cout << "[Server] ThreadPool starting\n";
    unsigned int thread_pool_size = std::thread::hardware_concurrency() * 2;
    if (thread_pool_size == 0)
    {
        thread_pool_size = 2;
    }
    for (unsigned int i = 0; i < thread_pool_size; ++i)
    {
       /* m_thread_pool.emplace_back(std::make_shared<std::thread>(
            [this]()
            {
                m_io.run();
            }));*/
        try
        {
            std::cout << "[Server] Thread " << i << " started, io_context running\n";
            m_io.run();
            std::cout << "[Server] Thread " << i << " f inished\n";
        }
        catch (const std::exception& ex)
        {
            std::cerr << "[Server] Exception in io_context thread: " << ex.what() << "\n";
        }
        catch (...)
        {
            std::cerr << "[Server] Unknown exception in io_context thread\n";
        }
    }
}

void Server::start()
{
    const unsigned short portNumber = 8080;
    "[Server] Server stard databases\n";
    UserDataBase& userDB = UserDataBase::getInstance();
    TokenDataBase& tokenDB = TokenDataBase::getInstance();
    userDB.start();
    tokenDB.start();

    m_server_acceptor.reset(new Acceptor(m_io, portNumber, tokenDB, userDB));
    m_server_acceptor->connect();
    startThread();
}

void Server::stop()
{
    std::cout << "[Server] stop\n";
    const size_t THREAD_POOL_SIZE = m_thread_pool.size();
    m_io.stop();
    for (size_t i = 0; i < THREAD_POOL_SIZE; ++i)
    {
        m_thread_pool.at(i)->join();
    }


    //m_serverAcceptor->stop();
}
