#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

// Boost
#include <boost/asio.hpp>


#include <map>
#include <string>
#include "http_request.h"

/** @file */

/*! \brief Basic Memory Only HTTP Result Class
 * 
 * This is a basic http_response class that is used to store the results of a HTTP Transfer in memory and pass the results back to the client
 * The results of a transfer are sent back to the caller as either a parameter in the http_request::setCallback function, or via a boost::promise via the http_request::Status variable.
 * Regardless of the success or failure of a HTTP Transfer, the http_response class is always sent back. Clients can check the http_request::getStatus() function to retrieve a status 
 * value that is either a HTTP response code (eg, 200 for a successful transfer) or a negative number in case of internal library error
 *
 * Classes that need to implement additional features should inherit this class as the base class.
 */
class http_response
{
friend http_request
public:

	http_response();
	~http_response();
	void clear();
	void reset();
	std::string getVersion();
	int getStatus();
	std::string getDescription();
	std::map<std::string, std::string>::iterator getHeadersBegin();
	std::map<std::string, std::string>::iterator getHeadersEnd();
	std::string getHeader(std::string);
	int getBodySize();
	std::string getBody();
protected:
	void setVersion(std::string);
	void setStatus(int);
	void setDescription(std::string);
	void setHeaders(std::string, std::string);
	void setBodySize(int);
	void setBody(std::string);
	void setBodyAppend(char);


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
