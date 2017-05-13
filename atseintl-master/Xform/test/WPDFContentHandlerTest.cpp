//----------------------------------------------------------------------------
//
//  File:  WPDFContentHandlerTest.cpp
//  Description: Unit test for WPDF
//  Created:  March 30, 2005
//  Authors:  Gregory Graham
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
//----------------------------------------------------------------------------

#include "Xform/test/WPDFContentHandlerTest.h"

#include "Common/Config/ConfigMan.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/Trx.h"
#include "DataModel/PricingDetailTrx.h"
#include <log4cxx/propertyconfigurator.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

using namespace std;
using namespace tse;

CPPUNIT_TEST_SUITE_REGISTRATION(WPDFContentHandlerTest);

log4cxx::LoggerPtr
WPDFContentHandlerTest::_logger(
    log4cxx::Logger::getLogger("atseintl.Xform.test.WPDFContentHandlerTest"));

void
WPDFContentHandlerTest::setUp()
{
  CppUnit::TestFixture::setUp();
}

void
WPDFContentHandlerTest::tearDown()
{
  CppUnit::TestFixture::tearDown();
}

void
WPDFContentHandlerTest::testParse()
{
  // initialize logger
  log4cxx::PropertyConfigurator::configure("log4cxx.properties");

  // create objects to test
  ConfigMan config;
  config.read("../detailXmlConfig.cfg");

  DataHandle dataHandle;

  // test data files
  char* dataFiles[] = { "wpdf_dtl_test1.xml", "wpdf_dtl_test2.xml", "wpdf_dtl_test3.xml",
                        "wpdf_dtl_test4.xml", "" };

  // output files
  char* outFiles[] = { "wpdf_out_test1.txt", "wpdf_out_test2.txt", "wpdf_out_test3.txt",
                       "wpdf_out_test4.txt", "" };

  int i = 0;
  while (strcmp(dataFiles[i], "") != 0)
  {
    PricingDetailTrx* trx = NULL;
    WPDFModelMap modelMap(config, dataHandle, static_cast<Trx*>(trx));
    CPPUNIT_ASSERT(modelMap.initialize());
    CPPUNIT_ASSERT(modelMap.createMap());
    WPDFContentHandler handler(config, *(static_cast<DataModelMap*>(&modelMap)));

    // load test data
    ifstream inFile(dataFiles[i]);
    inFile.seekg(0, ios::end);
    int length = inFile.tellg();
    inFile.seekg(0, ios::beg);
    char* buffer(new char[length + 1]);
    inFile.read(buffer, length);
    buffer[length] = '\0';
    inFile.close();

    // load output comparison data
    ifstream inFile2(outFiles[i]);
    inFile2.seekg(0, ios::end);
    length = inFile2.tellg();
    inFile2.seekg(0, ios::beg);
    char* outBuffer(new char[length + 1]);
    inFile2.read(outBuffer, length);
    outBuffer[length] = '\0';
    inFile2.close();

    // parse the message
    CPPUNIT_ASSERT(handler.parse(const_cast<const char*>(buffer)));
    delete[] buffer;

    // format the response
    WPDFResponseFormatter responseFormatter;
    responseFormatter.formatResponse(*trx);
    stringbuf* strbuf = trx->response().rdbuf();

    if (strbuf->str() != outBuffer)
    {
      LOG4CXX_ERROR(_logger,
                    "\n*******OUTPUT TEST DATA " << i << ":\n" << outBuffer << "*******\n");
      LOG4CXX_ERROR(_logger,
                    "\n*******WPDF RESPONSE " << i << ":\n" << strbuf->str() << "*******\n");
    }

    CPPUNIT_ASSERT(strbuf->str() == outBuffer);
    ++i;
  }
}
