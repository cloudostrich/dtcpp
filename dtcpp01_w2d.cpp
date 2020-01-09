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
#include <iomanip>
#include <fstream>

#include "DTCProtocol.cpp"
#include "DTCProtocol.h"

// Working Flag
static bool fin = false;
const int HRTBTINTERVAL= 15;

struct Header{
		u_int16_t size;
		u_int16_t type;
	};

void send_heartbeat(int &sock) {
	/* Create and send heartbeat message */
	std::cout << "HeartBeater Started...\n";
	char buf[16]; // heartbeat msg is only 16 b
	DTC::s_Heartbeat hrtbt;
	memcpy(buf, &hrtbt, hrtbt.Size);
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
	/* Listen for messages from dtc server */
	std::cout << "listen_server thread started! \n";
	std::fstream fileprice, filelog;
	fileprice.open("fileprice.csv", std::ios::app);
	filelog.open("filelog", std::ios::app);
	char buf[512];
	Header header;
	DTC::s_EncodingResponse enc_resp;
	DTC::s_LogonResponse logon_resp;
	DTC::s_MarketDataUpdateBidAskCompact mkt_bac;
	DTC::s_MarketDataUpdateTradeCompact mkt_tc;
	DTC::s_MarketDataSnapshot mkt_snap;

	while (!fin) {
		memset(buf, 0, 128);
		int bytesReceived = recv(sock, buf, 4, 0);
		memcpy(&header, static_cast<void*>(buf), 4);
		if (bytesReceived == -1){
			std::cout << "There was an error getting a response from the server \r\n";
		}
		else {
			//memset(buf+4, 0, header.size-4);
			int bytesReceived = recv(sock, buf+4, header.size-4, 0);
			time_t now1 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			auto now = std::put_time(std::localtime(&now1), "%F %T"); 
			if (header.type != 3) {	// ignore heartbeat responses
				switch(header.type) 
				{
					case 117 :  // Market Data Udate BidAsk Compact
						{
						//mkt_bac.Clear();
						mkt_bac.CopyFrom(static_cast<void*>(buf));
						fileprice << now << ", 117, " <<
							mkt_bac.GetMessageSize() << ", " <<
							mkt_bac.GetDateTime() << ", " << 
							mkt_bac.GetSymbolID() << ", " << 
							mkt_bac.GetBidPrice() << ", " << 
							mkt_bac.GetBidQuantity() << ", " << 
							mkt_bac.GetAskPrice() << ", " << 
							mkt_bac.GetAskQuantity() << "\n";
						std::cout << now  << ", 117, " <<
							mkt_bac.GetSymbolID() << ", " << 
							mkt_bac.GetBidPrice() << ", " << 
							mkt_bac.GetAskPrice() << "\n";
						break;
						}
					case 112 :	// Market Data Update Trade Compact
						{
						mkt_tc.CopyFrom(static_cast<void*>(buf));
						fileprice << now << ", 112, " <<
							mkt_tc.GetMessageSize() << ", " <<
							mkt_tc.GetDateTime() << ", " << 
							mkt_tc.GetSymbolID() << ", " << 
							mkt_tc.GetPrice() << ", " << 
							mkt_tc.GetVolume() << ", " << 
							mkt_tc.GetAtBidOrAsk() << "\n";
						std::cout << now << ", 112, " <<
							mkt_tc.GetSymbolID() << ", " << 
							mkt_tc.GetPrice() << ", " << 
							mkt_tc.GetVolume() << ", " << 
							//mkt_tc.GetAtBidOrAsk() << "\n";
						break;
						}
					case 104 :	// Market Data Snapshot
						{
						mkt_snap.CopyFrom(static_cast<void*>(buf));
						filelog << now << ", 104, " <<
							mkt_snap.GetMessageSize() << ", " <<
							mkt_snap.GetSymbolID() << ", " << 
							mkt_snap.GetSessionSettlementPrice() << ", " << 
							mkt_snap.GetSessionOpenPrice() << ", " << 
							mkt_snap.GetSessionHighPrice() << ", " << 
							mkt_snap.GetSessionLowPrice() << ", " <<
							mkt_snap.GetSessionVolume() << ", " <<
							mkt_snap.GetSessionNumTrades() << ", " <<
							mkt_snap.GetOpenInterest() << ", " <<
							mkt_snap.GetBidPrice() << ", " <<
							mkt_snap.GetAskPrice() << ", " <<
							mkt_snap.GetAskQuantity() << ", " <<
							mkt_snap.GetBidQuantity() << ", " <<
							mkt_snap.GetLastTradePrice() << ", " <<
							mkt_snap.GetLastTradeVolume() << ", " <<
							mkt_snap.GetLastTradeDateTime() << ", " <<
							mkt_snap.GetBidAskDateTime() << ", " <<
							mkt_snap.GetSessionSettlementDateTime() << ", " <<
							mkt_snap.GetTradingSessionDate() << ", " <<
							mkt_snap.GetTradingStatus() << ", " <<
							mkt_snap.GetMarketDepthUpdateDateTime() << ", " << "\n";
						std::cout << "MARKET_DATA_SNAPSHOT: 104 " <<"\n";
						break;
						}
					case 7 : // Encoding Response
						{ 
						enc_resp.CopyFrom(static_cast<void*>(buf));
						filelog << now << ", 7, " <<
							enc_resp.GetMessageSize() << ", " <<
							enc_resp.GetProtocolVersion() << ", " <<
							enc_resp.GetEncoding() << ", " <<
							enc_resp.GetProtocolType() << "\n";
						std::cout << "ENCODING_RESPONSE: " <<
							enc_resp.GetMessageSize() << ", " <<
							enc_resp.GetProtocolVersion() << ", " <<
							enc_resp.GetEncoding() << ", " <<
							enc_resp.GetProtocolType() << "\n";
						break;
						}
					case 2 : //DTC::s_LogonResponse logon_resp;
						{
						logon_resp.CopyFrom(static_cast<void*>(buf));
						filelog << now << ", 2, " <<
							logon_resp.GetMessageSize() << ", "  << 
							logon_resp.GetProtocolVersion() << ", " << 
							logon_resp.GetResult() << ", " <<
							logon_resp.GetResultText() << ", " <<
							logon_resp.GetReconnectAddress() << ", " << 
							logon_resp.GetInteger_1() << ", " << 
							logon_resp.GetServerName() << ", " << 
							logon_resp.GetMarketDepthUpdatesBestBidAndAsk() << ", " << 
							logon_resp.GetTradingIsSupported() << ", " << 
							logon_resp.GetOCOOrdersSupported() << ", " << 
							logon_resp.GetOrderCancelReplaceSupported() << ", " << 
							logon_resp.GetSymbolExchangeDelimiter() << ", " << 
							logon_resp.GetSecurityDefinitionsSupported() << ", " << 
							logon_resp.GetHistoricalPriceDataSupported() << ", " <<
							logon_resp.GetResubscribeWhenMarketDataFeedAvailable() <<  ", " << 
							logon_resp.GetMarketDepthIsSupported() << ", " << 
							logon_resp.GetOneHistoricalPriceDataRequestPerConnection() << ", " << 
							logon_resp.GetBracketOrdersSupported() << ", " << 
							logon_resp.GetUseIntegerPriceOrderMessages() << ", " << 
							logon_resp.GetUsesMultiplePositionsPerSymbolAndTradeAccount() << ", " << 
							logon_resp.GetMarketDataSupported() << "\n";
						std::cout << "LOGON_RESPONSE: " << 
							logon_resp.GetMessageSize() << ", "  << 
							logon_resp.GetProtocolVersion() << ", " << 
							logon_resp.GetResult() << ", " <<
							logon_resp.GetResultText() << ", " <<
							logon_resp.GetReconnectAddress() << ", " << 
							logon_resp.GetInteger_1() << ", " << 
							logon_resp.GetServerName() << ", " << 
							logon_resp.GetMarketDepthUpdatesBestBidAndAsk() << ", " << 
							logon_resp.GetTradingIsSupported() << ", " << 
							logon_resp.GetOCOOrdersSupported() << ", " << 
							logon_resp.GetOrderCancelReplaceSupported() << ", " << 
							logon_resp.GetSymbolExchangeDelimiter() << ", " << 
							logon_resp.GetSecurityDefinitionsSupported() << ", " << 
							logon_resp.GetHistoricalPriceDataSupported() << ", " <<
							logon_resp.GetResubscribeWhenMarketDataFeedAvailable() <<  ", " << 
							logon_resp.GetMarketDepthIsSupported() << ", " << 
							logon_resp.GetOneHistoricalPriceDataRequestPerConnection() << ", " << 
							logon_resp.GetBracketOrdersSupported() << ", " << 
							logon_resp.GetUseIntegerPriceOrderMessages() << ", " << 
							logon_resp.GetUsesMultiplePositionsPerSymbolAndTradeAccount() << ", " << 
							logon_resp.GetMarketDataSupported() << "\n";
						break;
						}
					default:	// Catch all messages not covered yet
						{
						filelog << now << "Unknown: " << header.type << "\n";
						std::cout << "Unknown: " << header.type << "\n";
						}
				}
			}
		}
	}
	fileprice.close();
	filelog.close();
	std::cout " Closed 2 files...\n";
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
		const char mysymbol[] = "XAUUSD";
		mktdat_req.RequestAction = DTC::RequestActionEnum::SUBSCRIBE;
		mktdat_req.SymbolID = 88;
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
