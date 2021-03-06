/*
using this block as example to add mutliple symbols
	const char sym1[10] = "abcdef";
	const char sym2[10] = "oilngas";
	const char sym3[10] = "rubbers";

	const char *symlist[10];

	symlist[0] = sym1;
	symlist[1] = sym2;
	symlist[2] = sym3;
	symlist[3] = NULL;

	for (int i=0; symlist[i] != NULL; i++) 
		printf("%s\n", symlist[i]);

TODO: remove feed.DateTime, when it is not needed anymore

** items to check:
HRTBTINTERVAL
instr
dt
port
ipAddress
myclientname
mktdat_req.SymbolID
*/

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

// Key Params
const int HRTBTINTERVAL= 15;
char INSTR[10] = "US";
char DT[10] = "20";
int PORT = 11099;
std::string IPADDRESS = "127.0.0.1";
char MYCLIENTNAME[] = "wings of ostrich";
int SYMBID = 69;

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

void listen_server(int &sock, char* instr, char* dt){
	/* Listen for messages from dtc server */
	std::cout << "listen_server thread started! \n";
	std::fstream fprice;
	std::fstream flog;
	const char sep[] = "_";
	
	char file_log[30] = "log_";
	strcat(file_log, instr);
	strcat(file_log, sep);
	strcat(file_log, dt);
	
	char file_prc[30] = "prc_";
	strcat(file_prc, instr);
	strcat(file_prc, sep);
	strcat(file_prc, dt);
	
	fprice.open(file_prc, std::ios::app);
	flog.open(file_log, std::ios::app);
	char buf[2048];
	Header header;
	DTC::s_EncodingResponse enc_resp;
	DTC::s_LogonResponse logon_resp;
	DTC::s_MarketDataUpdateBidAskCompact mkt_bac;
	DTC::s_MarketDataUpdateTradeCompact mkt_tc;
	DTC::s_MarketDataSnapshot mkt_snap;
	DTC::s_MarketDataUpdateSessionVolume mkt_vol;
	DTC::s_MarketDataUpdateSessionLow mkt_low;
	DTC::s_MarketDataUpdateSessionHigh mkt_high;
	DTC::s_MarketDataUpdateSessionSettlement mkt_set;

	while (!fin) {
		memset(buf, 0, 1024);
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
						fprice << now << ", 117, " <<
							//mkt_bac.Size << ", " <<
							//mkt_bac.DateTime << ", " << 
							//mkt_bac.SymbolID << ", " << 
							mkt_bac.BidPrice << ", " << 
							mkt_bac.BidQuantity << ", " << 
							mkt_bac.AskPrice << ", " << 
							mkt_bac.AskQuantity << "\n";
						std::cout << now  << ", 117, " <<
							//mkt_bac.SymbolID << ", " << 
							mkt_bac.BidPrice << ", " << 
							mkt_bac.AskPrice << "\n";	
						break;
						}
					case 112 :	// Market Data Update Trade Compact
						{
						mkt_tc.CopyFrom(static_cast<void*>(buf));
						fprice << now << ", 112, " <<
							//mkt_tc.Size << ", " <<
							//mkt_tc.DateTime << ", " << 
							//mkt_tc.SymbolID << ", " << 
							mkt_tc.Price << ", " << 
							mkt_tc.Volume << ", " << 
							mkt_tc.AtBidOrAsk << "\n";
						std::cout << now << ", 112, " <<
							//mkt_tc.SymbolID << ", " << 
							mkt_tc.Price << ", " << 
							mkt_tc.Volume << "\n";
							//mkt_tc.AtBidOrAsk << "\n";
						break;
						}
					case 113 :	// Market Data Update Session Volume
						{
						mkt_vol.CopyFrom(static_cast<void*>(buf));
						flog << now << ", 113, " <<
							//mkt_vol.Size << ", " <<
							//mkt_vol.SymbolID << ", " <<
							mkt_vol.Volume << "\n";
						std::cout << now << ", 113, " <<
							//mkt_vol.SymbolID << ", " <<
							mkt_vol.Volume << "\n";
						break;
						}
					case 114 :	// Market Data Update Session High
						{
						mkt_high.CopyFrom(static_cast<void*>(buf));
						flog << now <<", 114, " <<
							//mkt_high.Size << ", " <<
							//mkt_high.SymbolID << ", " <<
							mkt_high.Price << "\n";
						std::cout << now << ", 114," <<
							//mkt_high.SymbolID << ", " <<
							mkt_high.Price << "\n";
						break;
						}
					case 115 :	// Market Data Update Session Low
						{
						mkt_low.CopyFrom(static_cast<void*>(buf));
						flog << now << ", 115, " <<
							//mkt_low.Size << ", " <<
							//mkt_low.SymbolID << ", " <<
							mkt_low.Price << "\n";
						std::cout << now << ", 115," <<
							//mkt_low.SymbolID << ", " <<
							mkt_low.Price << "\n";
						break;
						}
					case 119 :	// Market Data Update Session Settlement
						{
						mkt_set.CopyFrom(static_cast<void*>(buf));
						flog << now << ", 119, " <<
							//mkt_set.Size << ", " <<
							//mkt_set.SymbolID << ", " <<
							mkt_set.Price << ", " << "\n";
							//mkt_set.DateTime << std::endl;
						std::cout << now << ", 119," <<
							//mkt_set.SymbolID << ", " <<
							mkt_set.Price << "\n";
						break;
						}

					case 104 :	// Market Data Snapshot
						{
						mkt_snap.CopyFrom(static_cast<void*>(buf));
						flog << now << ", 104, " <<
							//mkt_snap.Size << ", " <<
							//mkt_snap.SymbolID << ", " << 
							mkt_snap.SessionSettlementPrice << ", " << 
							mkt_snap.SessionOpenPrice << ", " << 
							mkt_snap.SessionHighPrice << ", " << 
							mkt_snap.SessionLowPrice << ", " <<
							mkt_snap.SessionVolume << ", " <<
							mkt_snap.SessionNumTrades << ", " <<
							mkt_snap.OpenInterest << ", " <<
							mkt_snap.BidPrice << ", " <<
							mkt_snap.AskPrice << ", " <<
							mkt_snap.AskQuantity << ", " <<
							mkt_snap.BidQuantity << ", " <<
							mkt_snap.LastTradePrice << ", " <<
							mkt_snap.LastTradeVolume << ", " <<
							mkt_snap.LastTradeDateTime << ", " <<
							mkt_snap.BidAskDateTime << ", " <<
							mkt_snap.SessionSettlementDateTime << ", " <<
							mkt_snap.TradingSessionDate << ", " <<
							mkt_snap.TradingStatus << std::endl;	//assumed to be 3rd response after encoding, logon responses, so flush here
						std::cout << "MARKET_DATA_SNAPSHOT: 104 " <<"\n";
						break;
						}
					case 7 : // Encoding Response
						{ 
						enc_resp.CopyFrom(static_cast<void*>(buf));
						flog << now << ", 7, " <<
							//enc_resp.Size << ", " <<
							enc_resp.ProtocolVersion << ", " <<
							enc_resp.Encoding << ", " <<
							enc_resp.ProtocolType << "\n";
						std::cout << "ENCODING_RESPONSE: " <<
							//enc_resp.Size << ", " <<
							enc_resp.ProtocolVersion << ", " <<
							enc_resp.Encoding << ", " <<
							enc_resp.ProtocolType << "\n";
						break;
						}
					case 2 : //DTC::s_LogonResponse logon_resp;
						{
						logon_resp.CopyFrom(static_cast<void*>(buf));
						flog << now << ", 2, " <<
							//logon_resp.Size << ", "  << 
							logon_resp.ProtocolVersion << ", " << 
							logon_resp.Result << ", " <<
							logon_resp.ResultText << ", " <<
							logon_resp.ServerName << "\n";
						std::cout << "LOGON_RESPONSE: " <<
							//logon_resp.Size << ", "  << 
							logon_resp.ProtocolVersion << ", " << 
							logon_resp.Result << ", " <<
							logon_resp.ResultText << ", " <<
							logon_resp.ServerName << "\n";
						break;
						}
					default:	// Catch all messages not covered yet
						{
						flog << now << ", " << "Unknown: " << ", " << header.type << "\n";
						std::cout << "Unknown: " << header.type << "\n";
						}
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	fprice.flush();
	fprice.close();
	flog.flush();
	flog.close();
	std::cout << " Closed 2 files...\n";
}


