#ifndef FAKE_SERVER_H
#define FAKE_SERVER_H

#include "Server/TseServer.h"

namespace tse
{

class FakeServer : public TseServer
{
public:
  using TseServer::initializeGlobalConfigMan;
  using TseServer::initializeGlobal;
};

} // end of tse

#endif
