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

#include "test/include/CppUnitHelperMacros.h"

#include "Rules/CustomerRestrictionApplicator.h"
#include "Rules/CustomerRestrictionRule.h"
#include "Rules/PaymentRuleData.h"
#include "ServiceInterfaces/CustomerService.h"

namespace tax
{

class CustomerServiceMock : public CustomerService
{
public:
  CustomerPtr getCustomer(const type::PseudoCityCode& pcc) const override
  {
    if (pcc == "MSK1")
      return std::make_shared<CustomerPtr::element_type>(
        true, false, true, pcc);
    else if (pcc == "MSK2")
      return std::make_shared<CustomerPtr::element_type>(
        false, true, false, pcc);

    return std::make_shared<CustomerPtr::element_type>();
  }
};

class CustomerRestrictionApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CustomerRestrictionApplicatorTest);
    CPPUNIT_TEST(testConstructor);

    CPPUNIT_TEST(testAny);

    CPPUNIT_TEST(testJJ);
    CPPUNIT_TEST(testG3);
    CPPUNIT_TEST(testT4);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _serviceMock.reset(new CustomerServiceMock());

    _paymentDetail.reset(
       new PaymentDetail(
             PaymentRuleData(
               type::SeqNo(),
               type::TicketedPointTag::MatchTicketedPointsOnly,
               TaxableUnitTagSet::none(),
               0,
               type::CurrencyCode(UninitializedCode),
               type::TaxAppliesToTagInd::Blank),
             Geo(),
             Geo(),
             TaxName()));
  }

  void testConstructor()
  {
    CustomerRestrictionRule rule("JJ");

    CPPUNIT_ASSERT_NO_THROW(
        CustomerRestrictionApplicator(rule, *_serviceMock, _abcd));
  }

  void testAny()
  {
    CustomerRestrictionRule rule("XX");
    CustomerRestrictionApplicator a(rule, *_serviceMock, _msk1);

    CPPUNIT_ASSERT(a.apply(*_paymentDetail));
  }

  void testJJ()
  {
    CustomerRestrictionRule rule("JJ");
    CustomerRestrictionApplicator a1(rule, *_serviceMock, _msk1);
    CustomerRestrictionApplicator a2(rule, *_serviceMock, _msk2);

    CPPUNIT_ASSERT(a1.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!a2.apply(*_paymentDetail));
  }

  void testG3()
  {
    CustomerRestrictionRule rule("G3");
    CustomerRestrictionApplicator a1(rule, *_serviceMock, _msk1);
    CustomerRestrictionApplicator a2(rule, *_serviceMock, _msk2);

    CPPUNIT_ASSERT(!a1.apply(*_paymentDetail));
    CPPUNIT_ASSERT(a2.apply(*_paymentDetail));
  }

  void testT4()
  {
    CustomerRestrictionRule rule("T4");
    CustomerRestrictionApplicator a1(rule, *_serviceMock, _msk1);
    CustomerRestrictionApplicator a2(rule, *_serviceMock, _msk2);

    CPPUNIT_ASSERT(!a1.apply(*_paymentDetail));
    CPPUNIT_ASSERT(a2.apply(*_paymentDetail));
  }

private:
  std::unique_ptr<CustomerService> _serviceMock;
  std::unique_ptr<PaymentDetail> _paymentDetail;
  type::PseudoCityCode _abcd{"ABCD"};
  type::PseudoCityCode _msk1{"MSK1"};
  type::PseudoCityCode _msk2{"MSK2"};
};

CPPUNIT_TEST_SUITE_REGISTRATION(CustomerRestrictionApplicatorTest);
}
