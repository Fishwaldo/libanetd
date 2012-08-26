#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>

#include <exception>
#include <map>
#include <string>
#include <vector>

#include "http_response.h"

class http_request
{
public:
	http_request(http_response *response, boost::asio::io_service &io_service);
	~http_request();

	void clear();
	void reset();
	void send();
	void send(std::string absolute_url);
	void receive();
	void disconnect();


	std::string method;
	std::string host;
	std::string url;
	std::string version;
	int status;
	std::string description;
	std::map<std::string, std::string> arguments;
	std::map<std::string, std::string> headers;
	std::string body;
	http_response *response;

	class connection_exception: public std::exception { };
	class policy_file_request_exception: public std::exception { };
private:
	size_t socksend(std::string data);
	size_t sockget(char *data, size_t size);
	bool verify_callback(bool preverified, boost::asio::ssl::verify_context &vctx);

	enum http_response_parser_state { VERSION, STATUS, DESCRIPTION, HEADER_KEY, HEADER_VALUE, BODY, OK };
	enum http_proxy_enum { NONE, HTTP_PROXY, HTTPS_PROXY};
	enum http_type_enum { PLAIN_HTTP, SSL_HTTPS};

	http_proxy_enum http_proxy;
	http_type_enum http_type;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ssl::context ctx;
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> sslsocket;



};

#endif // HTTP_REQUEST_H
