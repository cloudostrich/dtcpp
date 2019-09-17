#include <iostream>
#include <cstring>
#include <climits>
#include <cfloat>

#include "DTCProtocol.h"
#include "DTCProtocol.cpp"


int main()
{
	DTC::s_EncodingRequest enc_req;
	
	std::cout << "-------Contents of my encoding request:-------"<< std::endl;
	std::cout << "Size: " << enc_req.Size << std::endl;
	std::cout << "Type: " << enc_req.Type << std::endl;
	std::cout << "Protocol Version: " << enc_req.ProtocolVersion << std::endl;
	std::cout << "Encoding: " << enc_req.Encoding << std::endl;
	std::cout << "Protocol Type: " << enc_req.ProtocolType << std::endl;
	std::cout << "-------------------------------------" << std::endl;

	char* sbytes = static_cast<char*>(static_cast<void*>(&enc_req));
	std::cout << "My Bytes: " << &sbytes << std::endl;
	std::cout << "Size of mybytes: " << sizeof(&sbytes) << std::endl;
	
	// sending side
	char b[enc_req.Size];
	mempcpy(b, &enc_req, enc_req.Size);
	std::cout << "Size of B: " << sizeof(b) << std::endl;
	std::cout << "Here is B: " << &b << std::endl;
	std::cout << "Here is B: " << &b[0] << std::endl;

	DTC::s_EncodingRequest tmp; //Re-make the struct
    	memcpy(&tmp, b, sizeof(tmp));
	std::cout << tmp.Size << std::endl;
	std::cout << tmp.Type << std::endl;
	std::cout << tmp.ProtocolVersion << std::endl;
	std::cout << tmp.Encoding << std::endl;
	std::cout << tmp.ProtocolType << std::endl;
	std::cout << "--------------------------" << std::endl;

	struct Header{
		u_int16_t size;
		u_int16_t type;
	};

	Header header;
	std::cout << "Size of Header struct: " << 4 << std::endl;

	memcpy(&header, b, sizeof(header));
	std::cout << "Header: " << header.size << " AND " << header.type << std::endl;
	// for loop to read 4 bytes of b

	std::cout << "--------------dats all folks-------------" << std::endl;
	return 0;
}
