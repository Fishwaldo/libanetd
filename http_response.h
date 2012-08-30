#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <boost/thread/mutex.hpp>



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
 * Unless specifically mentioned as ThreadSafe in the member function descriptions, you should not call any of these functions from a client application till the transfer is complete.
 *
 * Classes that need to implement additional features should inherit this class as the base class.
 */
class http_response
{
public:
	/*! \brief Default Constructor
	 *
	 */
	http_response();
	/*! \brief Default Deconstructor
	 *
	 */
	~http_response();
	/*! \brief Reset the Response Class to defaults
	 *
	 *  This function is called by the http_request class to reset this class to defaults. This may be due to restarting the download from a HTTP Redirect, or other actions that require a new request to be sent to the server.
	 *
	 *  If subclassing this function, you should ensure that you your own class calls this base class, as well as reseting any variables that your own class may implement.
	 *
	 */
	void reset();
	/*! \brief get the HTTP Version that the server sent us when responding to our request.
	 *
	 * 	Calling this function anytime during the transfer is ThreadSafe
	 *
	 * @return a std::string with the HTTP Version string. eg "HTTP\1.0"
	 */
	std::string getVersion();
	/*! \brief get the Status of the Transfer.
	 *
	 * 	Returns either a HTTP Server status number, or a negative number indicating a internal library error.
	 * 	the HTTP Server Status number is usually in the range of 200-299 for successful transfers and anything else can indicate a error
	 * 	If the Library encounters a error (such as the connection times out) then the returned number will be negative.
	 *
	 * @return a int with the status number. Test for 200 for successful transfer.
	 */
	int getStatus();
	/*! \brief get the Description String that the Server return.
	 *
	 *  The description string is usually contained after the status number in HTTP protocol and is a textual representation of the request status.
	 *
	 * @return a string with the description field.
	 */
	std::string getDescription();
	/*! \brief get a Iterator to the head of a std::map of headers returned by the server
	 *
	 *  This function returns a Iterator to the start of a internal std::map that will allow a library to iterate over all headers that might have been returned by the HTTP Server or Web Site
	 *
	 * @return a std::map<std::string, std::string>::iterator to the start of the Map. The Key is the header name, and the value is the header value returned.
	 */
	std::map<std::string, std::string>::iterator getHeadersBegin();
	/*! \brief get a Iterator to the tail of a std::map of headers returned by the server
	 *
	 *  This function returns a Iterator to the end of a internal std::map that will allow a library to iterate over all headers that might have been returned by the HTTP Server or Web Site
	 *  See Also: http_response::getHeadersBegin()
	 *
	 * @return a std::map<std::string, std::string>::iterator to the end of the Map. The Key is the header name, and the value is the header value returned.
	 */
	std::map<std::string, std::string>::iterator getHeadersEnd();
	/*! \brief Return the value associated with a Header
	 *
	 *  This returns the value associated with a particular header who´s name is passed as the first param. Returns empty if no such header exists
	 *
	 * @param[in] name The header value to return
	 * @return a std::string containing the header value, or empty if no such header exists.
	 */
	std::string getHeader(std::string Header);
	/*! \brief Return the size of the Body of the HTTP transfer.
	 *
	 *  Returns the size of the body (in bytes) returned as part of the HTTP transfer.
	 * 	Calling this function anytime during the transfer is ThreadSafe
	 *
	 *
	 * @return a size_t representing the size.
	 */
	size_t getBodySize();
	/*! \brief Return the Body of the HTTP transfer as a string
	 *
	 *  Returns the Body of the HTTP Transfer as a string.
	 * 	Calling this function anytime during the transfer is ThreadSafe
	 *
	 * @return a string containing the body of the transfer
	 */
	std::string getBody();
	/*! \brief get the Current Progress of the Transfer
	 *
	 *  returns the size of the data downloaded so far. This can be used as a "progress" indicator for long running downloads if required.
	 *  Calling this function anytime during the transfer is ThreadSafe
	 *
	 * @return the amount of data currently downloaded.
	 */
	size_t getProgress();

protected:
	/*! \brief Set the Version of the HTTP Protocol used in the transfer
	 *
	 * Used by the http_request class only, this function sets the version of the transfer. If you need to perform
	 * some custom action on the data depending upon the version, then you should reimplement this function, but be sure to call this base class function as well.
	 *
	 * @param[in] Version the HTTP Version
	 */
	void setVersion(std::string Version );
	/*! \brief Set the Status of the HTTP Transfer
	 *
	 * Used by the http_request class only, this function sets the status of the transfer. If you need to perform
	 * some custom action on the data depending upon the status, then you should reimplement this function, but be sure to call this base class function as well.
	 *
	 * Postive Numbers are Status Numbers returned from the HTTP Server.
	 *
	 * Negative Numbers indicate a error condition the Library encountered.
	 *
	 * @param[in] Status of the HTTP Transfer
	 */
	void setStatus(int Status);
	/*! \brief Set the Description of the HTTP Protocol Response used in the transfer
	 *
	 * Used by the http_request class only, this function sets the Description of the transfer. If you need to perform
	 * some custom action on the data depending upon the Description, then you should reimplement this function, but be sure to call this base class function as well.
	 *
	 * @param[in] Description of the transfer
	 */
	void setDescription(std::string Description);
	/*! \brief Set a Header returned by the HTTP Server
	 *
	 * Used by the http_request class only, this function is used to store Headers returned by the HTTP Server or Web Site. This includes cookies sent as well.
	 * if you requrie some custom action on the data depending upon a header, then you should reimplement this function, but be sure to call this base class function as well.
	 *
	 * @param[in] Name the name of the header
	 * @param[in] Value the value to store
	 */
	void setHeaders(std::string Name, std::string Value);
	/*! \brief Set the Size of the Body of the HTTP Transfer
	 *
	 * Used by the http_request class only, this function sets the size of the Body of the transfer.
	 *
	 * Note, Some transfers will not send the size at the start of the transfer, so this function can sometimes be called when the http_request class is processing the headers (via the Content-Length Header)
	 * or at the end of the transfer after the library has completed the transfer.
	 *
	 * If you need to perform some custom action on the data depending upon the version, then you should reimplement this function, but be sure to call this base class function as well.
	 *
	 * @param[in] size the HTTP Version
	 */
	void setBodySize(size_t Size);
	/*! \brief Append a portion of the Body to the response_class result
	 *
	 * Used by the http_request class only, this function passes the current portion of the Body that the http_request class is processing. Depending up how the transfer happens, the
	 * http_request class can either pass in the Body via this function, or by the setBody(char c) member function.
	 * If you need to perform some custom action on the data depending upon the version, then you should reimplement this function, but be sure to call this base class function as well.
	 * 	Calling this function anytime during the transfer is ThreadSafe
	 *
	 * @param[in] Body a portion of the Body returned by the server.
	 */
	void setBody(std::string Body);
	/*! \brief Append a character of the Body to the response_class result
	 *
	 * Used by the http_request class only, this function passes the current character of the Body that the http_request class is processing. Depending up how the transfer happens, the
	 * http_request class can either pass in the Body via this function, or by the setBody(std::string Body) member function.
	 * If you need to perform some custom action on the data depending upon the version, then you should reimplement this function, but be sure to call this base class function as well.
	 * 	Calling this function anytime during the transfer is ThreadSafe
	 *
	 * @param[in] c a character of the Body returned by the server.
	 */
	void setBody(char c);
	friend class http_request;

private:
	std::string version;
	int status;
	std::string description;
	std::map<std::string, std::string> headers;
	size_t body_size;
	size_t progress;
	std::string body;
	std::map<std::string, std::string> cookies;
	boost::mutex TLock;
};

#endif // HTTP_RESPONSE_H
