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

#include "DataModel/Common/Types.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "Rules/TaxOnTaxApplicator.h"
#include "Rules/TaxOnTaxRule.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "ServiceInterfaces/Services.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/PaymentDetailMock.h"
#include "TestServer/Facades/FallbackServiceServer.h"

#include <memory>
#include <set>

namespace tax
{

class TaxOnTaxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxOnTaxTest);
  CPPUNIT_TEST(applyTaxOnTax);
  CPPUNIT_TEST(applyTaxOnTaxWithFareBreaks);
  CPPUNIT_TEST(applyWithZeroIfCommandExempt);
  CPPUNIT_TEST(applyOnlyInRange);


  CPPUNIT_TEST(dontApplyIfTaxNameIsNotOnList);
  CPPUNIT_TEST(applyIfTypeSubcodeIsBlank);
  CPPUNIT_TEST(dontApplyIfServiceBaggageApplIsNegative);
  CPPUNIT_TEST(dontApplyIfServiceBaggageIsNull);
  CPPUNIT_TEST(dontApplyIfFailedItinerary);

  CPPUNIT_TEST_SUITE_END();

public:
  void
  createApplicator(RawPayments* rawPayments, std::shared_ptr<ServiceBaggage const> serviceBaggage)
  {
    type::Index itemNo = 1;
    type::Vendor vendor = "ATP";
    _rule.reset(new TaxOnTaxRule(itemNo, vendor));
    _applicator.reset(new TaxOnTaxApplicator(*_rule, *_services, serviceBaggage, true, *rawPayments));
  }

  void setUp()
  {
    _services.reset(new DefaultServices());
    _services->setFallbackService(new FallbackServiceServer());

    _rawPayments = new RawPayments(RawPayments::WithCapacity(10));
    _serviceBaggage.reset(new ServiceBaggage());

    _taxPointBegin.reset(new Geo);
    _taxPointEnd.reset(new Geo);
    _taxPointBeginRaw.reset(new Geo);
    _taxPointEndRaw.reset(new Geo);

    createPaymentDetail(type::TaxAppliesToTagInd::Blank, type::TaxPointTag::Departure);
  }

  void tearDown()
  {
    _rule.reset();
    _applicator.reset();
    delete _paymentDetail;
    delete _rawPayments;
  }

  void createPaymentDetail(type::TaxAppliesToTagInd taxAppliesToTag,
                           type::TaxPointTag taxPointTag)
  {
    _paymentDetail = new PaymentDetailMock(taxAppliesToTag);

    _paymentDetail->getMutableTaxPointsProperties().resize(2);
    _paymentDetail->getMutableTaxPointsProperties()[0].isFirst = true;
    _paymentDetail->getMutableTaxPointsProperties()[0].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[1].isLast = true;

    TaxName taxName;
    taxName.taxPointTag() = taxPointTag;
    _paymentDetail->setTaxName(taxName);
  }

  void setUpPaymentDetail(type::Index beginId, type::Index endId)
  {
    _taxPointBegin->id() = beginId;
    _taxPointEnd->id() = endId;
    _paymentDetail->setTaxPointBegin(*_taxPointBegin);
    _paymentDetail->setTaxPointEnd(*_taxPointEnd);
  }

  TaxName makeTaxName(type::TaxCode taxCode, type::TaxType taxType)
  {
    TaxName ans;
    ans.taxCode() = taxCode;
    ans.taxType() = taxType;
    ans.taxPointTag() = type::TaxPointTag::Departure;
    return ans;
  }

  void emplacePayment(RawPayments& rawPayments, TaxName& taxName, type::Index beginId, type::Index endId)
  {
    static const PaymentRuleData prData(0, type::TicketedPointTag::MatchTicketedPointsOnly,
                                        TaxableUnitTagSet::none(), 10,
                                        type::CurrencyCode(UninitializedCode),
                                        type::TaxAppliesToTagInd::Blank);
    _taxPointBeginRaw->id() = beginId;
    _taxPointEndRaw->id() = endId;

    PaymentDetail& detail = rawPayments.emplace_back(prData, *_taxPointBeginRaw, *_taxPointEndRaw, taxName);
    detail.taxEquivalentAmount() = 50;
  }

  void addServiceBaggageEntry(type::TaxCode taxCode, type::TaxTypeOrSubCode taxTypeSubcode,
                              type::ServiceBaggageAppl applTag)
  {
    ServiceBaggageEntry* entry1 = new ServiceBaggageEntry();
    entry1->applTag = applTag;
    entry1->taxCode = taxCode;
    entry1->taxTypeSubcode = taxTypeSubcode;
    _serviceBaggage->entries.push_back(entry1);
  }

  void applyTaxOnTax()
  {
    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 0, 1);
    addServiceBaggageEntry("AA", "001", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(50), _paymentDetail->totalTaxOnTaxAmount());
  }

  void applyTaxOnTaxWithFareBreaks()
  {
    delete _paymentDetail;
    createPaymentDetail(type::TaxAppliesToTagInd::BetweenFareBreaks, type::TaxPointTag::Departure);

    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 0, 1);
    addServiceBaggageEntry("AA", "001", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(50), _paymentDetail->totalTaxOnTaxAmount());
  }

  void applyWithZeroIfCommandExempt()
  {
    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 0, 1);
    _rawPayments->back().detail.setCommandExempt(type::CalcRestriction::ExemptSpecifiedTaxes);
    addServiceBaggageEntry("AA", "001", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);


    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->totalTaxOnTaxAmount());
  }

  void applyOnlyInRange()
  {
    delete _paymentDetail;
    createPaymentDetail(type::TaxAppliesToTagInd::BetweenFareBreaks, type::TaxPointTag::Departure);

    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 2, 3);
    addServiceBaggageEntry("AA", "001", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->totalTaxOnTaxAmount());
  }

  void applyOnAllIfSaleTax()
  {
    delete _paymentDetail;
    createPaymentDetail(type::TaxAppliesToTagInd::BetweenFareBreaks, type::TaxPointTag::Sale);

    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 2, 3);
    addServiceBaggageEntry("AA", "001", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(50), _paymentDetail->totalTaxOnTaxAmount());
  }

  void applyOnAllIfAllBaseFare()
  {
    delete _paymentDetail;
    createPaymentDetail(type::TaxAppliesToTagInd::AllBaseFare, type::TaxPointTag::Departure);

    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 2, 3);
    addServiceBaggageEntry("AA", "001", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(50), _paymentDetail->totalTaxOnTaxAmount());
  }

  void dontApplyIfTaxNameIsNotOnList()
  {
    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 0, 1);
    addServiceBaggageEntry("BB", "001", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->totalTaxOnTaxAmount());
  }

  void applyIfTypeSubcodeIsBlank()
  {
    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 0, 1);
    addServiceBaggageEntry("AA", "", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(50), _paymentDetail->totalTaxOnTaxAmount());
  }

  void dontApplyIfServiceBaggageApplIsNegative()
  {
    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 0, 1);
    addServiceBaggageEntry("AA", "001", type::ServiceBaggageAppl::Negative);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->totalTaxOnTaxAmount());
  }

  void dontApplyIfServiceBaggageIsNull()
  {
    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 0, 1);

    createApplicator(_rawPayments, std::shared_ptr<ServiceBaggage>());
    setUpPaymentDetail(0, 1);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->totalTaxOnTaxAmount());
  }

  void dontApplyIfFailedItinerary()
  {
    TaxName taxName = makeTaxName("AA", "001");
    emplacePayment(*_rawPayments, taxName, 0, 1);
    addServiceBaggageEntry("AA", "001", type::ServiceBaggageAppl::Positive);

    createApplicator(_rawPayments, _serviceBaggage);
    setUpPaymentDetail(0, 1);
    _paymentDetail->getMutableItineraryDetail().setFailedRule(new TaxOnTaxRule(1, "ATP"));

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), _paymentDetail->taxOnTaxItems().size());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->totalTaxOnTaxAmount());
  }

private:
  std::unique_ptr<TaxOnTaxRule> _rule;
  std::unique_ptr<TaxOnTaxApplicator> _applicator;
  std::unique_ptr<Geo> _taxPointBegin;
  std::unique_ptr<Geo> _taxPointEnd;
  std::unique_ptr<Geo> _taxPointBeginRaw;
  std::unique_ptr<Geo> _taxPointEndRaw;
  PaymentDetailMock* _paymentDetail;
  RawPayments* _rawPayments;
  std::shared_ptr<ServiceBaggage> _serviceBaggage;
  std::unique_ptr<tax::DefaultServices> _services;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxOnTaxTest);
} // namespace tax
