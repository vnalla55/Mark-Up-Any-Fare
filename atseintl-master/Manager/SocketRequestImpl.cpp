//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "Manager/SocketRequestImpl.h"

#include "ClientSocket/EOSocket.h"
#include "Common/Logger.h"
#include "Manager/TseManagerUtil.h"
#include "Util/CompressUtil.h"
#include "Xform/Xform.h"

#include <boost/atomic.hpp>

#include <zlib.h>

namespace tse
{
namespace
{
Logger
logger("atseintl.Manager.SocketRequestImpl");
Logger
loggerRequest("atseintl.Request");

const std::string
DEFAULT_RSP("REST");
const std::string
COMPRESSED_RSP("REDF");
const std::string
DEFAULT_VERSION("0001");
const std::string
DEFAULT_REVISION("0000");

boost::atomic<int> requestId(0);

int
generateRequestID()
{
  return ++requestId;
}

}

SocketRequestImpl::SocketRequestImpl(DateTime startTime,
                                     eo::Socket* socket,
                                     Service& svce,
                                     Xform& xform,
                                     uint32_t& ioBufSize,
                                     uint16_t& timeout,
                                     ZThread::CountingSemaphore* countSem,
                                     ZThread::CountingSemaphore* processedRequestCS)
  : _startTime(startTime),
    _socket(socket),
    _service(svce),
    _xform(xform),
    _ioBufSize(ioBufSize),
    _timeout(timeout),
    _countSem(countSem),
    _processedRequestCS(processedRequestCS)
{
}

SocketRequestImpl::~SocketRequestImpl() { cleanup(); }

void
SocketRequestImpl::cleanup()
{
  if (_socket != nullptr)
  {
    delete _socket;
    _socket = nullptr;
  }

  if (_countSem != nullptr)
  {
    _countSem->release();
    LOG4CXX_DEBUG(logger, "Remaining trx slots = " << _countSem->count());
    _countSem = nullptr;
  }
}

void
SocketRequestImpl::run()
{

  TseManagerUtil::TrxCounter trxCounter;

  try
  {
    if (_socket == nullptr)
    {
      cleanup();
      return;
    }

    // Get request
    std::string request;
    std::string response;
    std::string command;
    std::string version;
    std::string revision;

    if (receive(command, version, revision, request) == false)
    {
      cleanup();
      return;
    }

    // if we have debug level logging, we assign the request an ID,
    // and then output the entire request with its ID. When we are
    // sending the response, we will log it with the ID.
    int id = generateRequestID();
    LOG4CXX_DEBUG(logger, "Request: ID(" << id << ") {{{\n" << request.c_str() << "\n}}}\n");

    // Process
    if (TseManagerUtil::process(
            _startTime, trxCounter, request, response, _xform, _service, _throttled, _processedRequestCS, id) ==
        false)
    {
      cleanup();
      return;
    }
    // if Shopping Request printing the processed request ID.
    std::string reqStr(request.c_str(), 0, 8);
    if (reqStr == "<Shoppin")
    {
      LOG4CXX_INFO(loggerRequest,
                   "Processed Request Response: ID(" << id << ") {{{\n"
                                                     << "}}}\n");
    }
    // now output the response with its ID.
    LOG4CXX_DEBUG(logger, "Response: ID(" << id << ") {{{\n" << response.c_str() << "\n}}}\n");

    const std::string& responseType = shouldZip(command) ? COMPRESSED_RSP : DEFAULT_RSP;

    // Respond
    if (respond(responseType, DEFAULT_VERSION, DEFAULT_REVISION, response) == false)
    {
      cleanup();
      return;
    }

    cleanup();
    return;
  }
  catch (...)
  {
    LOG4CXX_WARN(logger, "Exception caught in SocketRequestImpl::run() ");
    cleanup();

    throw;
  }
}

bool
SocketRequestImpl::receive(std::string& command,
                           std::string& version,
                           std::string& revision,
                           std::string& request)
{
  if (!command.empty())
    command.erase();

  if (!version.empty())
    version.erase();

  if (!revision.empty())
    revision.erase();

  if (!request.empty())
    request.erase();

  char requestBuf[_ioBufSize + 1];
  uint32_t headerLen = 0;

  LOG4CXX_DEBUG(logger, "Impl: " << this << ", reading on: " << _socket->getChannel());

  TrxHeader header;
  memset(&header, 0, sizeof(TrxHeader));

  // Read the header size (IntelliSell header will be total inclusive length
  if (_socket->read(&headerLen, sizeof(headerLen), _timeout, 0L) < 0)
  {
    LOG4CXX_WARN(logger,
                 "Impl: " << this << ", Header size read error: " << _socket->getChannel());
    return false;
  }
  uint32_t headerSize = ntohl(headerLen);
  if (headerSize != sizeof(TrxHeader))
  {
    // Assume IntelliSell header
    LOG4CXX_DEBUG(logger, "Assuming ISELL header!");
    header.payloadSize = headerSize;

    // Read command
    if (_socket->read(&header.command, 8, _timeout, 0L) < 0)
    {
      LOG4CXX_FATAL(logger,
                    "Impl: " << this
                             << ", IntelliSell command read error: " << _socket->getChannel());
      return false;
    }
    _use8ByteHeader = true;
  }
  else
  {
    // Read request header
    LOG4CXX_DEBUG(logger, "Reading header: " << headerSize);
    if (_socket->read(&header, headerSize, _timeout, 0L) < 0)
    {
      LOG4CXX_FATAL(logger, "Impl: " << this << ", Header read error: " << _socket->getChannel());
      return false;
    }
    version = std::string(header.schemaVersion, VERSION_SIZE);
    revision = std::string(header.schemaVersion, VERSION_SIZE);
  }
  command.assign(header.command, header.command + sizeof(header.command));

  LOG4CXX_DEBUG(logger, "Request command: '" << command << "'\n");

  bool decode = shouldZip(command);

  // ** Assume network byte order
  size_t numNeeded = 0;
  if (_use8ByteHeader)
  {
    numNeeded = header.payloadSize - sizeof(header.payloadSize) - 8;
  }
  else
  {
    numNeeded = ntohl(header.payloadSize);
  }

  LOG4CXX_DEBUG(logger, "numNeeded: " << numNeeded);

  while (numNeeded > 0)
  {
    // Read XML
    const long retCode =
        _socket->read(&requestBuf, std::min(numNeeded, size_t(_ioBufSize)), _timeout, 0L);
    LOG4CXX_DEBUG(logger, "Read N: " << retCode);
    if (retCode < 0)
    {
      LOG4CXX_FATAL(logger, "Impl: " << this << ", read error: " << _socket->getChannel());
      return false;
    }

    request.append(requestBuf, retCode);

    numNeeded -= retCode;
  }

  if (decode)
  {
    std::vector<char> buf(request.begin(), request.end());
    const bool res = CompressUtil::decompress(buf);
    if (res == false)
    {
      return false;
    }

    request.assign(buf.begin(), buf.end());
  }

  return true;
}

bool
SocketRequestImpl::respond(const std::string& command,
                           const std::string& version,
                           const std::string& revision,
                           const std::string& responseStr)
{
  LOG4CXX_DEBUG(logger, "Doing respond...");

  // add 1 for the terminating zero byte
  uint32_t len = uint32_t(responseStr.size() + 1);

  const std::string* responsePtr = &responseStr;
  std::string compressBuf;
  if (shouldZip(command))
  {
    std::vector<char> buf(responseStr.begin(), responseStr.end());
    buf.push_back(0); // add terminator on before compressing

    const bool res = CompressUtil::compress(buf);
    if (res == false)
    {
      return false;
    }

    compressBuf.assign(buf.begin(), buf.end());
    responsePtr = &compressBuf;

    // binary data is sent without a terminator. The text
    // will have a terminator after being decompressed.
    len = uint32_t(buf.size());
  }

  const std::string& response = *responsePtr;

  uint16_t retries = 3;

  TrxHeader header;
  uint32_t responseHeaderSize = 0;

  // Support both
  if (_use8ByteHeader)
  {
    // Length first
    responseHeaderSize =
        sizeof(header.payloadSize) + sizeof(header.command) + sizeof(header.schemaVersion);
    std::fill(header.schemaVersion, header.schemaVersion + sizeof(header.schemaVersion), 0);
    header.payloadSize = htonl(responseHeaderSize + len);
  }
  else
  {
    responseHeaderSize = sizeof(header);
    header.payloadSize = htonl(len);
    char tmpBuf[10];

    sprintf(tmpBuf, "%0*d", VERSION_SIZE, atoi(version.c_str()));
    memcpy(header.schemaVersion, tmpBuf, VERSION_SIZE);
    sprintf(tmpBuf, "%0*d", REVISION_SIZE, atoi(revision.c_str()));
    memcpy(header.schemaRevision, tmpBuf, REVISION_SIZE);
  }

  // place 'command' in 'header.command', with header.command padded
  // with 0' characters
  const std::size_t commandChars = std::min(command.size(), sizeof(header.command));
  std::copy(command.begin(), command.begin() + commandChars, header.command);

  bool writeOK = true;

  if (!_use8ByteHeader)
  {
    int32_t headerLen = htonl(responseHeaderSize);
    for (uint16_t i = 0; i < retries; i++)
    {
      const long retCode = _socket->write(&headerLen, sizeof(responseHeaderSize));
      if (retCode < 0)
      {
        LOG4CXX_FATAL(logger,
                      "Impl: " << this << " write (header size: " << responseHeaderSize
                               << ") failed, code: " << retCode
                               << ", channel: " << _socket->getChannel());
      }
      else
      {
        writeOK = true;
        break;
      }
    }
    if (writeOK == false)
      return writeOK;
  }

  for (uint16_t i = 0; i < retries; i++)
  {
    const long retCode = _socket->write(&header, responseHeaderSize);
    if (retCode < 0)
    {
      LOG4CXX_FATAL(logger,
                    "Impl: " << this << " write (header: " << responseHeaderSize
                             << ") failed, code: " << retCode
                             << ", channel: " << _socket->getChannel());
    }
    else
    {
      writeOK = true;
      break;
    }
  }
  if (writeOK == false)
    return writeOK;

  writeOK = true;
  for (uint16_t i = 0; i < retries; i++)
  {
    // Next the response data
    if (_socket->write(response.c_str(), len) < 0)
    {
      LOG4CXX_FATAL(logger,
          "Impl: " << this << ", write (response data) failed, channel:" << _socket->getChannel());
      writeOK = false;
    }
    else
    {
      writeOK = true;
      break;
    }
  }

  if (writeOK == false)
    return writeOK;

  LOG4CXX_DEBUG(logger, "Response successful");
  return true;
}

bool
SocketRequestImpl::shouldZip(const std::string& command)
{
  if (LIKELY(command == "RQDF" || command == "REDF"))
    return true;
  return false;
}
}
