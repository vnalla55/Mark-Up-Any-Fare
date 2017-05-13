#pragma once

#include "DBAccess/RemoteCache/RemoteCacheHeader.h"

#include <string>
#include <vector>

namespace tse
{

namespace RemoteCache
{

struct Request;
typedef std::shared_ptr<Request> RequestPtr;

// a request received from a client
struct Request
{
  explicit Request(uint64_t requestId)
    : _header(requestId)
  {
    _headerVect.reserve(_headerSz);
  }
  RemoteCacheHeader _header;
  std::vector<char> _headerVect;
  std::vector<char> _payload;
  std::vector<char> _buffer;
};

} // RemoteCache

} // tse