int main(int argc, char** argv)
{
	// Confirm contract name
	//char INSTR[5] = "US";
	strcat(INSTR, argv[1]);
	
	//char dt[10] = "20";
	strcat(DT, argv[2]);
	std::cout << "This feed for: " << INSTR << " on: " << DT <<"\n"; 

	// Create a socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		return 1;
	}


	// Create a hint structure for the server we're connecting with
	// ("127.0.0.1", 11099)
	//int PORT = 11099;
	//std::string IPADDRESS = "127.0.0.1";

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(PORT);
	inet_pton(AF_INET, IPADDRESS.c_str(), &hint.sin_addr);

	// Connect to the server on the socket
	int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connectRes == -1) {
		return 1;
	}

	// start listening thread
	std::thread listener (listen_server, std::ref(sock), INSTR, DT);

	// Encoding Request
	{
	DTC::s_EncodingRequest enc_req;
	char bytes2send[enc_req.Size];
	memcpy(bytes2send, &enc_req, enc_req.Size);
	send_message(sock, bytes2send, enc_req.Size);
	}

	// Logon request
	{
	//char myclientname[] = "John's flying ostrich";
	DTC::s_LogonRequest logon_req;
	logon_req.HeartbeatIntervalInSeconds = HRTBTINTERVAL;
	logon_req.SetClientName(MYCLIENTNAME);
	char bytes2send[logon_req.Size];
	memcpy(bytes2send, &logon_req, logon_req.Size);
	send_message(sock, bytes2send, logon_req.Size);
	}

	// Market Data Request
	{
		DTC::s_MarketDataRequest mktdat_req;
		char bytes2send[mktdat_req.Size];
		const char* mysymbol = INSTR;
		std::cout << "Symbol is: " << mysymbol << "\n";
		mktdat_req.RequestAction = DTC::RequestActionEnum::SUBSCRIBE;
		mktdat_req.SymbolID = SYMBID;
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
