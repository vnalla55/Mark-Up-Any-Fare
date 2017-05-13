//----------------------------------------------------------------------------
//
//  File:  WPDFContentHandlerTest.h
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

#ifndef WPDF_CONTENT_HANDLER_TEST_H
#define WPDF_CONTENT_HANDLER_TEST_H

#include "Common/Logger.h"

#include <cppunit/extensions/HelperMacros.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

class WPDFContentHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(WPDFContentHandlerTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testParse();

private:
  static log4cxx::LoggerPtr _logger;
};

#endif // WPDF_CONTENT_HANDLER_TEST_H
