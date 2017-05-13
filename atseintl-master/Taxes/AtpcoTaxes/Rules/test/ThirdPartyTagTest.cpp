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
#include <memory>
#include <stdexcept>

#include "test/include/CppUnitHelperMacros.h"

#include "Rules/ThirdPartyTagApplicator.h"
#include "Rules/ThirdPartyTagRule.h"
#include "DataModel/Common/Types.h"
#include "test/PaymentDetailMock.h"

namespace tax
{

class ThirdPartyTagApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ThirdPartyTagApplicatorTest);

  CPPUNIT_TEST(testConstructor);

  CPPUNIT_TEST(testPaidByTagGovernment);
  CPPUNIT_TEST(testPaidByTagMiles);
  CPPUNIT_TEST(testPaidByTagBlank);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paidBy3rdPartyTag.reset(new type::PaidBy3rdPartyTag(type::PaidBy3rdPartyTag::Blank));
    _formOfPayment.reset(new type::FormOfPayment(type::FormOfPayment::Blank));
  }

  void tearDown()
  {
    _paidBy3rdPartyTag.reset();
    _formOfPayment.reset();
  }

  void testConstructor()
  {
    CPPUNIT_ASSERT_NO_THROW(ThirdPartyTagApplicator(*_rule, *_paidBy3rdPartyTag, *_formOfPayment));
  }

  void testPaidByTagGovernment()
  {
    *_paidBy3rdPartyTag = type::PaidBy3rdPartyTag::Government;

    ThirdPartyTagApplicator applicator(*_rule, *_paidBy3rdPartyTag, *_formOfPayment);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(_paymentDetailMock));

    *_formOfPayment = type::FormOfPayment::Miles;
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(_paymentDetailMock));

    *_formOfPayment = type::FormOfPayment::Government;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(_paymentDetailMock));
  }

  void testPaidByTagMiles()
  {
    *_paidBy3rdPartyTag = type::PaidBy3rdPartyTag::Miles;

    ThirdPartyTagApplicator applicator(*_rule, *_paidBy3rdPartyTag, *_formOfPayment);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(_paymentDetailMock));

    *_formOfPayment = type::FormOfPayment::Government;
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(_paymentDetailMock));

    *_formOfPayment = type::FormOfPayment::Miles;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(_paymentDetailMock));
  }

  void testPaidByTagBlank()
  {
    *_paidBy3rdPartyTag = type::PaidBy3rdPartyTag::Blank;

    ThirdPartyTagApplicator applicator(*_rule, *_paidBy3rdPartyTag, *_formOfPayment);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(_paymentDetailMock));

    *_formOfPayment = type::FormOfPayment::Government;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(_paymentDetailMock));

    *_formOfPayment = type::FormOfPayment::Miles;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(_paymentDetailMock));
  }

private:
  ThirdPartyTagRule* _rule;
  PaymentDetailMock _paymentDetailMock;

  std::unique_ptr<type::PaidBy3rdPartyTag> _paidBy3rdPartyTag;
  std::unique_ptr<type::FormOfPayment> _formOfPayment;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ThirdPartyTagApplicatorTest);
} // namespace tax
