#ifndef TRANSMITTER_H
#define TRANSMITTER_H

// Boost
#include <boost/asio.hpp>

#include "http.h"

class transmitter
{
public:
	transmitter(boost::asio::io_service *postback);
	~transmitter();
	void connect(std::string url);
	void send(const char* data, int size);
	bool Starttransfer(std::string);
	class server_connection_exception: public std::exception { };
	http_response *response;
	void callback(http_response *res);
private:
	boost::asio::io_service io_service;
	boost::asio::io_service *postbackio;
};

#endif // TRANSMITTER_H
