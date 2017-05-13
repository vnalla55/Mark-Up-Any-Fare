// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "test/include/CppUnitHelperMacros.h"
//#include "test/include/TestMemHandle.h"
#include "test/PaymentDetailMock.h"

#include "Rules/SpecialTaxProcessingApplicator.h"
#include "Rules/SpecialTaxProcessingRule.h"

namespace tax
{

class SpecialTaxProcessingApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SpecialTaxProcessingApplicatorTest);
  CPPUNIT_TEST(testApply_settlementMethod_TCH);
  CPPUNIT_TEST(testApply_settlementMethod_not_TCH);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testApply_settlementMethod_TCH()
  {
    SpecialTaxProcessingRule rule("03");
    SpecialTaxProcessingApplicator applicator(rule, true);
    CPPUNIT_ASSERT(applicator.apply(_paymentDetailMock));
  }

  void testApply_settlementMethod_not_TCH()
  {
    SpecialTaxProcessingRule rule("03");
    SpecialTaxProcessingApplicator applicator(rule, false);
    CPPUNIT_ASSERT(!applicator.apply(_paymentDetailMock));
  }

private:
  PaymentDetailMock _paymentDetailMock;

};

CPPUNIT_TEST_SUITE_REGISTRATION(SpecialTaxProcessingApplicatorTest);
} // namespace tax
