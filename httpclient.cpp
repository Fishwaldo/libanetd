// Boost
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include "transmitter.h"


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

    std::cout << boost::this_thread::get_id() << " " << "Final count is " << count_ << "\n";

  }


  void print1 ()
  {

    if (count_ < 2)

      {

	std::cout << boost::this_thread::get_id() << " " << "Timer 1: " << count_ << "\n";

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



// Application entry point.
int
main (int argc, char *argv[])
{

  // Initialize default settings.
  std::string server = "https://www.google.com/search";

  boost::asio::io_service io;
  printer p(io);


  // Ask for the server URL if one wasn't read from command line arguments.
  if (server.empty ())

    {

      std::cout << boost::this_thread::get_id() << " " << "Please, specify the server URL: ";

      std::cin >> server;

    }


  boost::shared_ptr < transmitter > transmitter_block;


  // Initialize the transmitter block.
  transmitter_block =
    boost::shared_ptr < transmitter > (new transmitter (&io));


  // Repeat asking for the username and the server URL until connection is successfully established.
  bool succeed = false;

  while (!succeed)

    {

      try
      {

	// Try to connect to the server.
	transmitter_block->Starttransfer (server);

	// Connection succeeded.
	succeed = true;

      }

      catch (transmitter::server_connection_exception &)
      {

	// Failed to connect to the server. Ask for another server URL.
	std::cout << boost::this_thread::get_id() << " " << "Please, specify another server URL: ";

	std::cin >> server;

      }

    }
    io.run();



  return 0;

}
