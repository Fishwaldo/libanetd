#include "http_request.h"



http_request::http_request(http_response *myresponse, boost::asio::io_service &io) : socket(io) {
    this->response = myresponse;
    this->reset();
}

http_request::~http_request()
{
}

void http_request::clear()
{
	this->method = "";
	this->url = "";
	this->version = "";
	this->arguments.clear();
}

void http_request::reset()
{
	this->method = "GET";
	this->url = "/";
	this->version = "HTTP/1.0";
	this->arguments.clear();
	this->response->reset();
}


void http_request::send()
{
	headers["Content-Length"] = boost::lexical_cast<std::string>(body.length());

	std::string request = this->method + ' ' + this->url;
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
	for (std::map<std::string, std::string>::iterator header = this->headers.begin(); header != this->headers.end(); ++header)
		request += header->first + ": " + header->second + "\r\n";
	request += "\r\n" + this->body;
	this->socket.send(boost::asio::buffer(request.c_str(), request.length()));
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
	this->headers["Host"] = host;

	// Use the default port if no port is specified.
	if (port.empty())
		port = "80";

	// Use the empty path if no path is specified.
	if (this->url.empty())
		this->url = "/";

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
			this->socket.connect(endpoint);
			connected = true;
			break;
		}
		catch (boost::system::system_error& ec)
		{
		 std::cout << ec.what() << "\n";   
		}
	std::cout << endpoint << "\n";
	}
	// Check if the connection is successful.
	if (!connected)
		throw connection_exception();

	// Send the request.
	this->send();

}

void http_request::receive()
{
	this->response->reset();

	http_response_parser_state parser_state = VERSION;

	int buffer_size = 1024;
	char* buffer = new char[buffer_size];
	std::string status = "";
	std::string key = "";
	std::string value = "";
	
	try
	{
		do
		{
			int bytes_read = this->socket.read_some(boost::asio::buffer(buffer, buffer_size));

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
//						std::cout << "key: " << key << " value: " << value << "\n";
                                                this->response->setHeaders(key, value);
						key = "";
						parser_state = HEADER_KEY;
					}
					break;
				case BODY:
					this->response->setBodyAppend(*position++);
					if (this->response->getBody().length() == this->response->getBodySize())
						parser_state = OK;
					break;
				case OK:
					position = buffer + bytes_read;
					break;
				}
			} while (position < buffer + bytes_read);
//			std::cout << "Bytes Read: " << bytes_read << "\nFinished: " << (position < buffer + bytes_read)  << "\nState: " << parser_state << "\nBody: " << this->body.length() << " Size: " << this->body_size << "\n\n";
		
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

	if (socket.is_open())
	{
		try
		{
			socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			socket.close();
		}
		catch (boost::system::system_error&) { }
	}
}
