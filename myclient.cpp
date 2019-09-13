#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#include "DTCProtocol.cpp"
#include "DTCProtocol.h"

//using namespace std;

int main()
{
	// Create a socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		return 1;
	}


	// Create a hint structure for the server we're connecting with
	// ("127.0.0.1", 11099)
	int port = 11099;
	std::string ipAddress = "192.168.1.100";

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to the server on the socket
	int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connectRes == -1)
	{
		return 1;
	}


	// while Loop:
	char buf[4];
	std::string userInput;

	do {

		// Enter lines of text
		// std::cout << "> ";
		// getline(std::cin, userInput);
		
		// Encoding Request
		DTC::s_EncodingRequest enc_req;
		
		// Send to server
		int sendRes = send(sock, enc_req, enc_req.GetMessageSize(), 0);
		
		// Check if failed
		if (sendRes == -1)
		{
			std::cout << "Could not send to server! Whoops!\r\n";
			continue;
		}

		// Wait for response
		memset(buf, 0, 4);
		//int bytesReceived = recv(sock, buf, 4, 0);
		int header = recv(sock, buf, 4, 0);
		
		// prevent crash when error
		if (header == -1)
		{
			std::cout << "There was an error getting resonse from server \r\n";
		}
		else
		{
			// Display response
			//std::cout << "SERVER> " << std::string(buf, bytesReceived) << "\r\n";
			
			// Get Message Size and Type
			uint16_t m_size = header;
			uint16_t m_type = ;

			// Get rest of message

		}
		
		// Logon request
		// secdef?
	} while (true);
	// Close the socket
	close(sock);


	return 0;
}
