
#include "Server.h"
#include "Acceptor.h"
#include <iostream>

int main()
{
	std::cout << "Server\n";
	setlocale(LC_ALL, "ru");


	Server server;
	server.start();
	server.stop();


	return 0;
}