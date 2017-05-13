// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/EntryHelp.h"
#include "TaxDisplay/Response/ResponseFormatter.h"
#include "test/include/CppUnitHelperMacros.h"

#include <sstream>

namespace tax
{
namespace display
{

class EntryHelpTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EntryHelpTest);
  CPPUNIT_TEST(testBuildBody);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}

  void testBuildBody()
  {
    TaxDisplayRequest request;
    ResponseFormatter formatter;
    EntryHelp entry(request, formatter);
    entry.buildBody();
    CPPUNIT_ASSERT(formatter.linesList().front().str() == "TAX FORMAT TX* AND TX1* HELP DISPLAY");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EntryHelpTest);
} /* namespace display */
} /* namespace tax */
