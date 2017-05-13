#pragma once

#include "DBAccess/CompressedData.h"
#include "DBAccess/RemoteCache/RemoteCacheHeader.h"

#include <boost/asio.hpp>

#include <chrono>
#include <memory>

namespace tse
{

namespace RemoteCache
{

struct Reply;
typedef std::shared_ptr<Reply> ReplyPtr;

struct Reply
{
  explicit Reply(uint64_t requestId);

  RemoteCacheHeader _header;
  std::vector<char> _headerVect;
  std::vector<char> _payload;
  sfc::CompressedDataPtr _compressed;
  std::size_t _payloadSize{};
  std::chrono::steady_clock::time_point _start;

  // convert the reply into a vector of buffers. The buffers do not own the
  // underlying memory blocks, therefore the reply object must remain valid and
  // not be changed until the write operation has completed
  size_t convertToBuffers();
  size_t toBuffers();

  std::vector<boost::asio::const_buffer> _buffers;

  void stockReply(StatusType status);
};

} // RemoteCache

} // tse
