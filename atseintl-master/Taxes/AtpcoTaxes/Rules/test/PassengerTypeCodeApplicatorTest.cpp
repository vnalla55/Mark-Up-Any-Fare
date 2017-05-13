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

#include "Rules/PassengerTypeCodeApplicator.h"
#include "Rules/PassengerTypeCodeRule.h"
#include "DataModel/Common/Types.h"
#include "TestServer/Facades/PassengerTypesServiceServer.h"
#include "test/PaymentDetailMock.h"
#include "test/LocServiceMock.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "TestServer/Facades/PassengerMapperServer.h"

#include <memory>
#include <set>

namespace tax
{
class PassengerTypeCodeApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PassengerTypeCodeApplicatorTest);
  CPPUNIT_TEST(dontMatchIfNoRows);
  CPPUNIT_TEST(dontMatchIfNoRowMatches);
  CPPUNIT_TEST(matchIfFirstRowMatches);
  CPPUNIT_TEST(matchIfMiddleRowMatches);
  CPPUNIT_TEST(matchIfLastRowMatches);

  CPPUNIT_TEST(matchPassengerAge);
  CPPUNIT_TEST(matchNation);
  CPPUNIT_TEST(matchResidence);
  CPPUNIT_TEST(matchEmployment);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetail.reset(new PaymentDetailMock());
    _loc1.reset(new Geo());
    _loc1->id() = 0;
    _paymentDetail->setTaxPointBegin(*_loc1);
    _rule.reset(new PassengerTypeCodeRule("ATP", 17, type::TaxRemittanceId::Sale));
    _itin.reset(new Itin);
    _farePath.reset(new FarePath);
    _itin->farePath() = _farePath.get();
    _fare.reset(new Fare);
    _fare->outputPtc() = "INF";
    _farePath->validatingCarrier() = "LH";
    _farePath->outputPtc() = "ADT";
    _farePath->fareUsages().push_back(FareUsage());
    _farePath->fareUsages().back().fare() = _fare.get();
    _itin->passenger() = &_passenger;
    _geoPathMapping.reset(new GeoPathMapping);
    _itin->geoPathMapping() = _geoPathMapping.get();
    _geoPathMapping->mappings().push_back(Mapping());
    _geoPathMapping->mappings().back().maps().push_back(Map());
    _geoPathMapping->mappings().back().maps().back().index() = 0;
    _geoPathMapping->mappings().back().maps().push_back(Map());
    _geoPathMapping->mappings().back().maps().back().index() = 1;
    _date = type::Date(2014, 5, 2);
    _paxMapper.reset(new PassengerMapperServer(_mappings));
    _passenger._code = "ADT";
    _passenger._birthDate = type::Date(1984, 4, 20);
    _passenger._stateCode = "";
    _passenger._passengerStatus._nationality = "PL";
    _passenger._passengerStatus._residency = "FRA";
    _passenger._passengerStatus._employment = "BE";

    _items.reset(new PassengerTypeCodeItems());

    _locServiceMock.reset(new LocServiceMock());
  }

  void tearDown() { _mappings.clear(); }

  void addPassengerEntry(type::PtcApplTag applTag,
                         const type::PassengerCode& passengerType,
                         int16_t minAge,
                         int16_t maxAge,
                         type::PassengerStatusTag psTag,
                         const type::LocType& locationType,
                         const type::LocZoneText& locationCode,
                         type::PtcMatchIndicator matchIndicator)
  {
    std::unique_ptr<PassengerTypeCodeItem> item(new PassengerTypeCodeItem);
    item->applTag = applTag;
    item->passengerType = passengerType;
    item->minimumAge = minAge;
    item->maximumAge = maxAge;
    item->statusTag = psTag;
    item->location.type() = locationType;
    item->location.code() = locationCode;
    item->matchIndicator = matchIndicator;
    _items->push_back(item.release());
  }

  void prepareApplicator()
  {
    _applicator.reset(new PassengerTypeCodeApplicator(
        *(_rule.get()), *_itin, _date, *_paxMapper, _items, *_locServiceMock, "DFW"));
  }

  void dontMatchIfNoRows()
  {
    prepareApplicator();
    CPPUNIT_ASSERT(!_applicator->apply(*_paymentDetail));
  }

  void dontMatchIfNoRowMatches()
  {
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    prepareApplicator();
    CPPUNIT_ASSERT(!_applicator->apply(*_paymentDetail));
  }

  void matchIfFirstRowMatches()
  {
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "ADT",
                      13,
                      -1,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    prepareApplicator();
    CPPUNIT_ASSERT(_applicator->apply(*_paymentDetail));
  }

  void matchIfMiddleRowMatches()
  {
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "ADT",
                      13,
                      -1,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);

    prepareApplicator();
    CPPUNIT_ASSERT(_applicator->apply(*_paymentDetail));
  }

  void matchIfLastRowMatches()
  {
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "INF",
                      0,
                      2,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "ADT",
                      13,
                      -1,
                      type::PassengerStatusTag::Blank,
                      type::LocType::Blank,
                      type::LocZoneText(UninitializedCode),
                      type::PtcMatchIndicator::Input);

    prepareApplicator();
    CPPUNIT_ASSERT(_applicator->apply(*_paymentDetail));
  }

  void matchPassengerAge()
  {
    type::Date birthDate(1980, 4, 4);
    type::Date saleDate(2014, 5, 2);
    CPPUNIT_ASSERT( PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 34, 34));
    CPPUNIT_ASSERT( PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, -1, 34));
    CPPUNIT_ASSERT( PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 34, -1));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, -1, 33));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 35, -1));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate,  0,  2));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 35, -1));

    saleDate = type::Date(2014, 4, 4);
    CPPUNIT_ASSERT( PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 34, 34));
    CPPUNIT_ASSERT( PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, -1, 34));
    CPPUNIT_ASSERT( PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 34, -1));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, -1, 33));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 35, -1));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate,  0,  2));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 35, -1));

    saleDate = type::Date(2014, 4, 3);
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 34, 34));
    CPPUNIT_ASSERT( PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, -1, 34));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, 34, -1));
    CPPUNIT_ASSERT( PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate, -1, 33));
    CPPUNIT_ASSERT(!PassengerTypeCodeApplicator::ageMatches(saleDate, birthDate,  0,  2));
  }

  void matchEmployment()
  {
    prepareApplicator();
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "ADT",
                      0,
                      2,
                      type::PassengerStatusTag::Employee,
                      type::LocType::Nation,
                      "PL",
                      type::PtcMatchIndicator::Input);
    _passenger._passengerStatus._employment = "BE";
    CPPUNIT_ASSERT(!_applicator->locationMatches(_passenger, _items->back()));
    _passenger._passengerStatus._employment = "PL";
    CPPUNIT_ASSERT(_applicator->locationMatches(_passenger, _items->back()));
    _items->back().location.code() = "BE";
    CPPUNIT_ASSERT(!_applicator->locationMatches(_passenger, _items->back()));
    _passenger._passengerStatus._employment = "BE";
    CPPUNIT_ASSERT(_applicator->locationMatches(_passenger, _items->back()));
  }

  void matchResidence()
  {
    prepareApplicator();
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "ADT",
                      0,
                      2,
                      type::PassengerStatusTag::Resident,
                      type::LocType::Nation,
                      "KRK",
                      type::PtcMatchIndicator::Input);
    _passenger._passengerStatus._residency = "FRA";
    CPPUNIT_ASSERT(!_applicator->locationMatches(_passenger, _items->back()));
    _passenger._passengerStatus._residency = "KRK";
    CPPUNIT_ASSERT(_applicator->locationMatches(_passenger, _items->back()));
    _items->back().location.code() = "BRL";
    CPPUNIT_ASSERT(!_applicator->locationMatches(_passenger, _items->back()));
    _passenger._passengerStatus._residency = "BRL";
    CPPUNIT_ASSERT(_applicator->locationMatches(_passenger, _items->back()));
  }

  void matchNation()
  {
    prepareApplicator();
    addPassengerEntry(type::PtcApplTag::Permitted,
                      "ADT",
                      0,
                      2,
                      type::PassengerStatusTag::National,
                      type::LocType::Nation,
                      "PL",
                      type::PtcMatchIndicator::Input);
    _passenger._passengerStatus._nationality = "BE";
    CPPUNIT_ASSERT(!_applicator->locationMatches(_passenger, _items->back()));
    _passenger._passengerStatus._nationality = "PL";
    CPPUNIT_ASSERT(_applicator->locationMatches(_passenger, _items->back()));
    _items->back().location.code() = "BE";
    CPPUNIT_ASSERT(!_applicator->locationMatches(_passenger, _items->back()));
    _passenger._passengerStatus._nationality = "BE";
    CPPUNIT_ASSERT(_applicator->locationMatches(_passenger, _items->back()));
  }

private:
  std::unique_ptr<PassengerTypeCodeRule> _rule;
  std::unique_ptr<PassengerTypeCodeApplicator> _applicator;
  std::unique_ptr<Itin> _itin;
  std::unique_ptr<FarePath> _farePath;
  std::unique_ptr<Fare> _fare;
  std::unique_ptr<GeoPathMapping> _geoPathMapping;
  std::unique_ptr<Geo> _loc1;
  std::unique_ptr<PassengerMapper> _paxMapper;
  type::Date _date;
  Passenger _passenger;
  std::unique_ptr<PaymentDetailMock> _paymentDetail;
  std::unique_ptr<LocServiceMock> _locServiceMock;
  std::shared_ptr<PassengerTypeCodeItems> _items;
  boost::ptr_vector<tax::PaxTypeMapping> _mappings;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PassengerTypeCodeApplicatorTest);
} // namespace tax
