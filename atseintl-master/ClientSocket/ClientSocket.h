//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "ClientSocket/EOSocket.h"
#include "Common/Config/ConfigurableValue.h"
#include <string>

namespace tse
{

const int16_t COMMAND_SIZE = 4;
const int16_t VERSION_SIZE = 4;
const int16_t REVISION_SIZE = 4;
const bool LINGER_DEFAULT = false;
const bool KEEPALIVE_DEFAULT = false;
const int32_t IO_BUF_SIZE_DEFAULT = 40960;
const int32_t IO_BUF_SIZE_MAX = 65536;
const int32_t TIMEOUT_DEFAULT = 500;
const int32_t LINGER_TIME_DEFAULT = 1000;

struct ClientSocketConfigurableValues
{
  std::string serviceName;
  ConfigurableValue<std::string> linger;
  ConfigurableValue<uint32_t> lingerTime;
  ConfigurableValue<std::string> keepAlive;
  ConfigurableValue<uint32_t> ioBufSize;
  ConfigurableValue<uint16_t> timeout;
  ConfigurableValue<uint16_t> port;
  ConfigurableValue<std::string> server;

  ClientSocketConfigurableValues(const std::string& tagName)
    : serviceName(tagName),
      linger(serviceName, "LINGER"),
      lingerTime(serviceName, "LINGER_TIME"),
      keepAlive(serviceName, "KEEP_ALIVE"),
      ioBufSize(serviceName, "IO_BUF_SIZE"),
      timeout(serviceName, "TIMEOUT"),
      port(serviceName, "PORT"),
      server(serviceName, "SERVER")
  {
  }
};

class ClientSocket final
{
public:
  ClientSocket(ClientSocketConfigurableValues& clientSocketConfigurableValues)
    : _clientSocketConfigurableValues(clientSocketConfigurableValues)
  {
  }

  bool initialize();

  bool connect();

  //-------------------------------------------------------------------------
  // @function ClientSocket::receive()
  //
  // Description: read socket data
  //
  // @param command - where to put the command
  // @param version - where to put the version
  // @param revision - where to put the revision
  // @param response - where to put the request
  // @return - true if successful, false otherwise
  //-------------------------------------------------------------------------
  bool
  receive(std::string& command, std::string& version, std::string& revision, std::string& response);

  //-------------------------------------------------------------------------
  // @function SocketRequestHandler::send()
  //
  // Description: format and send a request
  //
  // @param command - the command to send
  // @param version - the version to send
  // @param revision - the revision to send
  // @param request - where to put the response
  // @return - true if successful, false otherwise
  //-------------------------------------------------------------------------
  bool send(const std::string& command,
            const std::string& version,
            const std::string& revision,
            const std::string& request);

private:
  // Generic trx header
  struct TrxHeader
  {
    uint32_t payloadSize;
    char command[COMMAND_SIZE];
    char schemaVersion[VERSION_SIZE];
    char schemaRevision[REVISION_SIZE];
  };

  eo::Socket _socket;
  uint32_t _ioBufSize = 0; // Our read buffer size
  uint16_t _timeout = 0; // Read/Write timeout value in seconds
  std::string _server; // Server to connect to
  uint16_t _port = 0; // Server port taking our requests
  bool _keepAlive = false; // keep alive on/off
  bool _linger = false; // linger on/off
  int _lingerTime = LINGER_TIME_DEFAULT; // linger time

  ClientSocketConfigurableValues& _clientSocketConfigurableValues;
};
} // namespace tse
