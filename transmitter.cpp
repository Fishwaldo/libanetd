#include "transmitter.h"

transmitter::transmitter(boost::asio::io_service *postback)
{
	this->postbackio = postback;
	this->response = new http_response;
	this->response->reset();
}

transmitter::~transmitter()
{
	delete this->response;
}

void transmitter::connect(std::string url)
{
	http_request request(this->response,this->io_service);
	int redirtimes = 0;

	while ((this->response->getStatus() < 200) || (this->response->getStatus() >= 300)) {
		// Send the HTTP request and receive the this->response.
		request.send(url);
		if ((this->response->getStatus() >= 300) && (this->response->getStatus() < 400)) {
			/* Redirect */
			if (redirtimes++ > 5) {
				request.disconnect();
				std::cout << boost::this_thread::get_id() << " "<< "Redirection Failure. Redirected too many times\n";
				throw;
			}
			std::string Location = this->response->getHeader("Location");
			if (Location != "") {
				std::cout << boost::this_thread::get_id() << " "<< "Redirecting (" << this->response->getStatus() << ") to " << Location << "\n";
				url = Location;
				request.reset();
				request.disconnect();
			} else {
				std::cout << boost::this_thread::get_id() << " "<< "Redirection Failure (" << this->response->getStatus() << "). No Location Specified\n";
				throw;
			}
		} else if ((this->response->getStatus() >= 400) && (this->response->getStatus() < 500)) {
			std::cout << boost::this_thread::get_id() << " "<< "Not Found (" << this->response->getStatus() << ") Error\n";
			request.disconnect();
			throw;
		} else if ((this->response->getStatus() >= 500) && (this->response->getStatus() < 600)) {
			std::cout << boost::this_thread::get_id() << " "<< "Server Error (" << this->response->getStatus() << ")\n";
			request.disconnect();
			throw;
		}
	}
	std::cout << boost::this_thread::get_id() << " "<< this->response->getStatus() << "\n";

	std::map<std::string, std::string>::iterator iter = this->response->getHeadersBegin();
	while (iter != this->response->getHeadersEnd()) {
		std::cout << boost::this_thread::get_id() << " "<< "Header: " << iter->first << " Value: " << iter->second << "\n";
		iter++;
	}

	//std::cout << boost::this_thread::get_id() << " "<< this->response->body << "\n";
	//Check for the this->response status.
	if (this->response->getStatus() != 200)
	{
		request.disconnect();
	}
	this->postbackio->post(boost::bind(&transmitter::callback, this, this->response));

}

bool transmitter::Starttransfer(std::string url) {
	boost::thread t(boost::bind(&transmitter::connect, this, url));
	std::cout << boost::this_thread::get_id() << " "<< t.get_id() << "\n";
	return true;
}



void transmitter::send(const char* data, int size)
{
#if 0
	if (!socket.is_open())
		throw server_connection_exception();

	boost::asio::write(socket, boost::asio::buffer(data, size));
#endif
}

void transmitter::callback(http_response *res) {
	sleep(1);
	std::cout << boost::this_thread::get_id() << " "<< "\n\nResponse:\n";
	std::cout << boost::this_thread::get_id() << " "<< res->getStatus() << "\n";

	std::map<std::string, std::string>::iterator iter = res->getHeadersBegin();
	while (iter != res->getHeadersEnd()) {
		std::cout << boost::this_thread::get_id() << " "<< "Header: " << iter->first << " Value: " << iter->second << "\n";
		iter++;
	}
}
