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

#include "test/include/CppUnitHelperMacros.h"

#include "Processor/CheckRemove.h"
#include "Rules/PaymentDetail.h"
#include "Rules/RulesGroups.h"
#include "test/PaymentDetailMock.h"

#include <memory>

namespace tax
{

class CheckRemoveTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CheckRemoveTest);

  CPPUNIT_TEST(testPaymentWithRulesValidation);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _geo1.reset(new Geo());
    _geo1->id() = 2;
    _geo2.reset(new Geo());
    _geo2->id() = 3;

    _taxName.reset(new TaxName());
    _taxName->taxCode() = "AA";
    _taxName->taxType() = "001";

    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(type::SeqNo(),
                                          type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                                          TaxableUnitTagSet::none(),
                                          0,
                                          type::CurrencyCode(UninitializedCode),
                                          type::TaxAppliesToTagInd::Blank),
                          *_geo1,
                          *_geo2,
                          *_taxName));
  }

  void tearDown() {}

  void testPaymentWithRulesValidation()
  {
    TaxName taxName;
    Geo geo1;
    Geo geo2;
    PaymentDetailMock rawPaymentDetail;
    PaymentWithRules paymentWithRules(
      rawPaymentDetail, nullptr);

    CheckRemove checkRemove(_paymentDetail.get());
    CPPUNIT_ASSERT_EQUAL(false, checkRemove(paymentWithRules));

    taxName.taxCode() = "AA";
    taxName.taxType() = "001";
    geo1.id() = 2;
    rawPaymentDetail.setTaxName(taxName);
    rawPaymentDetail.setTaxPointBegin(geo1);
    CPPUNIT_ASSERT_EQUAL(true, checkRemove(paymentWithRules));
  }

private:
  std::unique_ptr<PaymentDetail> _paymentDetail;
  std::unique_ptr<TaxName> _taxName;
  std::unique_ptr<Geo> _geo1;
  std::unique_ptr<Geo> _geo2;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CheckRemoveTest);
} // namespace tax
