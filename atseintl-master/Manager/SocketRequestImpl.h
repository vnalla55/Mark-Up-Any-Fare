//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/DateTime.h"
#include "Common/Thread/TseCallableTask.h"
#include "Common/TseStringTypes.h"
#include "Manager/TseManagerUtil.h"

#include <ZThreads/zthread/CountingSemaphore.h>

namespace eo
{
class Socket;
}

namespace tse
{
class Service;
class Xform;

const static int32_t COMMAND_SIZE = 4;
const static int32_t VERSION_SIZE = 4;
const static int32_t REVISION_SIZE = 4;

class SocketRequestImpl : public TseCallableTask
{
public:
  SocketRequestImpl(DateTime startTime,
                    eo::Socket* socket,
                    Service& svce,
                    Xform& xform,
                    uint32_t& ioBufSize,
                    uint16_t& timeout,
                    ZThread::CountingSemaphore* countSem = nullptr,
                    ZThread::CountingSemaphore* processedRequestsCS = nullptr);
  virtual ~SocketRequestImpl();

  bool initialize() { return true; }

  void run() override;

private:
  // Intellisell is currently only using the first four bytes.  The reader
  // and writer has the smarts to distinguish between four byte headers and
  // 8 byte headers.
  struct TrxHeader
  {
    uint32_t payloadSize; // in 8 byte header will be total size
    char command[COMMAND_SIZE];
    char schemaVersion[VERSION_SIZE];
    char schemaRevision[REVISION_SIZE];
  };

  DateTime _startTime;
  eo::Socket* _socket;
  Service& _service;
  Xform& _xform;
  uint32_t& _ioBufSize; // Our read buffer size
  uint16_t& _timeout; // Read/Write timeout value in seconds
  bool _use8ByteHeader = false;
  ZThread::CountingSemaphore* _countSem;
  ZThread::CountingSemaphore* _processedRequestCS;

  bool shouldZip(const std::string& command);
  void cleanup();

  //-------------------------------------------------------------------------
  // @function SocketRequestImpl::receive()
  //
  // Description: read socket data
  //
  // @param command - where to put the command
  // @param version - where to put the schema version
  // @param revision - where to put the schema revision
  // @param request - where to put the request
  // @return - true if successful, false otherwise
  //-------------------------------------------------------------------------
  bool
  receive(std::string& command, std::string& version, std::string& revision, std::string& request);

  //-------------------------------------------------------------------------
  // @function SocketRequestHandler::respond()
  //
  // Description: send response
  //
  // @param command - command to send
  // @param version - schema version to send
  // @param revision - schema revision to send
  // @param response - response to send
  // @return - true if successful, false otherwise
  //-------------------------------------------------------------------------
  bool respond(const std::string& command,
               const std::string& version,
               const std::string& revision,
               const std::string& response);
};
} // namespace tse
