#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include "http_request.h"

using namespace DynamX::HttpClient;
using namespace DynamX::HttpClient::Logging;
using namespace boost::archive::iterators;

typedef insert_linebreaks<base64_from_binary<transform_width<std::string::const_iterator,6,8> >, 72 > it_base64_t;

std::string Base64Encode(std::string s) {
	std::string base64(it_base64_t(s.begin()),it_base64_t(s.end()));
	base64.append((3-s.length()%3)%3, '=');
	return base64;
}


std::string get_env(std::string const& name)
{
   char* var = getenv(name.c_str());
   //getenv can return NULL, std::string(NULL) throws
   if(var) return var;
   else return "";
}


http_request::http_request(http_response *myresponse, boost::asio::io_service *postback) : io(), socket(this->io), ctx(boost::asio::ssl::context::sslv23), sslsocket(this->io, ctx) {
	this->response = myresponse;
    this->reset();
	this->postbackio = postback;
}

http_request::~http_request()
{
}

void http_request::clear()
{
	this->disconnect();
	this->method = "";
	this->url = "";
	this->version = "";
	this->host = "";
	this->arguments.clear();
	this->http_proxy = NONE;
	this->http_type = PLAIN_HTTP;
}

void http_request::reset()
{
	this->disconnect();
	this->method = "GET";
	this->url = "/";
	this->host = "";
	this->version = "HTTP/1.0";
	this->arguments.clear();
	this->response->reset();
	this->http_proxy = NONE;
	this->http_type = PLAIN_HTTP;
}


size_t http_request::socksend(std::string data) {
	switch (this->http_type) {
		case PLAIN_HTTP:
			return this->socket.send(boost::asio::buffer(data.c_str(), data.length()));
			break;
		case SSL_HTTPS:
			return this->sslsocket.write_some(boost::asio::buffer(data.c_str(), data.length()));
			break;
	}
}

size_t http_request::sockget(char *data, size_t size) {
	switch (this->http_type) {
		case PLAIN_HTTP:
			return this->socket.read_some(boost::asio::buffer(data, size));
			break;
		case SSL_HTTPS: {
				boost::system::error_code es;
				size_t retsize;
				retsize = this->sslsocket.read_some(boost::asio::buffer(data,size), es);
				if (es.category() == boost::asio::error::get_ssl_category() && es.value() != ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ)) {
					return retsize;
				}
				if (retsize == 0) {
					throw boost::system::system_error(boost::asio::error::eof);
				}
				return retsize;
			}
			break;
	}
}

void http_request::send()
{
	headers["Content-Length"] = boost::lexical_cast<std::string>(body.length());

	std::string request;
	switch (this->http_proxy) {
		case NONE:
			request = this->method + ' ' + this->url;
			break;
		case HTTP_PROXY:
		case HTTPS_PROXY:
			request = this->method + ' ' + this->host + this->url;
			break;
	}
	if (arguments.begin() != arguments.end())
	{
		request += '?';
		bool first = true;
		for (std::map<std::string, std::string>::iterator argument = this->arguments.begin(); argument != this->arguments.end(); ++argument)
		{
			std::vector<std::string> values;
			boost::split(values, argument->second, boost::is_any_of(", "));
			for (std::vector<std::string>::iterator value = values.begin(); value != values.end(); ++value)
			{
				if (!first)
					request += '&';
				else
					first = false;
				request += argument->first + '=' + *value;
			}
		}
	}
	request += ' ' + this->version + "\r\n";
	/* send the headers if needed */
	for (std::map<std::string, std::string>::iterator header = this->headers.begin(); header != this->headers.end(); ++header)
		request += header->first + ": " + header->second + "\r\n";
	/* if we have a username, password stored, send that */
	if (this->httpauth.first.length()>0)
		request += "Authorization: Basic " + Base64Encode(this->httpauth.first +":"+this->httpauth.second) + "\r\n";
	if ((this->http_proxy == HTTP_PROXY) && (this->proxyauth.first.length() > 0))
		request += "Proxy-Authorization: Basic " + Base64Encode(this->proxyauth.first + ":" + this->proxyauth.second) + "\r\n";

	request += "\r\n" + this->body;
	LogTrace() << "Sending: " << request;
	this->socksend(request);
	this->receive();
}

