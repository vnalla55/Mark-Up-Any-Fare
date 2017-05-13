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

#include "Rules/PointOverlappingForItineraryApplicator.h"
#include "Rules/PointOverlappingForYqYrApplicator.h"
#include "Rules/TaxData.h"
#include "test/PaymentDetailMock.h"

#include <memory>

namespace tax
{

namespace
{
  const type::CurrencyCode noCurrency (UninitializedCode);
}

class PointOverlappingApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PointOverlappingApplicatorTest);

  CPPUNIT_TEST(testDepartureDifferentTaxName);
  CPPUNIT_TEST(testDepartureStartingIn2);
  CPPUNIT_TEST(testDepartureStartingIn2Failed);
  CPPUNIT_TEST(testDepartureStartingIn2Exempt);
  CPPUNIT_TEST(testDepartureStartingIn4);
  CPPUNIT_TEST(testArrival1);
  CPPUNIT_TEST(testArrival2);
  CPPUNIT_TEST(testArrival3);
  CPPUNIT_TEST(testArrival5);
  CPPUNIT_TEST(testArrival6);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _rawPayments = new RawPayments(RawPayments::WithCapacity(10));

    _rule.reset(new PointOverlappingForItineraryRule());
    _applicatorItin.reset(new PointOverlappingForItineraryApplicator(_rule.get(), *_rawPayments));
    _applicatorYqYr.reset(new PointOverlappingForYqYrApplicator(_rule.get(), *_rawPayments));

    _paymentDetail1.reset(new PaymentDetailMock());
    _paymentDetail2.reset(new PaymentDetailMock());

    _geo1 = new Geo();
    _geo2 = new Geo();
    _geo1->id() = 2;
    _geo1->loc().tag() = type::TaxPointTag::Departure;
    _geo2->id() = 5;
    _geo2->loc().tag() = type::TaxPointTag::Arrival;
    _geo3 = new Geo();
    _geo4 = new Geo();
    _geo3->id() = 7;
    _geo3->loc().tag() = type::TaxPointTag::Arrival;
    _geo4->id() = 2;
    _geo4->loc().tag() = type::TaxPointTag::Departure;

    _taxName1 = new TaxName();
    _taxName1->taxPointTag() = type::TaxPointTag::Departure;
    _taxName1->taxCode() = "AA";
    _taxName1->taxType() = "001";
    _taxName2 = new TaxName();
    _taxName2->taxPointTag() = type::TaxPointTag::Arrival;
    _taxName2->taxCode() = "AB";
    _taxName2->taxType() = "001";

    ((PaymentDetailMock*)_paymentDetail1.get())->setTaxPointBegin(*_geo1);
    ((PaymentDetailMock*)_paymentDetail1.get())->setTaxPointEnd(*_geo2);
    ((PaymentDetailMock*)_paymentDetail1.get())->setTaxName(*_taxName1);
    ((PaymentDetailMock*)_paymentDetail1.get())->setTaxableUnit(type::TaxableUnit::Itinerary);

    ((PaymentDetailMock*)_paymentDetail2.get())->setTaxPointBegin(*_geo2);
    ((PaymentDetailMock*)_paymentDetail2.get())->setTaxPointEnd(*_geo1);
    ((PaymentDetailMock*)_paymentDetail2.get())->setTaxName(*_taxName2);
    ((PaymentDetailMock*)_paymentDetail2.get())->setTaxableUnit(type::TaxableUnit::Itinerary);
  }

  void tearDown()
  {
    delete _geo1;
    delete _geo2;
    delete _geo3;
    delete _geo4;
    delete _rawPayments;
    delete _taxName1;
    delete _taxName2;
    _paymentDetail1.reset();
    _paymentDetail2.reset();
  }

  TaxableUnitTagSet itineraryTag()
  {
    TaxableUnitTagSet tut = TaxableUnitTagSet::none();
    tut.setTag(type::TaxableUnit::Itinerary);
    return tut;
  }

  PaymentDetail& emplaceDetail1(RawPayments& rawPayments)
  {
    PaymentDetail& detail = rawPayments.emplace_back(
      PaymentRuleData(type::SeqNo(),
                      type::TicketedPointTag::MatchTicketedPointsOnly,
                      itineraryTag(),
                      0,
                      noCurrency,
                      type::TaxAppliesToTagInd::Blank),
      *_geo1,
      *_geo2,
      *_taxName1);

    TaxableYqYrs& taxableYqYrs1 = detail.getMutableYqYrDetails();
    std::vector<type::Index> ids;
    ids.push_back(1);
    ids.push_back(3);
    taxableYqYrs1.init(std::vector<TaxableYqYr>(2), ids);
    std::vector<TaxableData>& data = taxableYqYrs1._data;
    data[0]._taxPointLoc2 = _geo2;
    data[1]._taxPointLoc2 = _geo3;
    return detail;
  }

  PaymentDetail& emplaceDetail2(RawPayments& rawPayments)
  {
    return rawPayments.emplace_back(
      PaymentRuleData(type::SeqNo(),
                      type::TicketedPointTag::MatchTicketedPointsOnly,
                      itineraryTag(),
                      0,
                      noCurrency,
                      type::TaxAppliesToTagInd::Blank),
      *_geo2,
      *_geo1,
      *_taxName2);
  }

  enum ExemptWhom{ExemptNone, ExemptDetail1, ExemptDetail2};

  void testDeparture(type::Index geoId,
                     type::Index yqYrId,
                     const TaxName& taxName,
                     const BusinessRule* rule,
                     bool onItin,
                     bool onYqYr,
                     bool first,
                     bool second,
                     ExemptWhom exempt = ExemptNone)
  {
    PaymentDetail& payDetail2 = emplaceDetail2(*_rawPayments); // Arr, geo 5-2
    PaymentDetail& payDetail1 = emplaceDetail1(*_rawPayments); // Dep, geo 2-5

    if (exempt == ExemptDetail1)
      payDetail1.setExempt();
    else if (exempt == ExemptDetail2)
      payDetail2.setExempt();

    Geo geo;
    geo.id() = geoId;
    geo.loc().tag() = type::TaxPointTag::Departure;
    TaxableData data;
    TaxableItinerary taxableItinerary(data);

    PaymentDetail& paymentDetail = _rawPayments->emplace_back(
      PaymentRuleData(0, type::TicketedPointTag::MatchTicketedPointsOnly,
                      TaxableUnitTagSet::none(), 0, noCurrency, type::TaxAppliesToTagInd::Blank),
      geo,
      Geo(),
      taxName); // this is different than the original 

    TaxableYqYrs& taxableYqYrs = paymentDetail.getMutableYqYrDetails();
    std::vector<TaxableYqYr> subjects(2);
    std::vector<type::Index> ids;
    ids.push_back(yqYrId);
    ids.push_back(3);
    taxableYqYrs.init(subjects, ids);

    payDetail1.setFailedRule(rule);
    if (rule)
      payDetail1.getMutableYqYrDetails().setFailedRule(0, *rule);

    _applicatorItin->apply(paymentDetail);
    CPPUNIT_ASSERT_EQUAL(onItin, paymentDetail.getItineraryDetail().isFailedRule());
    _applicatorYqYr->apply(paymentDetail);
    CPPUNIT_ASSERT_EQUAL(onYqYr, paymentDetail.getYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT_EQUAL(first, taxableYqYrs.isFailedRule(0));
    CPPUNIT_ASSERT_EQUAL(second, taxableYqYrs.isFailedRule(1));
  }

  void testDepartureDifferentTaxName()
  {
    testDeparture(0, 2, TaxName(), 0, false, false, false, false);
  }

  void testDepartureStartingIn2()
  {
    testDeparture(2, 2, *_taxName1, 0, true, false, false, true);
  }

  void testDepartureStartingIn2Failed()
  {
    ExemptTagRule rule;
    testDeparture(2, 1, *_taxName1, &rule, false, false, false, true);
  }

  void testDepartureStartingIn2Exempt()
  {
    testDeparture(2, 1, *_taxName1, 0, true, true, true, true, ExemptDetail1);
  }

  void testDepartureStartingIn4()
  {
    ExemptTagRule rule;
    testDeparture(4, 1, *_taxName1, &rule, false, false, false, false);
  }

  void testArrival(type::Index geoId,
                   const TaxName& taxName,
                   const BusinessRule* rule,
                   bool onItin,
                   ExemptWhom exempt = ExemptNone)
  {
    PaymentDetail& payDetail1 = emplaceDetail1(*_rawPayments); // Dep, geo 2-5
    PaymentDetail& payDetail2 = emplaceDetail2(*_rawPayments); // Arr, geo 5-2

    if (exempt == ExemptDetail1)
      payDetail1.setExempt();
    else if (exempt == ExemptDetail2)
      payDetail2.setExempt();

    Geo geo;
    geo.id() = geoId;
    geo.loc().tag() = type::TaxPointTag::Arrival;
    TaxableData data;
    TaxableItinerary taxableItinerary(data);

    _rawPayments->emplace_back(
      PaymentRuleData(0, type::TicketedPointTag::MatchTicketedPointsOnly, TaxableUnitTagSet::none(),
                      0, noCurrency, type::TaxAppliesToTagInd::Blank),
      geo,
      Geo(),
      taxName);

    payDetail2.setFailedRule(rule);

    _applicatorItin->apply(_rawPayments->back().detail);
    CPPUNIT_ASSERT_EQUAL(onItin, _rawPayments->back().detail.isFailedRule());
  }

  void testArrival1() //
  {
    TaxName taxName;
    testArrival(5, taxName, 0, false);
  }

  void testArrival2()
  {
    testArrival(5, *_taxName2, 0, true);
  }

  void testArrival3() //
  {
    testArrival(3, *_taxName2, 0, false);
  }

  void testArrival5()
  {
    ExemptTagRule rule;
    testArrival(5, *_taxName2, &rule, false);
  }

  void testArrival6() //
  {
    testArrival(5, *_taxName2, 0, true, ExemptDetail2);
  }

private:
  std::unique_ptr<PointOverlappingForItineraryRule> _rule;
  std::unique_ptr<PointOverlappingForItineraryApplicator> _applicatorItin;
  std::unique_ptr<PointOverlappingForYqYrApplicator> _applicatorYqYr;
  RawPayments* _rawPayments;
  std::shared_ptr<PaymentDetail> _paymentDetail1;
  std::shared_ptr<PaymentDetail> _paymentDetail2;
  TaxName* _taxName1;
  TaxName* _taxName2;
  Geo* _geo1;
  Geo* _geo2;
  Geo* _geo3;
  Geo* _geo4;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PointOverlappingApplicatorTest);
} // namespace tax
