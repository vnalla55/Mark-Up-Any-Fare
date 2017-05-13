//-------------------------------------------------------------------
//
//  File:        SocketUtils.cpp
//  Created:     May 05, 2005
//  Authors:     Mark Kasprowicz
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "AppConsole/SocketUtils.h"

#include "ClientSocket/EOSocket.h"
#include "Common/FallbackUtil.h"
#include "Util/BranchPrediction.h"

#include <cstring>

#include <stdint.h>

namespace tse
{
  FIXEDFALLBACK_DECL(fallbackRefactoring00);
}

namespace
{
const uint32_t BASE_HEADER_SIZE = 16;

typedef struct ip_format
{
  uint32_t headerSize;
  uint32_t payloadSize;
  char command[4];
  char xmlVersion[4];
  char xmlRevision[4];
} IP_FORMAT;

typedef struct ip_format_12
{
  uint32_t totalMsgSize;
  char command[4];
  char xmlVersion[2];
  char xmlRevision[2];
} IP_FORMAT_12;
}

using namespace std;
using namespace ac;

void
SocketUtils::Message::clearAll()
{
  if (!command.empty())
    command.clear();
  if (!xmlVersion.empty())
    xmlVersion.clear();
  if (!xmlRevision.empty())
    xmlRevision.clear();
  if (!payload.empty())
    payload.clear();
}

//-------------------------------------------------------------------
// readMessage()
//-------------------------------------------------------------------
bool
SocketUtils::readMessage(eo::Socket& socket, Message& message, bool legacy12ByteFormat)
{

  // For both formats the first thing to do is get a leading length
  uint32_t msgLen = 0;

  if (socket.read(&msgLen, sizeof(uint32_t)) < 0)
    return false;

  msgLen = ntohl(msgLen);

  IP_FORMAT ipHeader;
  IP_FORMAT_12 ip12Header;

  void* header;
  long toRead;

  if (UNLIKELY(legacy12ByteFormat))
  {
    ip12Header.totalMsgSize = msgLen;
    header = &ip12Header.command; // lint !e545

    toRead = sizeof(IP_FORMAT_12) - sizeof(ip12Header.totalMsgSize);
  }
  else
  {

    if (UNLIKELY(msgLen != BASE_HEADER_SIZE))
    {
      // We dont know what this is!
      return false;
    }

    ipHeader.headerSize = msgLen;
    header = &ipHeader.payloadSize;

    toRead = msgLen;
  }

  if (UNLIKELY(socket.read(header, toRead) < 0))
    return false;

  if (UNLIKELY(legacy12ByteFormat))
  {
    message.command.append(ip12Header.command, 4);
    message.xmlVersion.append(ip12Header.xmlVersion, 2);

    toRead = ip12Header.totalMsgSize - sizeof(IP_FORMAT_12);
  }
  else
  {
    // Fixup the byte ordering
    ipHeader.payloadSize = ntohl(ipHeader.payloadSize); // lint !e644

    message.command.append(ipHeader.command, 4);
    message.xmlVersion.append(ipHeader.xmlVersion, 4);
    message.xmlRevision.append(ipHeader.xmlRevision, 4);

    if (tse::fallback::fixed::fallbackRefactoring00())
    {
      // This is dead code since ipHeader.headerSize is always 16 when this point is reached
      if (ipHeader.headerSize > BASE_HEADER_SIZE)
      {
        // These is some application specific stuff we're skipping for now
        toRead = ipHeader.headerSize - BASE_HEADER_SIZE;
        char buff[toRead];

        if (socket.read(buff, toRead) < 0)
          return false;
      }
    }

    toRead = ipHeader.payloadSize;
  }

  // Now get the payload
  if (toRead > 0)
  {
    char buff[toRead];

    if (socket.read(buff, toRead) < 0)
      return false;

    message.payload.append(buff, toRead);
  }

  return true;
}

//-------------------------------------------------------------------
// writeMessage()
//-------------------------------------------------------------------
bool
SocketUtils::writeMessage(eo::Socket& socket, Message& message, bool legacy12ByteFormat)
{
  IP_FORMAT ipHeader;
  IP_FORMAT_12 ip12Header;

  void* header;
  long toWrite = 0;
  size_t payloadSize = message.payload.length() + 1;

  if (UNLIKELY(legacy12ByteFormat))
  {
    header = &ip12Header;
    toWrite = sizeof(IP_FORMAT_12);

    ip12Header.totalMsgSize = htonl(toWrite + payloadSize); // They want a null at the end

    strncpy(ip12Header.command, message.command.c_str(), sizeof(ip12Header.command));
    strncpy(ip12Header.xmlVersion, message.xmlVersion.c_str(), sizeof(ip12Header.xmlVersion));
    memset(ip12Header.xmlRevision, 0, sizeof(ip12Header.xmlRevision));
  }
  else
  {
    header = &ipHeader;
    toWrite = sizeof(IP_FORMAT);

    ipHeader.headerSize = htonl(BASE_HEADER_SIZE);
    ipHeader.payloadSize = htonl(payloadSize); // They want a null at the end

    strncpy(ipHeader.command, message.command.c_str(), sizeof(ipHeader.command));
    strncpy(ipHeader.xmlVersion, message.xmlVersion.c_str(), sizeof(ipHeader.xmlVersion));
    strncpy(ipHeader.xmlRevision, message.xmlRevision.c_str(), sizeof(ipHeader.xmlRevision));
  }

  if (UNLIKELY(socket.write(header, toWrite, 0L, 0L) < 0))
    return false;

  toWrite = payloadSize;

  if (UNLIKELY(socket.write(message.payload.c_str(), toWrite, 0L, 0L) < 0))
    return false;

  return true;
}