void http_request::send(std::string absolute_url)
{

	// Parse the URL.
	std::vector<std::string> url_parts;
	boost::regex url_expression(
		// protocol            host               port
		"^(\?:([^:/\?#]+)://)\?(\\w+[^/\?#:]*)(\?::(\\d+))\?"
		// path                file       parameters
		"(/\?(\?:[^\?#/]*/)*)\?([^\?#]*)\?(\\\?(.*))\?"
		);
	boost::regex_split(std::back_inserter(url_parts), absolute_url, url_expression);
	std::string host = url_parts[1];
	std::string port = url_parts[2];

	this->url = url_parts[3] + url_parts[4];

	// Add the 'Host' header to the request. Not doing this is treated as bad request by many servers.


	// Use the empty path if no path is specified.
	if (this->url.empty())
		this->url = "/";
	boost::algorithm::to_lower(url_parts[0]);
	/* check for Proxy Server */
	if (url_parts[0] == "http") {
		this->http_type = PLAIN_HTTP;
		if (port.empty())
			port = "80";
		this->host = url_parts[0] + "://" + url_parts[1] + ":" + (url_parts[2].empty() ? "80" : url_parts[2]);

		this->targeturl = url_parts[1] + ":" + port;

		this->headers["Host"] = host;

		std::string proxy = get_env("http_proxy");
		if (proxy != "") {
			std::vector<std::string> proxy_parts;
			boost::regex proxy_expression(
				// protocol            host               port
				"^(\?:([^:/\?#]+)://)\?(\\w+[^/\?#:]*)(\?::(\\d+))\?"
			);
			boost::regex_split(std::back_inserter(proxy_parts), proxy, proxy_expression);
			LogDebug() << "Connecting Via HTTP Proxy at: "<< proxy_parts[0] << "://" << proxy_parts[1] << ":" << proxy_parts[2];
			host = proxy_parts[1];
			port = proxy_parts[2];
			this->http_proxy = HTTP_PROXY;
		}
	} else if (url_parts[0] == "https") {
		this->http_type = SSL_HTTPS;
		if (port.empty())
			port = "443";
		char *data = new char[1024];
		this->host = url_parts[0] + "://" + url_parts[1] + ":" + (url_parts[2].empty() ? "443" : url_parts[2]);

		this->targeturl = url_parts[1] + ":" + port;

		this->headers["Host"] = host;

		std::string proxy = get_env("https_proxy");
		if (proxy != "") {
			std::vector<std::string> proxy_parts;
			boost::regex proxy_expression(
				// protocol            host               port
				"^(\?:([^:/\?#]+)://)\?(\\w+[^/\?#:]*)(\?::(\\d+))\?"
			);
			boost::regex_split(std::back_inserter(proxy_parts), proxy, proxy_expression);
			LogDebug() << "Connecting Via HTTPS Proxy at: "<< proxy_parts[0] << "://" << proxy_parts[1] << ":" << proxy_parts[2];
			host = proxy_parts[1];
			port = proxy_parts[2];
			this->http_proxy = HTTP_PROXY;
		}
	}





	// Resolve the hostname.
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), host, port);
	boost::asio::ip::tcp::resolver::iterator iterator;
	try
	{
		iterator = resolver.resolve(query);
	}
	catch (boost::system::system_error&)
	{
		throw connection_exception();
	}

	// Try to connect to the server using one of the endpoints.
	bool connected = false;
	for (iterator; iterator != boost::asio::ip::tcp::resolver::iterator(); ++iterator)
	{
		boost::asio::ip::tcp::endpoint endpoint = iterator->endpoint();
		try
		{
			switch (this->http_type) {
				case PLAIN_HTTP:
					this->socket.connect(endpoint);
					break;
				case SSL_HTTPS:
					this->sslsocket.set_verify_mode(boost::asio::ssl::context::verify_none);
					this->sslsocket.set_verify_callback(boost::bind(&http_request::verify_callback, this, _1, _2));
					this->sslsocket.lowest_layer().connect(endpoint);
					/* if we are connecting through a proxy.... */
					if (this->http_proxy == HTTP_PROXY) {
						/* send our Proxy Headers */
						std::string proxycmd = "CONNECT " + this->targeturl + " " + this->version + "\r\n";
						if (this->proxyauth.first.length() > 0)
							proxycmd += "Proxy-Authorization: Basic " + Base64Encode(this->proxyauth.first + ":" + this->proxyauth.second) + "\r\n";
						proxycmd += "\r\n";
						LogTrace() << "Proxy Command: " << proxycmd;
						this->sslsocket.next_layer().send(boost::asio::buffer(proxycmd.c_str(), proxycmd.length()));
						char *data = new char[1024];
						while (this->sslsocket.next_layer().read_some(boost::asio::buffer(data, 1024))) {
							std::string proxyresponse(data);
							//typedef  split_vector;
							std::vector<std::string> SplitResponse;
							boost::is_equal testequal;
							boost::algorithm::split(SplitResponse, proxyresponse, boost::algorithm::is_space(), boost::algorithm::token_compress_on);
							//boost::algorithm::split(SplitResponse, proxyresponse, testequal(" "));
							if (SplitResponse[1] == "200") {
								break;
							} else {
								throw;
							}
						}
					}
					this->sslsocket.handshake(boost::asio::ssl::stream_base::client);

					break;
			}
			connected = true;
			break;
		}
		catch (boost::system::system_error& ec)
		{
		 LogCritical() << "Error Connecting to " << this->url << ": "<< ec.what();
		}
	}
	// Check if the connection is successful.
	if (!connected)
		throw connection_exception();

	// Send the request.
	this->send();

}

