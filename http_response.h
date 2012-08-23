#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

// Boost
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp> 
#include <boost/serialization/string.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <map>
#include <string>

#include "pointer_utils.h"

class http_response
{
public:
	http_response();
	http_response(boost::asio::ip::tcp::socket& socket);
	~http_response();
	void clear();
	void reset();
	
	void setVersion(std::string);
	std::string getVersion();
	void setStatus(int);
	int getStatus();
	void setDescription(std::string);
	std::string getDescription();
	void setHeaders(std::string, std::string);
	std::map<std::string, std::string>::iterator getHeadersBegin();
	std::map<std::string, std::string>::iterator getHeadersEnd();
	std::string getHeader(std::string);
	void setBodySize(int);
	int getBodySize();
	void setBody(std::string);
	void setBodyAppend(char);
	std::string getBody();


private:
	std::string version;
	int status;
	std::string description;
	std::map<std::string, std::string> headers;
	int body_size;
	std::string body;
	std::map<std::string, std::string> cookies;


};

#endif // HTTP_RESPONSE_H
