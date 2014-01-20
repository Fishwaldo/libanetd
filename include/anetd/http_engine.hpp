#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
/*
 * include file for http_engine Class in libanetd
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


#include "http_response.hpp"

/** @file */


namespace DynamX {
	namespace anetd {

		/*! \mainpage HTTP Client
		 *
		 * \section introduction Introduction
		 * This is the home of the anetd Library and Command Line Tool
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
		 * Example Code that demonstrates how to use the http_engine class with the standard http_response class.
		 * This example demonstrates using the class in both Callback Mode as well as via the a boost::promise polling.
		 *
		 * \link httpclient.cpp \endlink
		 *
		 * \section seealso See Also
		 *
		 * The http code makes extensive use of the boost::asio, boost::thread (for threading and promise support) boost::algorithm, boost::regex and boost::date_time classes
		 * You need to link your application against the relevent boost libraries in order to fully support the http_engine class
		 *
		 * The Homepage and Further Development Information can be found here: <a href=http://wiki.my-ho.st/confluence/display/HTC/anetd+Home>http://wiki.my-ho.st/confluence/display/HTC/anetd+Home</a>
		 * \example httpclient.cpp
		 * This is a example of using the http_engine in both Callback and boost::unique_future mode
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
		 * \namespace DynamX::anetd
		 * \brief the Main Namespace for the anetd Library
		 *
		 * \namespace DynamX::anetd::Logging
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
		 * \li 2. using a boost::unique_future via the http_engine::Status and either polling for the completion, or using the wait handlers
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
		 *		boost::shared_ptr < http_engine > transmitter_block;
		 *		http_response response;
		 *		// Initialize the transmitter block.
		 *		transmitter_block = boost::shared_ptr < http_engine > (new http_engine (&response, &io));
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
		 *		boost::shared_ptr < http_engine > transmitter_block;
		 *		http_response response;
		 *		// Initialize the transmitter block.
		 *		transmitter_block = boost::shared_ptr < http_engine > (new http_engine (&response, &io));
		 *		transmitter_block->Starttransfer (server);
		 *		// Wait for the transfer to complete
		 *		transmitter_block->Status.wait();
		 *		std::cout << transmitter_block->Status.get()->getStatus();
		 * 	 }
		 *
		 *
		 * \endcode
		 */

		class http_engine
		{
			public:
				/*! \brief Constructor
				 *
				 * Create and Initialize the http_engine classes. Takes one paramaters.
				 *
				 * @param[in] postback this is the IO Service to use to post the callback on (the callback will run on threads that are calling the run() method on this IO Service
				 */
				http_engine(boost::asio::io_service *postback);
				/*! \brief Destructor
				 *
				 * Destruct the http_engine Class
				 */
				~http_engine();
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
				bool Starttransfer(http_response *myresponse);
				/*! \brief Typedef of the Callback Function
				 *
				 * This is the typedef of the Callback Function. It accepts one arguement which is
				 * a pointer to the http_response class that was passed in the http_engine::http_engine constructor
				 */
				typedef boost::function<void (http_response *)> t_callbackFunc;
				/*! \brief Set a Callback Function
				 *
				 * This sets a Callback Function that will be called when the request completes (regardless of success or failure)
				 * The Callback Function will be executed on any thread that is calling boost::asio::io_service::run() on the postback IO service passed in the http_engine::http_engine constructor
				 *
				 * @param[in] func The Function to call when the request has completed. Function Signature must match the http_engine::t_callbackFunc typedef
				 */
				bool setCallback(t_callbackFunc); /**< a Member Function. Details */
				/*! \brief a Boost::unique_future to indicate when the request has completed.
				 *
				 * a boost::unique_future to indicate when the request has completed. Applications can poll the unique_future to see when the request is finished, and then use the .get() function to retrieve
				 * the http_response class with the results of the request.
				 */
				boost::unique_future<http_response*> Status;

				/*! \brief set the Username and Password to use to authenticate to a HTTP Proxy
				 *
				 * sets the username and password to use to authenticate to a HTTP Proxy
				 *
				 * @param username the username to use
				 * @param password the password to use
				 * @return a bool indicating success or failure
				 */
				bool setProxyAuth(std::string username, std::string password);

				/*! \brief Return if the Transfer has completed (does not indicate errors though)
				 *
				 * Returns if the transfer has complete, but does not indicate if its a successfull transfer
				 * Check the response class for the actual Transfer Status
				 *
				 * @return a bool indicating Success or Failure
				 */
				bool getCompletion();

				class connection_exception: public std::exception { };
				class server_connection_exception: public std::exception { };
				class policy_file_request_exception: public std::exception { };
				private:
				size_t socksend(std::string data);
				size_t sockget(char *data, size_t size);
#if BOOST_VERSION > 104700
				bool verify_callback(bool preverified, boost::asio::ssl::verify_context &vctx);
#endif
				bool parse_status(int status);
				bool parse_header();

				bool send();
				bool receive();
				void disconnect();
				void callback(http_response *res);
				void clear();
				http_response *connect();

				std::string method;
				std::string host;
				std::string url;
				std::string targeturl;
				std::string version;
				int status;
				std::string description;
				std::map<std::string, std::string> arguments;
				std::pair<std::string, std::string> proxyauth;
				std::string body;
				http_response *response;
				t_callbackFunc CallbackFunction;
				enum http_response_parser_state { ANETD_VERSION, ANETD_STATUS, ANETD_DESCRIPTION, ANETD_HEADER_KEY, ANETD_HEADER_VALUE, ANETD_BODY, ANETD_OK };
				enum http_proxy_enum { NONE, HTTP_PROXY, HTTPS_PROXY};
				enum http_type_enum { PLAIN_HTTP, SSL_HTTPS};
				int redirtimes;

				http_proxy_enum http_proxy;
				http_type_enum http_type;
				boost::asio::io_service io;
				boost::asio::ip::tcp::socket socket;
				boost::asio::ssl::context ctx;
				boost::asio::ssl::stream<boost::asio::ip::tcp::socket> sslsocket;
				boost::asio::io_service *postbackio;
		};


	}
}

#endif // HTTP_REQUEST_H
