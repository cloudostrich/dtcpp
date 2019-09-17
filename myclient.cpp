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

struct Header{
		u_int16_t size;
		u_int16_t type;
	};


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
	char buf[1024];
	//std::string userInput;

	//do {

	// Enter lines of text
	// std::cout << "> ";
	// getline(std::cin, userInput);
	
	// Encoding Request
	DTC::s_EncodingRequest enc_req;
	
	// Send to server
	char bytes2send[enc_req.Size];
	memcpy(bytes2send, &enc_req, enc_req.Size);
	int sendRes = send(sock, bytes2send, enc_req.Size, 0);
	
	// Check if failed
	if (sendRes == -1)
	{
		std::cout << "Could not send to server! Whoops!\r\n";
		//continue;
	}

	// Wait for response
	DTC::s_EncodingResponse m_resp;
	Header header;

	memset(buf, 0, 4);
	//int bytesReceived = recv(sock, buf, 4, 0);
	int bytesReceived = recv(sock, buf, 4, 0);
	memcpy(&header, static_cast<void*>(buf), 4);	
	// prevent crash when error
	if (bytesReceived == -1)
	{
		std::cout << "There was an error getting resonse from server \r\n";
	}
	else
	{
		// Display response
		//std::cout << "SERVER> " << std::string(buf, bytesReceived) << "\r\n";
		
		// Get rest of message
		DTC::s_EncodingResponse enc_resp;
		u_int16_t m_size = header.size - 4;
		std::cout << "  m_size: " << m_size << std::endl;
		memset(buf + 4, 0, m_size);
		int bytesReceived = recv(sock, buf+4, m_size, 0);
		//memcpy(&enc_resp, bytesReceived, header.size);
		
		// combine header and message into 1 byte array

		enc_resp.CopyFrom(static_cast<void*>(buf));
		std::cout << "Enc_Resp: Size " << enc_resp.Size << std::endl;
		std::cout << "Enc_Resp: Type " << enc_resp.Type << std::endl;
		std::cout << "Enc_Resp: ProtocolVersion " << enc_resp.ProtocolVersion << std::endl;
		std::cout << "Enc_Resp: Encoding " << enc_resp.Encoding << std::endl;
		std::cout << "Enc_Resp: ProtocolType " << enc_resp.ProtocolType << std::endl;

	}
	
	// Logon request
	// secdef?
	//} while (true);
	// Close the socket
	close(sock);


	return 0;
}
