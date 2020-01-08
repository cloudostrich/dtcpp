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

void send_message(int sock, void *msg2send, u_int16_t size){
	int sendRes = send(sock, msg2send, size, 0);
	// Check if failed
	if (sendRes == -1)
	{
		std::cout << "Could not send to server! Whoops!\r\n";
	}
}

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
	std::string ipAddress = "127.0.0.1";

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to the server on the socket
	int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connectRes == -1) {
		return 1;
	}


	// while Loop:
	char buf[1024];
	
	// Encoding Request
	DTC::s_EncodingRequest enc_req;
	
	// Send to server
	char bytes2send[enc_req.Size];
	memcpy(bytes2send, &enc_req, enc_req.Size);
	send_message(sock, bytes2send, enc_req.Size);
	
	// Wait for response
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
    	// Get rest of message
		DTC::s_EncodingResponse enc_resp;
		u_int16_t m_size = header.size - 4;
		std::cout << "  m_size: " << m_size << std::endl;
		memset(buf + 4, 0, m_size);
		int bytesReceived = recv(sock, buf+4, m_size, 0);
		// combine header and message into 1 byte array

		enc_resp.CopyFrom(static_cast<void*>(buf));
		std::cout << "Enc_Resp: Size: " << enc_resp.Size << std::endl;
		std::cout << "Enc_Resp: Type: " << enc_resp.Type << std::endl;
		std::cout << "Enc_Resp: ProtocolVersion: " << enc_resp.ProtocolVersion << std::endl;
		std::cout << "Enc_Resp: Encoding: " << enc_resp.Encoding << std::endl;
		std::cout << "Enc_Resp: ProtocolType: " << enc_resp.ProtocolType << std::endl;

	}
	
	// Logon request
	DTC::s_LogonRequest logon_req;
	
	// Send to server
	char bytes2send2[logon_req.Size];
	memcpy(bytes2send2, &logon_req, logon_req.Size);
	send_message(sock, bytes2send2, logon_req.Size);

	// Wait for response

	memset(buf, 0, 4);
	int bytesReceived2 = recv(sock, buf, 4, 0);
	memcpy(&header, static_cast<void*>(buf), 4);	
	// prevent crash when error
	if (bytesReceived2 == -1)
	{
		std::cout << "There was an error getting resonse from server \r\n";
	}
	else
	{
    	// Get rest of message
		DTC::s_LogonResponse logon_resp;
		u_int16_t m_size = header.size - 4;
		std::cout << "  m_size: " << m_size << std::endl;
		memset(buf + 4, 0, m_size);
		int bytesReceived2 = recv(sock, buf+4, m_size, 0);
		// combine header and message into 1 bytearray

		logon_resp.CopyFrom(static_cast<void*>(buf));
		std::cout << "Logon_Resp: Size: " << logon_resp.Size << "\n";
		std::cout << "Logon_Resp: Type: " << logon_resp.Type << "\n";
		std::cout << "Logon_Resp: ProtocolVersion: " << logon_resp.ProtocolVersion << "\n";
		std::cout << "Logon_Resp: Status Enum: " << logon_resp.Result << "\n";
		std::cout << "Logon_Resp: ReconnectAddr: " << logon_resp.ReconnectAddress << "\n";
		std::cout << "Logon_Resp: Integer_1: " << logon_resp.Integer_1 << "\n";
		std::cout << "Logon_Resp: ServerName: " << logon_resp.ServerName << "\n";
		std::cout << "Logon_Resp: MarketDepthUpdatesBestBidAndAsk: " << logon_resp.MarketDepthUpdatesBestBidAndAsk << "\n";
		std::cout << "Logon_Resp: TradingIsSupported: " << logon_resp.TradingIsSupported << "\n";
		std::cout << "Logon_Resp: OCOOrdersSupported: " << logon_resp.OCOOrdersSupported << "\n";
		std::cout << "Logon_Resp: OrderCancelReplaceSupported: " << logon_resp.OrderCancelReplaceSupported << "\n";
		std::cout << "Logon_Resp: SymbolExchangeDelimiter: " << logon_resp.SymbolExchangeDelimiter << "\n";
		std::cout << "Logon_Resp: SecurityDefinitionsSupported: " << logon_resp.SecurityDefinitionsSupported << "\n";
		std::cout << "Logon_Resp: HistoricalPriceDataSupported: " << logon_resp.HistoricalPriceDataSupported << "\n";
		std::cout << "Logon_Resp: ResubscribeWhenMarketDataFeedAvailable: " << logon_resp.ResubscribeWhenMarketDataFeedAvailable << "\n";
		std::cout << "Logon_Resp: MarketDepthIsSupported: " << logon_resp.MarketDepthIsSupported << "\n";
		std::cout << "Logon_Resp: OneHistoricalPriceDataRequestPerConnection: " << logon_resp.OneHistoricalPriceDataRequestPerConnection << "\n";
		std::cout << "Logon_Resp: BracketOrdersSupported: " << logon_resp.BracketOrdersSupported << "\n";
		std::cout << "Logon_Resp: UseIntegerPriceOrderMessages: " << logon_resp.UseIntegerPriceOrderMessages << "\n";
		std::cout << "Logon_Resp: UsesMultiplePositionsPerSymbolAndTradeAccount: " << logon_resp.UsesMultiplePositionsPerSymbolAndTradeAccount << "\n";
		std::cout << "Logon_Resp: MarketDataSupported: " << logon_resp.MarketDataSupported << "\n";
		}

	// secdef?
	//} while (true);
	// Close the socket
	close(sock);


	return 0;
}
