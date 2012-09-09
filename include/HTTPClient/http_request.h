#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
/*
 * include file for http_request Class in libHTTPClient
 * Copyright (C) 2012 Justin Hammond
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
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

/** @file */


namespace DynamX {
namespace HttpClient {

/*! \mainpage HTTP Client
 *
 * \section introduction Introduction
 * This is the home of the HttpClient Library and Command Line Tool
 *
 * This library implements a multithreaded http client that can be used via the command line, or embeded into other applications. It leverages the Boost Library for Multi Threaded and Network Support (boost::thread and boost::asio) and currently supports HTTP 1.0
 *
 * Eventually, its planned to support a subset of HTTP 1.1 (Chunked Transfers), and HTTPS protocols as well as honoring System Configured Proxy Servers. Its aimed to be easily embeddable into existing applications with a simple API and a response class that allows the developer to customize what to do with the result that the client recieves. for example:
 *
 * \li Save the result in Memory, with all associated headers and pass the result back to the application
 * \li Save the result into a file, and pass the headers and filename back to the application
 * \li Post-Process the RAW data (eg, XML) and do necessary decoding in a seperate thread before passing back the decoded data to the application.
 *
 * \section example Example Code
 * Example Code that demonstrates how to use the http_request class with the standard http_response class.
 * This example demonstrates using the class in both Callback Mode as well as via the a boost::promise polling.
 *
 * \link httpclient.cpp \endlink
 *
 * \section seealso See Also
 *
 * The http code makes extensive use of the boost::asio, boost::thread (for threading and promise support) boost::algorithm, boost::regex and boost::date_time classes
 * You need to link your application against the relevent boost libraries in order to fully support the http_request class
 *
 * The Homepage and Further Development Information can be found here: <a href=http://wiki.my-ho.st/confluence/display/HTC/HttpClient+Home>http://wiki.my-ho.st/confluence/display/HTC/HttpClient+Home</a>
 * \example httpclient.cpp
 * This is a example of using the http_request in both Callback and boost::unique_future mode
 */

/*! \namespace boost::asio
 *  \brief The Boost ASIO Classes
 *
 * The Boost ASIO Class Documentation can be read here: <a href=http://www.boost.org/libs/asio/>htpp://www.boost.org/libs/asio/</a>
 *
 * \namespace boost::thread
 * \brief the Boost Threading Classes
 *
 * The Boost Thread Class Documentation can be read here: <a href=http://www.boost.org/libs/thread/>http://www.boost.org/libs/thread/</a>
 *
 * \namespace boost::algorithm
 * \brief the Boost Algorithm Classes
 *
 * The Boost Algorithm Class Documentation can be read here: <a href=http://www.boost.org/libs/algoritm/>http://www.boost.org/libs/algorithm/</a>
 *
 * \namespace boost::regex
 * \brief the Boost Regular Expressions Classes
 *
 * The Boost Regular Expression Class Documentation can be read here: <a href=http://www.boost.org/libs/regex/>http://www.boost.org/libs/regex/</a>
 *
 * \namespace boost::date_time
 * \brief the Boost Date/Time Classes
 *
 * The Boost Date and Time Class Documentation can be read here: <a href=http://www.boost.org/libs/datetime/>http://www.boost.org/libs/datetime/</a>
 *
 * \namespace DynamX::HttpClient
 * \brief the Main Namespace for the HTTPClient Library
 *
 * \namespace DynamX::HttpClient::Logging
 * \brief the Logging Namespace for the Library
 */

class http_response;



/*! \brief This is the main Class that Controls all downloads.
 *
 * This is the main class used to initiate all downloads. The constructor accepts the Response Object (that will contain the results) and a
 * IO Service Object (From boost::asio) that will be used to execute a callback when the request completes.
 *
 * There are two implementations possible.
 * \li 1. Using a Callback function (via the setCallback() function) when the download completes
 * \li 2. using a boost::unique_future via the http_request::Status and either polling for the completion, or using the wait handlers
 *
 * An Example of using the Callback method:
 * \code
 * 	 void HTTPCallback(http_response *response) {
 *		// Process the response pointer here to find out if successful
 *		...
 *	 }
 *
 *	 void main() {
 *		boost::asio::io_service io;
 *		boost::shared_ptr < http_request > transmitter_block;
 *		http_response response;
 *		// Initialize the transmitter block.
 *		transmitter_block = boost::shared_ptr < http_request > (new http_request (&response, &io));
 *		transmitter_block->setCallback(boost::bind(HTTPCallback, _1));
 *		transmitter_block->Starttransfer (server);
 *		// Assuming you do other stuff here, and add work to the io service
 *		io.run();
 * 	 }
 * \endcode
 * An Example of using the unique_future method:
 * \code
 *	 void main() {
 *		boost::asio::io_service io;
 *		boost::shared_ptr < http_request > transmitter_block;
 *		http_response response;
 *		// Initialize the transmitter block.
 *		transmitter_block = boost::shared_ptr < http_request > (new http_request (&response, &io));
 *		transmitter_block->Starttransfer (server);
 *		// Wait for the transfer to complete
 *		transmitter_block->Status.wait();
 *		std::cout << transmitter_block->Status.get()->getStatus();
 * 	 }
 *
 *
 * \endcode
 */

class http_request
{
public:
	/*! \brief Constructor
	 *
	 * Create and Initialize the http_request classes. Takes two paramaters
	 *
	 * @param[in] response This is the class to store the results into and pass back via either the callback or unique_future interface
	 * @param[in] postback this is the IO Service to use to post the callback on (the callback will run on threads that are calling the run() method on this IO Service
	 */
	http_request(http_response *response, boost::asio::io_service *postback);
	/*! \brief Constructor
	 *
	 * Create and Initialize the http_request classes. Takes one paramaters. The response object is automatically created
	 *
	 * @param[in] postback this is the IO Service to use to post the callback on (the callback will run on threads that are calling the run() method on this IO Service
	 */
	http_request(boost::asio::io_service *postback);
	/*! \brief Destructor
	 *
	 * Destruct the http_request Class
	 */
	~http_request();
	/*! \brief Reset the Requests to prepare for a new transfer
	 *
	 * Resets the Class back to defaults to prepare for a new HTTP transfer
	 * New Transfers will use the existing http_response class passed in the constructor
	 */
	void reset();
	/*! \brief Start the Transfer from a URL
	 *
	 * Starts a new transfer (in a new Thread) from the passed URL.
	 * The URL can be either a http or https site
	 *
	 * @param[in] url a string containing the full URL of the resource to be downloaded
	 * \return Success or Failure of starting the transfer.
	 */
	bool Starttransfer(std::string url);
	/*! \brief Typedef of the Callback Function
	 *
	 * This is the typedef of the Callback Function. It accepts one arguement which is
	 * a pointer to the http_response class that was passed in the http_request::http_request constructor
	 */
	typedef boost::function<void (http_response *)> t_callbackFunc;
	/*! \brief Set a Callback Function
	 *
	 * This sets a Callback Function that will be called when the request completes (regardless of success or failure)
	 * The Callback Function will be executed on any thread that is calling boost::asio::io_service::run() on the postback IO service passed in the http_request::http_request constructor
	 *
	 * @param[in] func The Function to call when the request has completed. Function Signature must match the http_request::t_callbackFunc typedef
	 */
	bool setCallback(t_callbackFunc); /**< a Member Function. Details */
	/*! \brief a Boost::unique_future to indicate when the request has completed.
	 *
	 * a boost::unique_future to indicate when the request has completed. Applications can poll the unique_future to see when the request is finished, and then use the .get() function to retrieve
	 * the http_response class with the results of the request.
	 */
	boost::unique_future<http_response*> Status;
	/*! \brief set a Header to send to the Server when a request is made
	 *
	 * a function to set a custom header to send to the Server when a request is made
	 *
	 * \param[in] name the name of the header to send
	 * \param[in] value the value of the header to send
	 * \return a bool indicating success
	 */
	bool setHeader(std::string name, std::string value);
	/*! \brief set the Username/Password to use to authenticate to the HTTP Server
	 *
	 * sets the username and password to use to authenticate to the HTTP Server
	 *
	 * @param username the username to use
	 * @param password the password to use
	 * @return a bool indicating success or failure
	 */
	bool setHTTPAuth(std::string username, std::string password);

