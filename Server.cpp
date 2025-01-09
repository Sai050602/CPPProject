#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <memory>
#define PORT 9108
#define BUFLEN 10008

class Server
{
	WSADATA data;
	SOCKET	clientSocket;
	SOCKET serverSocket;
	sockaddr_in serverAddr;
	std::vector<SOCKET> clients; //STL container - vector to push connected clients
public:
	Server()
	{
		try { //exception handling

			WSAStartup(MAKEWORD(2, 2), &data);

			std::cout << "Initialised\n";
			serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(PORT);

			InetPton(AF_INET, _T("0.0.0.0"), &serverAddr.sin_addr);
			bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
			listen(serverSocket, SOMAXCONN);
			std::cout << "Socket is listening\n";
		}
		catch (...)
		{
			std::cout << "Exiting Server connection with error code " << WSAGetLastError() << "\n";
			WSACleanup();
			exit(EXIT_FAILURE);
		}
	}
	~Server()
	{
		closesocket(serverSocket);
		WSACleanup();

	}
	void InteractWithClient(SOCKET clientSock, std::vector<SOCKET>& clients)
	{
		char buffer[BUFLEN];
		try //exception handling
		{
			while (true)
			{
				int bytrecvd = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
				if (bytrecvd > 0)
				{
					buffer[bytrecvd] = '\0';
					std::string message(buffer);
					std::cout << "Client says: " << message << std::endl;
					for (auto cli : clients)
					{
						if (cli != clientSock)
						{
							int bytsent = send(cli, message.c_str(), message.size(), 0);
							
						}
					}

				}
			}
			auto it = std::find(clients.begin(), clients.end(), clientSock);
			if (it!= clients.end())
			{
				clients.erase(it);
			}

			closesocket(clientSock);


		}
		catch (...)
		{
			std::cout << "No communication with client with error code " << WSAGetLastError() << std::endl;
			
		}
	}

	void Start()
	{
		try //exception handling
		{
			while (true)
			{
				clientSocket = accept(serverSocket, nullptr, nullptr);
				clients.push_back(clientSocket);
				std::thread th(&Server::InteractWithClient, this, clientSocket, std::ref(clients));
				th.detach();
			}
		}
		catch (...)
		{
			std::cout << "Unable to accept connection" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
};

int main()
{
	system("title Chat Server");
	std::shared_ptr<Server> server( new Server); //created shared ptr to access the server object to manage memory dynamically
	server->Start();
	return 0;
	
}