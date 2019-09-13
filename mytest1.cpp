#include <iostream>
#include <cstring>
#include <climits>
#include <cfloat>

#include "DTCProtocol.h"
#include "DTCProtocol.cpp"

int HIS = 999999999;

int main()
{
	DTC::s_EncodingRequest enc_req;
	DTC::s_LogonRequest logon_req;
	
	logon_req.SetClientName("GoGoGomezzz");
	logon_req.HeartbeatIntervalInSeconds = HIS;

	std::cout << " Contents of my encoding request: "<< std::endl;
	std::cout << "Size: " << enc_req.Size << std::endl;
	std::cout << "Type: " << enc_req.Type << std::endl;
	std::cout << "Protocol Version: " << enc_req.ProtocolVersion << std::endl;
	std::cout << "Encoding: " << enc_req.Encoding << std::endl;
	std::cout << "Protocol Type: " << enc_req.ProtocolType << std::endl;
	std::cout << "-------------------------------------" << std::endl;
	std::cout << "Contents of my logon request" << std::endl;
	std::cout << "Size: " << logon_req.Size << std::endl;
	std::cout << "Type: " << logon_req.Type << std::endl;
	std::cout << "Protocol Version: " << logon_req.ProtocolVersion << std::endl;
	std::cout << "Username: " << logon_req.Username << std::endl;
	std::cout << "Password: " << logon_req.Password << std::endl;
	std::cout << "GeneralTextData: " << logon_req.GeneralTextData << std::endl;
	std::cout << "Integer1: " << logon_req.Integer_1 << std::endl;
	std::cout << "Integer2: " << logon_req.Integer_2 << std::endl;
	std::cout << "HeartbeatIntervalinSeconds: " << logon_req.HeartbeatIntervalInSeconds << std::endl;
	std::cout << "Trademode: " << logon_req.TradeMode << std::endl;
	std::cout << "TradeAccount: " << logon_req.TradeAccount << std::endl;
	std::cout << "HardwareIdentifier: " << logon_req.HardwareIdentifier << std::endl;
	std::cout << "ClientName: " << logon_req.ClientName << std::endl;
	std::cout << "-------------------------------------" << std::endl;
	std::cout << " Dats all folks!!!" << std::endl;
	std::cout << logon_req.GetHeartbeatIntervalInSeconds() << std::endl;
	std::cout << logon_req.GetMessageSize() << std::endl;
}


