// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Rules/ExemptTagApplicator.h"
#include "Rules/ExemptTagRule.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "ServiceInterfaces/Services.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "test/PaymentDetailMock.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tax
{

class ExemptTagApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ExemptTagApplicatorTest);

  CPPUNIT_TEST(testExempt);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _services.reset(new DefaultServices());
    _services->setFallbackService(new FallbackServiceServer());
  }

  void tearDown()
  {
  }

  void testExempt()
  {
    ExemptTagApplicator applicator((ExemptTagRule()), *_services);

    PaymentDetailMock paymentDetailMock;
    paymentDetailMock.taxEquivalentAmount() = 100;

    CPPUNIT_ASSERT(applicator.apply(paymentDetailMock));
    CPPUNIT_ASSERT(paymentDetailMock.taxEquivalentAmount() == 0);
    CPPUNIT_ASSERT(paymentDetailMock.taxEquivalentWithMarkupAmount() == 0);
  }

private:
  std::unique_ptr<tax::DefaultServices> _services;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExemptTagApplicatorTest);

} // namespace tse
