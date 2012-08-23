#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/optional.hpp>

#include <boost/shared_array.hpp>

#include <boost/bind.hpp>
#include <boost/date_time.hpp>

#include <boost/interprocess/detail/move.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp> 
#include <boost/serialization/string.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <exception>
#include <map>
#include <string>
#include <vector>

#include "pointer_utils.h"
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
	enum http_response_parser_state { VERSION, STATUS, DESCRIPTION, HEADER_KEY, HEADER_VALUE, BODY, OK };
	boost::asio::ip::tcp::socket socket;



};

#endif // HTTP_REQUEST_H