	/*! \brief set the Username and Password to use to authenticate to a HTTP Proxy
	 *
	 * sets the username and password to use to authenticate to a HTTP Proxy
	 *
	 * @param username the username to use
	 * @param password the password to use
	 * @return a bool indicating success or failure
	 */
	bool setProxyAuth(std::string username, std::string password);
	class connection_exception: public std::exception { };
	class server_connection_exception: public std::exception { };
	class policy_file_request_exception: public std::exception { };
private:
	size_t socksend(std::string data);
	size_t sockget(char *data, size_t size);
#if BOOST_VERSION > 104700
	bool verify_callback(bool preverified, boost::asio::ssl::verify_context &vctx);
#endif
	void send();
	void send(std::string absolute_url);
	void receive();
	void disconnect();
	void callback(http_response *res);
	void clear();
	http_response *connect(std::string url);

	std::string method;
	std::string host;
	std::string url;
	std::string targeturl;
	std::string version;
	int status;
	std::string description;
	std::map<std::string, std::string> arguments;
	std::map<std::string, std::string> headers;
	std::pair<std::string, std::string> httpauth;
	std::pair<std::string, std::string> proxyauth;
	std::string body;
	http_response *response;
	t_callbackFunc CallbackFunction;
	enum http_response_parser_state { VERSION, STATUS, DESCRIPTION, HEADER_KEY, HEADER_VALUE, BODY, OK };
	enum http_proxy_enum { NONE, HTTP_PROXY, HTTPS_PROXY};
	enum http_type_enum { PLAIN_HTTP, SSL_HTTPS};

