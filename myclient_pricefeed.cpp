#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

#include "DTCProtocol.cpp"
#include "DTCProtocol.h"

//using namespace std;

// Working Flag
static bool fin = false;
const int HRTBTINTERVAL= 15;

struct Header{
		u_int16_t size;
		u_int16_t type;
	};

void send_heartbeat(int &sock) {
	// Create heartbeat message
	std::cout << "HeartBeater Started...\n";
	char buf[16];
	DTC::s_Heartbeat hrtbt;
	//std::cout << "Heartbeater Size: " << hrtbt.Size << "\n";
	while (!fin) {
		int sendRes = send(sock, buf, hrtbt.Size, 0);
		if (sendRes == -1) {
			std::cout << "Could not send Heartbeat to Server!!!\n";
		}
		std::this_thread::sleep_for(std::chrono::seconds(HRTBTINTERVAL));
	}
}

void send_message(int sock, void *msg2send, u_int16_t size){
    // Send message to server
	int sendRes = send(sock, msg2send, size, 0);
	
	// Check if failed
	if (sendRes == -1)
	{
		std::cout << "Could not send to server! Whoops!\r\n";
		//continue;
	}
}

void listen_server(int &sock){
	// Listen for messages from dtc server
	std::cout << "listen_server thread started! \n";
	char buf[1024];
	Header header;
	DTC::s_EncodingResponse enc_resp;
	DTC::s_LogonResponse logon_resp;

	while (!fin) {
		memset(buf, 0, 4);
		int bytesReceived = recv(sock, buf, 4, 0);
		memcpy(&header, static_cast<void*>(buf), 4);
		if (bytesReceived == -1){
			std::cout << "There was an error getting a response from the server \r\n";
		}
		else {
			memset(buf+4, 0, header.size-4);
			int bytesReceived = recv(sock, buf+4, header.size - 4, 0);
			// ignore heartbeat responses
			if (header.type != 3) {
				// switch case here
				switch(header.type) 
				{
					case 117 :
						std::cout << "MARKET_DATA_UPDATE_BID_ASK_COMPACT: " << header.type << "\n";  
						break;
					case 112 :
						std::cout << "MARKET_DATA_UPDATE_TRADE_COMPACT: " << header.type <<"\n";
						break;
					case 104 :
						std::cout << "MARKET_DATA_SNAPSHOT: " << header.type <<"\n";
						break;
					case 7 : // Encoding Response
						{ 
						enc_resp.CopyFrom(static_cast<void*>(buf));
						std::cout << "ENCODING_RESPONSE: " <<
							enc_resp.Size << ", " <<
							enc_resp.Type << ", " <<
							enc_resp.ProtocolVersion << ", " <<
							enc_resp.Encoding << ", " <<
							enc_resp.ProtocolType << "\n";
						break;
						}
					case 2 : //DTC::s_LogonResponse logon_resp;
						{
						logon_resp.CopyFrom(static_cast<void*>(buf));
						std::cout << "LOGON_RESPONSE: " << 
							logon_resp.Size << ","  << 
							logon_resp.Type << ", " << 
							logon_resp.ProtocolVersion << ", " << 
							logon_resp.Result << ", " << 
							logon_resp.ReconnectAddress << ", " << 
							logon_resp.Integer_1 << ", " << 
							logon_resp.ServerName << ", " << 
							logon_resp.MarketDepthUpdatesBestBidAndAsk << ", " << 
							logon_resp.TradingIsSupported << ", " << 
							logon_resp.OCOOrdersSupported << ", " << 
							logon_resp.OrderCancelReplaceSupported << ", " << 
							logon_resp.SymbolExchangeDelimiter << ", " << 
							logon_resp.SecurityDefinitionsSupported << ", " << 
							logon_resp.HistoricalPriceDataSupported << ", " << 
							logon_resp.ResubscribeWhenMarketDataFeedAvailable <<  ", " << 
							logon_resp.MarketDepthIsSupported << ", " << 
							logon_resp.OneHistoricalPriceDataRequestPerConnection << ", " << 
							logon_resp.BracketOrdersSupported << ", " << 
							logon_resp.UseIntegerPriceOrderMessages << ", " << 
							logon_resp.UsesMultiplePositionsPerSymbolAndTradeAccount << ", " << 
							logon_resp.MarketDataSupported << "\n";
						break;
						}
					default:
						std::cout << "Unknown: " << header.type << "\n";
				}
			}
		}
	}
}


int main()
{
	// Create a socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		return 1;
	}


	// Create a hint structure for the server we're connecting with
	// ("127.0.0.1", 11099)
	int port = 11099;
	std::string ipAddress = "192.168.1.171";

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to the server on the socket
	int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connectRes == -1) {
		return 1;
	}

	// start listening thread
	std::thread listener (listen_server, std::ref(sock));

	// Encoding Request
	{
	DTC::s_EncodingRequest enc_req;
	char bytes2send[enc_req.Size];
	memcpy(bytes2send, &enc_req, enc_req.Size);
	send_message(sock, bytes2send, enc_req.Size);
	}

	// Logon request
	{
	char myclientname[] = "John's Beast Machine";
	DTC::s_LogonRequest logon_req;
	logon_req.HeartbeatIntervalInSeconds = HRTBTINTERVAL;
	logon_req.SetClientName(myclientname);
	char bytes2send[logon_req.Size];
	memcpy(bytes2send, &logon_req, logon_req.Size);
	send_message(sock, bytes2send, logon_req.Size);
	}
	
	// Market Data Request
	{
		DTC::s_MarketDataRequest mktdat_req;
		char bytes2send[mktdat_req.Size];
		//std::string mysymbol = "USOil";
		char mysymbol[] = "USOil";
		mktdat_req.RequestAction = DTC::RequestActionEnum::SUBSCRIBE;
		mktdat_req.SymbolID = 888;
		//strncpy(mktdat_req.Symbol, mysymbol.c_str(), sizeof(mysymbol));
		mktdat_req.SetSymbol(mysymbol);
		memcpy(bytes2send, &mktdat_req, mktdat_req.Size);
		send_message(sock, bytes2send, mktdat_req.Size);
	}

	// Start Heartbeater
	std::thread heartbeater (send_heartbeat, std::ref(sock));

	// wait for keypress to terminate program
	std::cin.get();
	
	// termination process
	fin = true;
	// create and send logoff message
	{
	DTC::s_Logoff logoff;
	char bytes2send[logoff.Size];
	memcpy(bytes2send, &logoff, logoff.Size);
	send_message(sock, bytes2send, logoff.Size);
	}
	listener.join();
	heartbeater.join();
	close(sock);
	std::cout << "Program Terminated...\n";

	return 0;
}
