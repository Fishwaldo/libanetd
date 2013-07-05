/*
 * Example Client for libanetd
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


// Boost
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include "anetd/anetd.hpp"

using namespace DynamX::anetd;
using namespace DynamX::anetd::Logging;

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
  //std::string server = "https://www.google.com/search";
  //server = "http://10.1.1.1/Services.asp";
  std::string server = "http://weather.yahooapis.com/forecastrss?w=2502265&u=c";
  
  boost::asio::io_service io;
  printer p(io);


  // Ask for the server URL if one wasn't read from command line arguments.
  if (server.empty ())

    {

      std::cout << boost::this_thread::get_id() << " " << "Please, specify the server URL: ";

      std::cin >> server;

    }


  boost::shared_ptr < http_engine > transmitter_block;
  http_response_file response;
  response.setHTTPAuth("User", "password");
  response.setURL(server);
  // Initialize the transmitter block.
  transmitter_block =
    boost::shared_ptr < http_engine > (new http_engine (&io));
  transmitter_block->setProxyAuth("User", "password");
  transmitter_block->setCallback(boost::bind(HTTPCallback, _1));
  transmitter_block->Starttransfer (&response);

  io.run();
  transmitter_block->Status.wait();
  std::cout << transmitter_block->Status.get()->getStatus() << std::endl;
  std::cout << transmitter_block->Status.get()->getDescription() << std::endl;


  return 0;

}