	http_proxy_enum http_proxy;
	http_type_enum http_type;
	boost::asio::io_service io;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ssl::context ctx;
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> sslsocket;
	boost::asio::io_service *postbackio;
};

namespace Logging {

/*! \brief Return a String with the current timestamp
 */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

inline std::string NowTime()
{
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
            "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in NowTime()";

    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
    return result;
}

#else

#include <sys/time.h>

inline std::string NowTime()
{
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
    return result;
}

#endif //WIN32

/*! \typedef TLogLevels
 * \brief a Enum to indicate the Log Level to Log a message at
 * \relates LogClass
 */
typedef enum TLogLevels {LOG_TRACE, LOG_DEBUG, LOG_WARN, LOG_ERROR, LOG_CRITICAL} TLogLevels;

/*! \brief A Logging Class to log actions from the http_request class
 *
 * This is a generic logging class that takes messages from the http_request class
 * and logs them. The application can either use this class with the Output2StdErr class which will send
 * the output to the std::cerr stream, or can extend Output2StdErr class to write their own
 * output sink.
 */
template <typename T>
class LogClass {
public:
        /*! \brief LogClass Constructor
         *
         * Default Constructor.
         */
	LogClass() {  };
	/*! \brief LogClass Descructor
	 *
	 * Default Destructor. Flushes the output to the Class provided as a template
	 * paramater using the Output2StdErr::Output call.
	 */
	virtual ~LogClass() {
    	    os << std::endl;
    	    T::Output(os.str());
    	}
    	/*! \brief Get the LogCall Output Stream for Logging Messages
    	 *
    	 * returns a output stream to the caller that users can then use to Log Messages
    	 * The Output Stream is not flushed till the LogClass::~LogClass destructor is called (either automatically when it goes out of scope) or by manually calling it
    	 *
    	 * This also prepends any messages, with a timestamp and a log level description
    	 *
    	 * @param[in] level - The Log Level To log the message at
    	 * @return a std::ostringstream that the messages can be logged to
    	 */
    	std::ostringstream& Get(TLogLevels level = LOG_TRACE) {
    	    os << "- " << NowTime();
    	    os << " " << ToString(level) << ": ";
    	    os << std::string(level > LOG_CRITICAL ? level - LOG_CRITICAL : 0, '\t');
    	    return os;
        }
        /*! \brief Set (or Get) the minimum LogLevel to Log At
         *
         * this Sets or gets the LogLevel to Log Messages at
         *
         * @return the Current minimum LogLevel to report at. Defaults to LOG_TRACE
         */
        static TLogLevels& ReportingLevel() {
            static TLogLevels reportingLevel = LOG_TRACE;
            return reportingLevel;
        }
        /*! \brief Get a string representation of a LogLevel
         *
         * Gets a string representation of the LogLevel passed in
         *
         * @param[in] level - The Log Level to get the string representation of
         *
         * @return a string describing the LogLevel
         */
        static std::string ToString(TLogLevels level) {
            static const char* const buffer[] = {"TRACE", "DEBUG", "WARN", "ERROR", "CRITICAL"};
   	    return buffer[level];
        }
        /*! \brief Get a LogLevel that matches the string passed in
         *
         * Returns a LogLevel that matches the string passed in.
         *
         * Usefull for converting human string representations into a level that can be passed to
         * LogLevel::ReportingLevel()
         *
         * if the string can't be matched against a loglevel, then it defaults to LOG_WARN
         *
         * @param[in] level - A string representation to search the LogLevels for
         *
         * @return a Loglevel that matches the string.
         */
        static TLogLevels FromString(const std::string& level) {
            if (level == "TRACE")
                return LOG_TRACE;
            if (level == "DEBUG")
                return LOG_DEBUG;
            if (level == "WARN")
                return LOG_WARN;
            if (level == "ERROR")
                return LOG_ERROR;
            if (level == "CRITICAL")
                return LOG_CRITICAL;
            LogClass<T>().Get(LOG_WARN) << "Unknown logging level '" << level << "'. Using WARN level as default.";
            return LOG_WARN;
        }
protected:
        /*! \brief the LogStream that we Log Against
        */
        std::ostringstream os;
private:
    LogClass(const LogClass&);
    LogClass& operator =(const LogClass&);
};

/*! \brief A Output Sink that logs messages to std::cerr
 *
 * This is a output Sink for the LogClass Class that will output messages to the std::cerr stream
 * Subclass this and pass your new class as a template Param to the definition HTTPLog to implement your own
 * Logging Sink
 */

class Output2StdErr
{
public:
    /*! \brief Open a Stream that can be used to Log Messages
     *
     * Opens a Stream that you want to Log To
     * @return a FILE stream to Log to. Use this in the Output2StdErr::Output function
     */
    static FILE*& Stream() {
        static FILE* pStream = stderr;
        return pStream;
    }
    /*! \brief Called by the LogClass to flush the output stream
     *
     * This is called by the LogClass passing a string to flush to the Logs.
     *
     * @param[in] msg The Message to Log
     */
    static void Output(const std::string& msg) {
    	std::cerr << msg;
    }
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   if defined (BUILDING_HTTPLOG_DLL)
#       define HTTPLOG_DECLSPEC   __declspec (dllexport)
#   elif defined (USING_HTTPLOG_DLL)
#       define HTTPLOG_DECLSPEC   __declspec (dllimport)
#   else
#       define HTTPLOG_DECLSPEC
#   endif // BUILDING_HTTPLOG_DLL
#else
#   define HTTPLOG_DECLSPEC
#endif // _WIN32

/*! \brief The Logging Class that is used to Log Messages to. Used by various Macro's in the http_request class
 * \relates LogClass
 *
 * This is the Actual Implementation of the LogClass with the output sink Output2StdErr already configured
 */
class HTTPLOG_DECLSPEC HTTPLog : public LogClass<Output2StdErr> {};

#define LogFormat() << boost::this_thread::get_id() << ": "  << __FILE__ << ":" <<  __LINE__ << "(" << __FUNCTION__ << "): "

/*! \def LogTrace()
 * \relates LogClass
 * Log a Message at Trace Level
 */
#define LogTrace() if (LOG_TRACE >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_TRACE) LogFormat()
/*! \def LogDebug()
 * \relates LogClass
 * Log a Message at Debug level
 */
#define LogDebug() if (LOG_DEBUG >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_DEBUG) LogFormat()
/*! \def LogWarn()
 * \relates LogClass
 * Log a Message at the Warn Level
 */
#define LogWarn() if (LOG_WARN >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_WARN) LogFormat()
/*! \def LogError()
 * \relates LogClass
 * Log a Message at the LogError Level
 */
#define LogError() if (LOG_ERROR >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_ERROR) LogFormat()
/*! \def LogCritical()
 * Log a Message at the LogCritical Level
 * \relates LogClass
 */
#define LogCritical() if (LOG_CRITICAL >= HTTPLog::ReportingLevel()) HTTPLog().Get(LOG_CRITICAL) LogFormat()

}
}
}

#endif // HTTP_REQUEST_H