bool http_request::verify_callback(bool preverified, boost::asio::ssl::verify_context &vctx) {
	   char subject_name[256];
	        X509* cert = X509_STORE_CTX_get_current_cert(vctx.native_handle());
	        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
	        LogDebug() << "Verifying Certificate: " << subject_name;

	        return preverified;

}

void http_request::receive()
{
	this->response->reset();

	http_response_parser_state parser_state = VERSION;

	size_t buffer_size = 2048;
	char* buffer = new char[buffer_size];
	std::string status = "";
	std::string key = "";
	std::string value = "";

	try
	{
		do
		{
			int bytes_read = this->sockget(buffer, buffer_size);
			char* position = buffer;
			do
			{
				switch (parser_state)
				{
				case VERSION:
					if (*position != ' ')
						status += *position++;
					else
					{
						position++;
						parser_state = STATUS;
                        this->response->setVersion(status);
                        status = "";
					}
					break;
				case STATUS:
					if (*position != ' ')
						status += *position++;
					else
					{
						this->response->setStatus( boost::lexical_cast<int>(status) );
						position++;
						parser_state = DESCRIPTION;
					}
					break;
				case DESCRIPTION:
					if (*position == '\r')
						position++;
					else if (*position != '\n')
						status += *position++;
					else
					{
						position++;
						key = "";
						this->response->setDescription(status);
						status = "";
						parser_state = HEADER_KEY;
					}
					break;
				case HEADER_KEY:
					if (*position == '\r')
						position++;
					else if (*position == '\n')
					{
						position++;
						std::string ContentLength = this->response->getHeader("Content-Length");
						if (ContentLength  != "" )
						{
							this->response->setBodySize(boost::lexical_cast<int>(ContentLength));
							parser_state = (this->response->getBodySize() == 0) ? OK : BODY;
						}
						else
						{
						        this->response->setBodySize(0);
							parser_state = BODY;
						}
					}
					else if (*position == ':')
						position++;
					else if (*position != ' ')
						key += *position++;
					else
					{
						position++;
						value = "";
						parser_state = HEADER_VALUE;
					}
					break;
				case HEADER_VALUE:
					if (*position == '\r')
						position++;
					else if (*position != '\n')
						value += *position++;
					else
					{
						position++;
						LogTrace() << boost::this_thread::get_id() << "Header key: " << key << " value: " << value;
                        this->response->setHeaders(key, value);
						key = "";
						parser_state = HEADER_KEY;
					}
					break;
				case BODY:
					this->response->setBody(*position++);
					if (this->response->getBody().length() == this->response->getBodySize())
						parser_state = OK;
					break;
				case OK:
					position = buffer + bytes_read;
					break;
				}
			} while (position < buffer + bytes_read);

		} while (parser_state != OK);

	}
	catch (boost::system::system_error& e)
	{
		if (e.code() == boost::asio::error::eof) {
			this->response->setBodySize(this->response->getBody().length());
		} else
		{
			delete buffer;
			throw;
		}
	}
	catch (...)
	{
		delete buffer;
		throw;
	}
	delete buffer;
}

