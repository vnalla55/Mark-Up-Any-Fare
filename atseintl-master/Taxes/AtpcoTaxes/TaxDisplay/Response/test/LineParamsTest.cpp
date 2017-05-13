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

#include "TaxDisplay/Response/LineParams.h"
#include "test/include/CppUnitHelperMacros.h"

#include <sstream>

namespace tax
{
namespace display
{

class LineParamsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(LineParamsTest);
  CPPUNIT_TEST(testOperatorEqual);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}

  void testOperatorEqual()
  {
    LineParams p1, p2;
    p1.setLeftMargin(5);
    p1.isUserDefined() = true;
    p2.setLeftMargin(5);
    p2.isUserDefined() = true;
    CPPUNIT_ASSERT(p1 == p2);

    LineParams p3(p1);
    p3.setLeftMargin(0);
    CPPUNIT_ASSERT(p1 != p3);

    LineParams p4(p1);
    p4.isUserDefined() = false;
    CPPUNIT_ASSERT(p1 != p4);

    LineParams p5(p1);
    p5.longLineFormatting() = LineParams::LongLineFormatting::TRUNCATE;
    CPPUNIT_ASSERT(p1 != p5);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(LineParamsTest);

} /* namespace display */
} /* namespace tax */
