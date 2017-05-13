#include <iostream>
#include <fstream>
#include "Internal/test/InternalServiceTest.h"
#include "Common/TseConsts.h"
#include "Xform/XformClientXML.h"
#include "Common/ItinUtil.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/PricingTrx.h"

using namespace tse;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(InternalServiceTest);

//---------------------------------------------------------------------
// testInternalService()
//---------------------------------------------------------------------
void
InternalServiceTest::testInternalService()
{
  setUp();

  std::string serverName = "TestServer";
  DataHandle dataHandle;

  CPPUNIT_ASSERT(initializeConfig() == true);

  _configMan->setValue("DEFAULT_CONFIG", "./xmlConfig.cfg", "DATAHANDLER_CONFIGS");
  _configMan->setValue("DETAIL_CONFIG", "./xmlConfig.cfg", "DATAHANDLER_CONFIGS");

  XformClientXML* testServer = new XformClientXML(serverName, *_configMan);
  CPPUNIT_ASSERT(testServer != NULL);
  CPPUNIT_ASSERT(testServer->initialize(0, (char**)0) == true);

  // Read file contents
  ifstream inFile("request02BD09.xml");
  inFile.seekg(0, ios::end);
  int length = inFile.tellg();
  inFile.seekg(0, ios::beg);
  char* buffer = new char[length + 1];
  inFile.read(buffer, length);
  buffer[length] = '\0';
  inFile.close();

  // What to work with
  std::string aRequest(buffer);
  CPPUNIT_ASSERT(!aRequest.empty());

  // Need a  transaction
  Trx* trx;
  CPPUNIT_ASSERT(testServer->convert(dataHandle, aRequest, trx) == true);

  PricingTrx& pricingTrx = dynamic_cast<PricingTrx&>(*trx);

  // Do some InternalService stuff
  InternalServiceController* internalServiceController = new InternalServiceController();
  CPPUNIT_ASSERT(internalServiceController != NULL);
  CPPUNIT_ASSERT(internalServiceController->process(pricingTrx));

  delete internalServiceController;
  delete testServer;
}

//----------------------------------------------------------------------------
// initializeConfig()
//----------------------------------------------------------------------------
bool
InternalServiceTest::initializeConfig()
{
  //  default
  //
  _cfgFileName = "xmlConfig.cfg";

  // Load configuration
  //
  if (!_aConfig.read(_cfgFileName))
  {
    return false; // failure
  }

  return true; // success
}
