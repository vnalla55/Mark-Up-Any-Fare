//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "ClientSocket/ClientSocket.h"
#include "Common/Logger.h"

using namespace eo;

namespace
{
const std::string
DEFAULT_RSP("REST");
const std::string
DEFAULT_VERSION("0001");
const std::string
DEFAULT_REVISION("0000");
}

namespace tse
{
static Logger
logger("atseintl.ClientSocket.ClientSocket");

bool
ClientSocket::initialize()
{
  if (_clientSocketConfigurableValues.linger.getValue() != "FALSE")
    _linger = true;
  else
    _linger = false;
  _lingerTime = _clientSocketConfigurableValues.lingerTime.getValue();
  if (_clientSocketConfigurableValues.keepAlive.getValue() != "FALSE")
    _keepAlive = true;
  else
    _keepAlive = false;
  _ioBufSize = _clientSocketConfigurableValues.ioBufSize.getValue();
  _timeout = _clientSocketConfigurableValues.timeout.getValue();
  _port = _clientSocketConfigurableValues.port.getValue();
  _server = _clientSocketConfigurableValues.server.getValue();
  // Connect here
  return connect();
}

bool
ClientSocket::connect()
{
  if (!_socket.connect(_server, _port, 1))
  {
    const char* socketErrMsg;
    long socketErrNo = 0;
    socketErrNo = _socket.getLastError(&socketErrMsg);

    LOG4CXX_FATAL(logger,
                  "Failed to connect to server:  " << _server << ", on port: " << _port << " - "
                                                   << socketErrNo << ": " << socketErrMsg);
    return false;
  }

  LOG4CXX_INFO(logger, "Connected to server: " << _server << ", on port: " << _port);
  // Set what we have so far
  if (_linger == true)
    _socket.setLinger(true, _lingerTime);
  else
    _socket.setLinger(false);

  if (_keepAlive == true)
    _socket.setKeepAlive(true);
  else
    _socket.setKeepAlive(false);

  // Only set IO_BUF_SIZE if greater than EO current value
  if (_ioBufSize > MAX_SOCKET_BUFFER_SIZE)
    _socket.setBufferSize(eo::BOTH, _ioBufSize);

  return true;
}

bool
ClientSocket::receive(std::string& command,
                      std::string& version,
                      std::string& revision,
                      std::string& response)
{
  char responseBuf[_ioBufSize + 1];

  if (!command.empty())
    command.erase();

  if (!version.empty())
    version.empty();

  if (!revision.empty())
    revision.empty();

  if (!response.empty())
    response.erase();

  uint32_t headerLen = 0;

  LOG4CXX_DEBUG(logger, "Impl: " << this << ", reading on: " << _socket.getChannel());

  TrxHeader header;

  // Read the header size
  if (_socket.read(&headerLen, sizeof(headerLen), _timeout, 0L) < 0)
  {
    const char* socketErrMsg;
    const long socketErrNo = _socket.getLastError(&socketErrMsg);

    LOG4CXX_FATAL(logger,
                  "Impl: " << this << ", Header size read error: " << _socket.getChannel()
                           << " - socket error: " << socketErrNo << ": " << socketErrMsg);

    return false;
  }
  headerLen = ntohl(headerLen);
  if (headerLen != sizeof(TrxHeader))
  {
    LOG4CXX_FATAL(logger, "Unrecognized HEADER size");
    return false;
  }

  // Read request header
  LOG4CXX_DEBUG(logger, "Reading header: " << headerLen);
  if (_socket.read(&header, headerLen, _timeout, 0L) < 0)
  {
    const char* socketErrMsg;
    long socketErrNo = 0;
    socketErrNo = _socket.getLastError(&socketErrMsg);

    LOG4CXX_FATAL(logger,
                  "Impl: " << this << ", Header read error: " << _socket.getChannel()
                           << " - socket error: " << socketErrNo << ": " << socketErrMsg);

    return false;
  }
  version = std::string(header.schemaVersion, VERSION_SIZE);
  revision = std::string(header.schemaRevision, REVISION_SIZE);
  command = std::string(header.command, COMMAND_SIZE);

  uint32_t numNeeded = ntohl(header.payloadSize);

  LOG4CXX_DEBUG(logger, "numNeeded: " << numNeeded);

  while (numNeeded > 0)
  {
    // Read XML
    const long retCode = _socket.read(&responseBuf, std::min(numNeeded, _ioBufSize), _timeout, 0L);
    LOG4CXX_DEBUG(logger, "Read N: " << retCode);
    if (retCode < 0)
    {
      const char* socketErrMsg;
      long socketErrNo = 0;
      socketErrNo = _socket.getLastError(&socketErrMsg);

      LOG4CXX_FATAL(logger,
                    "Impl: " << this << ", read error: " << _socket.getChannel()
                             << " - socket error: " << socketErrNo << ": " << socketErrMsg);

      return false;
    }

    response.append(responseBuf, retCode);

    numNeeded -= uint32_t(retCode);
  }
  return true;
}

bool
ClientSocket::send(const std::string& command,
                   const std::string& version,
                   const std::string& revision,
                   const std::string& request)
{
  LOG4CXX_DEBUG(logger, "Doing send...");

  uint16_t retries = 3;

  TrxHeader header;
  memset((void*)&header, 0x0, sizeof(header));
  uint32_t responseHeaderSize = 0;

  // add 1 for the terminating zero byte
  uint32_t len = uint32_t(request.size());

  responseHeaderSize = uint32_t(sizeof(header));
  header.payloadSize = htonl(len);
  char tmpBuf[10];

  sprintf(tmpBuf, "%0*d", VERSION_SIZE, atoi(version.c_str()));
  memcpy(header.schemaVersion, tmpBuf, VERSION_SIZE);
  sprintf(tmpBuf, "%0*d", REVISION_SIZE, atoi(revision.c_str()));
  memcpy(header.schemaRevision, tmpBuf, REVISION_SIZE);

  // place 'command' in 'header.command', with header.command padded
  // with 0' characters
  const std::size_t commandChars = std::min(command.size(), sizeof(header.command));
  std::copy(command.begin(), command.begin() + commandChars, header.command);

  bool writeOK = true;

  int32_t headerLen = htonl(responseHeaderSize);
  for (uint16_t i = 0; i < retries; i++)
  {
    const long retCode = _socket.write(&headerLen, sizeof(responseHeaderSize));
    if (retCode == WRITE_ERROR)
    {
      const char* socketErrMsg;
      long socketErrNo = 0;
      socketErrNo = _socket.getLastError(&socketErrMsg);

      LOG4CXX_FATAL(logger,
                    "Impl: " << this << " write (header size: " << responseHeaderSize
                             << ") failed, code: " << retCode
                             << ", channel: " << _socket.getChannel()
                             << " - socket error: " << socketErrNo << ": " << socketErrMsg);

      // if connection was lost, connect to server again
      if (!_socket.reConnect(_server, _port))
      {
        LOG4CXX_FATAL(logger,
                      "Failed to re-connect to server:  " << _server << ", on port: " << _port);
        writeOK = false;
        break;
      }
    }
    else if (retCode == WRITE_TIMEDOUT)
    {
      LOG4CXX_FATAL(logger,
                    "Impl: " << this << " write (header size: " << responseHeaderSize
                             << ") timed-out, code: " << retCode
                             << ", channel: " << _socket.getChannel());
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

  for (uint16_t i = 0; i < retries; i++)
  {
    const long retCode = _socket.write(&header, responseHeaderSize);
    if (retCode == WRITE_ERROR)
    {
      LOG4CXX_FATAL(logger,
                    "Impl: " << this << " write (header: " << responseHeaderSize
                             << ") failed, code: " << retCode
                             << ", channel: " << _socket.getChannel());
    }
    else if (retCode == WRITE_TIMEDOUT)
    {
      LOG4CXX_FATAL(logger,
                    "Impl: " << this << " write (header: " << responseHeaderSize
                             << ") timed-out, code: " << retCode
                             << ", channel: " << _socket.getChannel());
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

  writeOK = true;
  for (uint16_t i = 0; i < retries; i++)
  {
    // Next the response data
    const long retCode = _socket.write(request.c_str(), len);
    if (retCode == WRITE_ERROR)
    {
      LOG4CXX_FATAL(logger,
                    "Impl: " << this
                             << ", write (response data) failed, channel:" << _socket.getChannel());
      writeOK = false;
    }
    else if (retCode == WRITE_TIMEDOUT)
    {
      LOG4CXX_FATAL(logger,
                    "Impl: " << this << ", write (response data) timed-out, channel:"
                             << _socket.getChannel());
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
} // namespace tse
