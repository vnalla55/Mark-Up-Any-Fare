//----------------------------------------------------------------------------
//
//  File:        TseServer.C
//  Description: See .h file
//  Created:     Dec 11, 2003
//  Authors:     Mark Kasprowicz
//
//  Description:
//
//  Return types:
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

#include "test/include/MockTseServer.h"

#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

MockTseServer::MockTseServer(const std::string& name) : TseServer(name)
{
  initializeConfig();
  initializeGlobalConfigMan();
  _testConfigInitializer = new TestConfigInitializer(&Global::config());
  initializeDiskCache();
  initializeGlobal();
}

MockTseServer::~MockTseServer()
{
  delete _testConfigInitializer;
  MockGlobal::clear();
}

}
