#ifndef TSESERVERSTUB_H
#define TSESERVERSTUB_H

#include "Server/TseServer.h"

namespace tse
{

struct TseServerStub : public TseServer
{
  TseServerStub() : TseServer("TseServerStub") {}
};

} // tse

#endif // TSESERVERSTUB_H
