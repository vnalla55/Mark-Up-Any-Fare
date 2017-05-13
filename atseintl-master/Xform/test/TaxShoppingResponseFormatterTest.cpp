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

#include "Common/ErrorResponseException.h"
#include "test/include/CppUnitHelperMacros.h"
#include "Xform/TaxShoppingResponseFormatter.h"

#include <string>

namespace tse
{

class TaxShoppingResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxShoppingResponseFormatterTest);
  CPPUNIT_TEST(testFormatErrorResponse);
  CPPUNIT_TEST_SUITE_END();

public:

  void testFormatErrorResponse()
  {
    ErrorResponseException e(ErrorResponseException::UNKNOWN_EXCEPTION, "TEST");

    std::string response;
    TaxShoppingResponseFormatter::formatResponse(e, response);

    CPPUNIT_ASSERT_EQUAL(std::string("<TRS><ERE Q1F=\"9999\" S01=\"TEST\"/></TRS>"),
                         response);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxShoppingResponseFormatterTest);

} // namespace tse
