//----------------------------------------------------------------------------
//
//  Description: Mock Tse Server class for the cppUnit test.
//
//
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#ifndef MOCK_TSE_SERVER_H
#define MOCK_TSE_SERVER_H

#include <string>
#include "Server/TseServer.h"
#include "Server/LoadableModule.h"

class TestConfigInitializer;

namespace tse
{

class MockTseServer : public TseServer
{
public:
  MockTseServer(const std::string& name = "TseServer");
  ~MockTseServer();

private:
  TestConfigInitializer* _testConfigInitializer = nullptr;
};
}
#endif // End #ifndef TSE_SERVER_H
