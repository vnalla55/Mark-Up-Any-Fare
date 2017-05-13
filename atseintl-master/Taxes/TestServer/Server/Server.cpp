// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "TestServer/Xform/BuildInfo.h"

#include "TestServer/Server/Session.h"
#include "TestServer/Server/Settings.h"
#include "TestServer/Server/TestServerRequestRecorder.h"

using namespace tax;
using namespace boost;
using namespace boost::asio;
using ip::tcp;

class Server
{
public:
  Server(io_service& ioService, uint16_t port)
    : _ioService(ioService), _acceptor(_ioService, tcp::endpoint(tcp::v6(), port))
  {
    _newSession = new Session(_ioService);
    _acceptor.async_accept(_newSession->getSocket(),
                           bind(&Server::handle_accept, this, _newSession, boost::asio::placeholders::error));
  }

  ~Server()
  {
    delete _newSession;
    for(Session * session : _sessions)
    {
      delete session;
    }
  }

  void handle_accept(Session* new_session, const boost::system::error_code& error)
  {
    if (!error)
    {
      new_session->start();
      new_session = new Session(_ioService);
      _sessions.push_back(new_session);
      _acceptor.async_accept(new_session->getSocket(),
                             bind(&Server::handle_accept, this, new_session, boost::asio::placeholders::error));
    }
    else
    {
      delete new_session;
      new_session = 0;
    }
  }

private:
  io_service& _ioService;
  tcp::acceptor _acceptor;
  Session* _newSession;
  std::vector<Session*> _sessions;
};

int main(int argc, char* argv[])
{
  std::cout << std::endl << "Server built at " << BuildInfo::date() << " on " << BuildInfo::host()
            << " by " << BuildInfo::user() << " using commit " << BuildInfo::commit() << std::endl;

  TestServerRequestRecorder::Init()->active() = true;
  std::vector<std::string> args(argv + 1, argv + argc);
  try
  {
    uint16_t port = DEFAULT_PORT;
    for (std::vector<std::string>::iterator i = args.begin(); i != args.end(); ++i)
    {
      if (*i == "-h" || *i == "--help")
      {
        std::cout << "Parameters help information" << std::endl;
        std::cout << "Option: --build  Rebuild server from source" << std::endl;
        std::cout << "Option: --clean  Clear binary files" << std::endl;
        std::cout << "Option: -r       Record request to Taxes/TestServer/Server/"
                     "test/ComponentTests.xml" << std::endl;
        std::cout << "Option: -p PORT  Start server on custom PORT" << std::endl;
        std::cout << "Option: -v	 Server display requests and responses" << std::endl;
        std::cout << "Option: -h or --help	 Display this help" << std::endl;
        return -1;
      }
      else if (*i == "-r")
      {
        std::cout << "Server will record component tests!" << std::endl;
        TestServerRequestRecorder::Init()->setFile("TestServer/Server/test/ComponentTests.xml");
      }
      else if (*i == "-p")
      {
        port = uint16_t(std::atoi((*++i).c_str()));
      }
      else if (*i == "-v")
      {
        TestServerRequestRecorder::Init()->verbose() = true;
        std::cout << "Verbose mode enabled" << std::endl;
      }
      else
      {
        std::cout << "Warning " << *i << " is unrecognized parameter! It will be ignored"
                  << std::endl;
      }
    }

    if (port == 0)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    std::cout << "Server listen on port: " << port << std::endl;

    io_service io_service;
    Server s(io_service, port);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