void http_request::disconnect()
{
	switch (this->http_type) {
		case PLAIN_HTTP:
			if (socket.is_open())
			{
				try
				{
					socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
					socket.close();
				}
				catch (boost::system::system_error& ec) {
					LogWarn() << "Shutdown error: " << ec.what();
				}
			}
			break;
		case SSL_HTTPS:
			if (this->sslsocket.lowest_layer().is_open()) {
				try {
					boost::system::error_code se;
					if (this->sslsocket.shutdown(se)) {
						if (se.category() == boost::asio::error::get_ssl_category() && se.value() != ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ)) {
							throw se;
						}
					}
					this->sslsocket.lowest_layer().close();
				} catch (boost::system::system_error& ec) {
					LogWarn() << "Shutdown error: " << ec.what();

				}
			}
			break;
	}
}

http_response *http_request::connect(std::string url)
{
	sleep(2);
	//http_request request(this->response,this->io_service);
	int redirtimes = 0;

	while ((this->response->getStatus() < 200) || (this->response->getStatus() >= 300)) {
		// Send the HTTP request and receive the this->response.
		this->send(url);
		if ((this->response->getStatus() >= 300) && (this->response->getStatus() < 400)) {
			/* Redirect */
			if (redirtimes++ > 5) {
				this->disconnect();
				LogCritical() << "Redirection Failure. Redirected too many times";
				return this->response;
			}
			std::string Location = this->response->getHeader("Location");
			if (Location != "") {
				LogTrace() << "Redirecting (" << this->response->getStatus() << ") to " << Location;
				url = Location;
				this->reset();
			} else {
				LogCritical() << "Redirection Failure (" << this->response->getStatus() << "). No Location Specified";
				return this->response;
			}
		} else if ((this->response->getStatus() >= 400) && (this->response->getStatus() < 500)) {
			LogCritical() << "Not Found (" << this->response->getStatus() << ") Error";
			this->disconnect();
			return this->response;
		} else if ((this->response->getStatus() >= 500) && (this->response->getStatus() < 600)) {
			LogCritical() << "Server Error (" << this->response->getStatus() << ") Error";
			this->disconnect();
			return this->response;
		}
	}
	LogDebug() << "Response: " << this->response->getStatus();

	//LogTrace() << "Body: " << this->response->getBody();
	this->postbackio->post(boost::bind(&http_request::callback, this, this->response));
	return this->response;
}

bool http_request::Starttransfer(std::string url) {
	boost::packaged_task<http_response *> TransferStatus(boost::bind(&http_request::connect, this, url));
	this->Status = TransferStatus.get_future();
	boost::thread t(boost::move(TransferStatus));
	return true;
}
bool http_request::setCallback(t_callbackFunc func) {
	this->CallbackFunction = func;
}

void http_request::callback(http_response *res) {
	if (this->CallbackFunction)
		CallbackFunction(res);
}

bool http_request::setHeader(std::string name, std::string value) {
	if (this->headers.find(name) == this->headers.end())
		return false;
	std::pair<std::map<std::string, std::string>::iterator,bool> ret = this->headers.insert(std::pair<std::string, std::string>(name, value));
	return ret.second;
}
bool http_request::setHTTPAuth(std::string username, std::string password) {
	this->httpauth.first = username;
	this->httpauth.second = password;
	return true;
}
bool http_request::setProxyAuth(std::string username, std::string password) {
	this->proxyauth.first = username;
	this->proxyauth.second = password;
	return true;
}
