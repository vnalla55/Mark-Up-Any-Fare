#include "DBAccess/RemoteCache/ASIOServer/Reply.h"

namespace tse
{

namespace RemoteCache
{

Reply::Reply(uint64_t requestId)
  : _header(requestId)
  , _start(std::chrono::steady_clock::now())
{
}

size_t Reply::convertToBuffers()
{
  _headerVect.resize(_headerSz);
  writeHeader(_header, _headerVect);
  _buffers.push_back(boost::asio::buffer(_headerVect, _headerSz));
  if (_compressed)
  {
    _payloadSize = _compressed->_deflated.size();
    _buffers.push_back(boost::asio::buffer(_compressed->_deflated, _payloadSize));
  }
  else if (!_payload.empty())
  {
    _payloadSize = _payload.size();
    _buffers.push_back(boost::asio::buffer(_payload, _payloadSize));
  }
  return _headerSz + _payloadSize;
}

size_t Reply::toBuffers()
{
  size_t responseSize(0);
  _headerVect.resize(_headerSz);
  writeHeader(_header, _headerVect);
  responseSize += _headerSz;
  _buffers.push_back(boost::asio::buffer(_headerVect));
  if (_compressed)
  {
    _buffers.push_back(boost::asio::buffer(_compressed->_deflated));
    responseSize += _compressed->_deflated.size();
  }
  else if (!_payload.empty())
  {
    _buffers.push_back(boost::asio::buffer(_payload));
    responseSize += _payload.size();
  }
  return responseSize;
}

void Reply::stockReply(StatusType status)
{
  std::string statusStr(statusToString(status));
  _header._status = status;
  _payload.insert(_payload.end(), statusStr.begin(), statusStr.end());
}

} // RemoteCache

} // tse
