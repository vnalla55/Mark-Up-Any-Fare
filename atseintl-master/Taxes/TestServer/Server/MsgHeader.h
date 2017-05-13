// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#pragma once

#include <string>
#include <boost/asio.hpp>

namespace tax
{

struct MsgHeader
{
  MsgHeader(uint32_t xmlSize, std::string command = "REST", uint32_t schemaVersion = 1,
            uint32_t schemaRevision = 0)
  {
    _headerSize = htonl(16);
    _payloadSize = htonl(xmlSize);
    std::copy(command.begin(), command.end(), _command);
    _schemaVersion = htonl(schemaVersion);
    _schemaRevision = htonl(schemaRevision);
  }

private:
  uint32_t _headerSize;
  uint32_t _payloadSize;
  char _command[4];
  uint32_t _schemaVersion;
  uint32_t _schemaRevision;
};
}
