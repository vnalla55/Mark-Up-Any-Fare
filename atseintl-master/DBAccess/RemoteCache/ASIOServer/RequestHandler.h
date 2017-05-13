#pragma once

#include <boost/asio.hpp>

namespace tse
{

namespace RemoteCache
{

struct Reply;
struct Request;

namespace RequestHandler
{
  // handle a request and produce a reply
  void handleRequest(boost::asio::ip::tcp::socket& socket,
                     const Request& req,
                     Reply& rep);

}// RequestHandler

} // RemoteCache

} // tse
