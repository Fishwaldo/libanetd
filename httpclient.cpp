// Boost
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include "http.h"

using namespace DynamX::HttpClient;
using namespace DynamX::HttpClient::Logging;

class printer
{

public:
  printer (boost::asio::io_service & io) :
    timer1_ (io, boost::posix_time::seconds (1)),
    count_ (0)
  {

    timer1_.async_wait (boost::bind (&printer::print1, this));


  }


   ~printer ()
  {

    LogTrace() << "Final count is " << count_;

  }


  void print1 ()
  {

    if (count_ < 20)

      {

	LogTrace() << "Timer 1: " << count_ ;

	++count_;


	timer1_.expires_at (timer1_.expires_at () +
			    boost::posix_time::seconds (1));

	timer1_.
	  async_wait (boost::bind (&printer::print1, this));

      }

  }


private:

  boost::asio::deadline_timer timer1_;


  int count_;

};

void HTTPCallback(http_response *response) {
	LogTrace() << response->getStatus();
}



// Application entry point.
int
main (int argc, char *argv[])
{

  // Initialize default settings.
  std::string server = "https://www.google.com/search";
  server = "http://10.1.1.1/Services.asp";
  boost::asio::io_service io;
  printer p(io);


  // Ask for the server URL if one wasn't read from command line arguments.
  if (server.empty ())

    {

      std::cout << boost::this_thread::get_id() << " " << "Please, specify the server URL: ";

      std::cin >> server;

    }


  boost::shared_ptr < http_request > transmitter_block;
  http_response response;

  // Initialize the transmitter block.
  transmitter_block =
    boost::shared_ptr < http_request > (new http_request (&response, &io));
  transmitter_block->setHTTPAuth("Fishwaldo", "z1pp0baby69");
  transmitter_block->setProxyAuth("Fish", "baby69");
  transmitter_block->setCallback(boost::bind(HTTPCallback, _1));
  transmitter_block->Starttransfer (server);

  io.run();
  transmitter_block->Status.wait();
  std::cout << transmitter_block->Status.get()->getStatus() << std::endl;


  return 0;

}
