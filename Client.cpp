#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <cstdlib>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <string>
#include <atomic>
#include <memory>

#define PORT 9108
#define BUFLEN 10008
#define SERVERADDRESS "127.0.0.1"

class Message
{
	std::atomic<bool> boolExit = false;
	std::string message;
public:
	void sendMsg(SOCKET sock)
	{
		try {
			std::string name;
			std::cout << "Enter your name: ";
			std::getline(std::cin, name);
			
			
			std::cout << "All set to chat to anyone!\nIf you want to exit the chat app, type 'exit' (without quotes)\n";
			while (!boolExit)
			{
				
				std::getline(std::cin, message);
				if (message == "exit")
				{
					boolExit = true;
					std::cout << "Chat completed\n";
					exit(0);
				}

				std::string context = name + ": " + message;
				int bytsent = send(sock, context.c_str(), context.length(), 0);


			}
		}
		catch (...)
		{
			std::cout << "Unable to send data to server " << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	void receiveMsg(SOCKET sock)
	{
		try //exception handling
		{
			char buffer[BUFLEN];
			int recvlength;
			while (!boolExit)
			{
				recvlength = recv(sock, buffer, sizeof(buffer) - 1, 0);
				if (recvlength > 0)
				{
					buffer[recvlength] = '\0';
					std::string msg(buffer);
					std::cout << msg << std::endl;
				}
				else
				{
					std::cout << "Connection closed or unable to receive data from server\n";
					boolExit = true;
					break;
				}
			}
		}
		catch (...)
		{
			std::cout << "Not receiving data " << std::endl;
			exit(EXIT_FAILURE);
		}
	}
};
class Client : public Message //inheritance
{
	WSADATA data;
	SOCKET s;
	sockaddr_in serverAddr;
public:
	Client()
	{
		try //exception handling
		{
			WSAStartup(MAKEWORD(2, 2), &data);
			s = socket(AF_INET, SOCK_STREAM, 0);
			
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(PORT);
			
			inet_pton(AF_INET, SERVERADDRESS, &(serverAddr.sin_addr));
			connect(s, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
			std::cout << "Connected to server\n";


		}
		catch (...)
		{
			std::cout << "Unable to connect to client with error code " << WSAGetLastError() << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	~Client()
	{
		closesocket(s);
		WSACleanup();
	}
	void Start()
	{
		Message msg;
		//concurrency - multiple threads for sending and receiving msgs
		std::thread sender(&Message::sendMsg, &msg, s); 
		std::thread receiver(&Message::receiveMsg, &msg, s);

		sender.join();
		receiver.join();
	}

};

int main()
{
	system("title Chat Client");
	std::shared_ptr<Client> client(new Client); //Smart pointer to create client object
	
	client->Start();
	return 0;
}