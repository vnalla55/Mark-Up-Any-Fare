//-------------------------------------------------------------------
//
//  File:        SocketUtils.h
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

#pragma once

#include <string>

namespace eo
{
class Socket;
}

namespace ac
{

class SocketUtils
{

public:
  struct Message
  {
    std::string command;
    std::string xmlVersion;
    std::string xmlRevision;
    std::string payload;

    void clearAll();
  };

  // ** Note**
  // This method appends the new values to any values in message
  // if you are reusing a message object be sure to invoke clearAll()
  // before repeated invocations of the readMessage() method.

  static bool readMessage(eo::Socket& socket, Message& message, bool legacy12ByteFormat = false);
  static bool writeMessage(eo::Socket& socket, Message& message, bool legacy12ByteFormat = false);

protected:
private:
  // Placed here so they wont be called
  SocketUtils(const SocketUtils& rhs);
  SocketUtils& operator=(const SocketUtils& rhs);
}; // End class

} // End namespace

