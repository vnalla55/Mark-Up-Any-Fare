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

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/BusinessRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/TaxPointLoc2StopoverTagApplicator.h"
#include "Rules/TaxPointLoc2StopoverTagRule.h"
#include "ServiceInterfaces/MileageGetter.h"
#include "ServiceInterfaces/MileageService.h"
#include "test/ServicesMock.h"
#include "TestServer/Facades/FallbackServiceServer.h"

#include <memory>
#include <gmock/gmock.h>

using testing::_;
using testing::StrictMock;
using testing::Return;
using testing::ReturnRef;

namespace tax
{
namespace
{
class MileageServiceMock : public MileageService
{
public:
  MOCK_CONST_METHOD5(getMiles,
                     const std::vector<GeoIdMile>&(const GeoPath&,
                                                   const std::vector<tax::FlightUsage>&,
                                                   const type::Index,
                                                   const type::Index,
                                                   const type::Timestamp&));
  MOCK_CONST_METHOD3(getMileageGetter,
                     const MileageGetter&(const GeoPath&,
                                          const std::vector<FlightUsage>&,
                                          const type::Timestamp&));
};
}

class TaxPointLoc2StopoverTagApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc2StopoverTagApplicatorTest);

  CPPUNIT_TEST(testStopover_Departure_apply);
  CPPUNIT_TEST(testStopover_Departure_applyOpen);
  CPPUNIT_TEST(testStopover_Arrival_apply);
  CPPUNIT_TEST(testStopover_Arrival_applyOpen);

  CPPUNIT_TEST(testFareBreak_Departure_applyInDestination);
  CPPUNIT_TEST(testFareBreak_Departure_applyInMiddle);
  CPPUNIT_TEST(testFareBreak_Departure_applyInMiddleOpen);
  CPPUNIT_TEST(testFareBreak_Departure_applyInMiddleTicketedOnly);
  CPPUNIT_TEST(testFareBreak_Arrival_applyInDestination);
  CPPUNIT_TEST(testFareBreak_Arrival_applyInMiddle);
  CPPUNIT_TEST(testFareBreak_Arrival_applyInMiddleOpen);
  CPPUNIT_TEST(testFareBreak_Arrival_applyInMiddleTicketedOnly);

  CPPUNIT_TEST(testFareBreak_Departure_applyInMiddle_TaxMatchingAppl06_noStop);
  CPPUNIT_TEST(testFareBreak_Departure_applyInMiddle_TaxMatchingAppl06_isStop);
  CPPUNIT_TEST(testFareBreak_Arrival_applyInMiddle_TaxMatchingAppl06_noStop);
  CPPUNIT_TEST(testFareBreak_Arrival_applyInMiddle_TaxMatchingAppl06_isStop);

  CPPUNIT_TEST(testFurthest_Departure_apply);
  CPPUNIT_TEST(testFurthest_Departure_applyTicketedOnly);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl03_OneWay);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl03_RoundInbound);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl03_RoundOutbound);

  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl05_withoutSurface);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl05_withSurface);

  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_no48stops);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_48stopMiddle);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_48stopBegin);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_48stopBeforeBegin);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd_surfaceIntDom);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd_surfaceDomInt);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd_ticketedOnly);
  CPPUNIT_TEST(testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd_matchTickAndUntick);

  CPPUNIT_TEST(testFurthest_Arrival_apply);
  CPPUNIT_TEST(testFurthest_Arrival_applyTicketedOnly);
  CPPUNIT_TEST(testFurthest_Arrival_TaxMatchingAppl03_OneWay);
  CPPUNIT_TEST(testFurthest_Arrival_TaxMatchingAppl03_RoundInbound);
  CPPUNIT_TEST(testFurthest_Arrival_TaxMatchingAppl03_RoundOutbound);

  CPPUNIT_TEST(testStopover_Departure_applyOnYqYr);
  CPPUNIT_TEST(testStopover_Departure_applyOnYqYrPart);
  CPPUNIT_TEST(testStopover_Departure_applyOnYqYrOpen);
  CPPUNIT_TEST(testStopover_Arrival_applyOnYqYr);
  CPPUNIT_TEST(testStopover_Arrival_applyOnYqYrPart);
  CPPUNIT_TEST(testStopover_Arrival_applyOnYqYrOpen);

  CPPUNIT_TEST(testFareBreak_Departure_applyOnYqYrs);
  CPPUNIT_TEST(testFareBreak_Departure_applyOnYqYrsLimit);
  CPPUNIT_TEST(testFareBreak_Departure_applyOnYqYrsLimitTicketedOnly);
  CPPUNIT_TEST(testFareBreak_Arrival_applyOnYqYrs);
  CPPUNIT_TEST(testFareBreak_Arrival_applyOnYqYrsLimit);
  CPPUNIT_TEST(testFareBreak_Arrival_applyOnYqYrsLimitTicketedOnly);

  CPPUNIT_TEST(testStopover_Departure_applyOnOC);
  CPPUNIT_TEST(testStopover_Arrival_applyOnOC);

  CPPUNIT_TEST(testFareBreak_Departure_applyOnOC);
  CPPUNIT_TEST(testFareBreak_Departure_applyOnOCTicketedOnly);
  CPPUNIT_TEST(testFareBreak_Departure_applyIgnoredOC);
  CPPUNIT_TEST(testFareBreak_Arrival_applyOnOC);
  CPPUNIT_TEST(testFareBreak_Arrival_applyOnOCTicketedOnly);
  CPPUNIT_TEST(testFareBreak_Arrival_applyIgnoredOC);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _loc2StopoverTag.reset(new type::Loc2StopoverTag(type::Loc2StopoverTag::Blank));
    _taxPointTagArrival.reset(new type::TaxPointTag(type::TaxPointTag::Arrival));
    _taxPointTagDeparture.reset(new type::TaxPointTag(type::TaxPointTag::Departure));
    _taxMatchingApplTag.reset(new type::TaxMatchingApplTag());
    _taxProcessingApplTag.reset(new type::TaxProcessingApplTag());
    _taxPointLoc1Zone.reset(new LocZone());
    _taxPointLoc2Zone.reset(new LocZone());
    _taxableUnits.reset(new TaxableUnitTagSet(TaxableUnitTagSet::none()));

    _servicesMock.reset(new ServicesMock());
    _mileageServiceMock = new StrictMock<MileageServiceMock>();
    _servicesMock->setMileageService(_mileageServiceMock);
    _servicesMock->setFallbackService(new FallbackServiceServer());

    _seqNo.reset(new type::SeqNo());
    _ticketedPointTag.reset(
        new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints));
    _taxName = new TaxName();
    _itin.reset(new Itin());

    setUpGeosAndFlights(8);
  }

  void tearDown()
  {
    _properties.clear();
    delete _taxName;
    delete _geoPath;
  }

  void setUpRules()
  {
    _taxPointLoc2ArrivalRule.reset(new TaxPointLoc2StopoverTagRule(
        *_loc2StopoverTag, *_taxMatchingApplTag, *_taxPointTagArrival, *_taxProcessingApplTag,
        *_taxPointLoc1Zone, *_taxPointLoc2Zone));
    _taxPointLoc2DepartureRule.reset(new TaxPointLoc2StopoverTagRule(
        *_loc2StopoverTag, *_taxMatchingApplTag, *_taxPointTagDeparture, *_taxProcessingApplTag,
        *_taxPointLoc1Zone, *_taxPointLoc2Zone));
  }

  void setUpGeosAndFlights(int numOfGeos)
  {
    assert(numOfGeos % 2 == 0);

    _geoPath = new GeoPath();
    _geoPath->id() = 0;
    for (int i = 0; i < numOfGeos; ++i)
    {
      _geoPath->geos().push_back(Geo());
      _geoPath->geos()[i].id() = i;
      _geoPath->geos()[i].loc().tag() =
          (i % 2) ? type::TaxPointTag::Arrival : type::TaxPointTag::Departure;
    }

    for (int i = 1; i < numOfGeos; ++i)
    {
      _geoPath->geos()[i - 1].setNext(&_geoPath->geos()[i]);
      _geoPath->geos()[i].setPrev(&_geoPath->geos()[i - 1]);
    }

    _flights.clear();
    _flightUsages.clear();

    int numFlights = numOfGeos / 2;
    _flights.resize(numFlights);
    _flightUsages.resize(numFlights);

    _itin->travelOriginDate() = type::Date(2016, 11, 4);
    for (int i = 0; i < numFlights; ++i)
    {
      _flights[i].departureTime() = type::Time(8, 00);
      _flights[i].arrivalTime() = type::Time(21, 00);
      _flightUsages[i].flight() = &_flights[i];
      _flightUsages[i].markDepartureDate(type::Date(2016, 11, static_cast<short>(4 + i)));
    }

    _itin->geoPath() = _geoPath;
    _itin->geoPathRefId() = _geoPath->id();
    _itin->flightUsages() = _flightUsages;

    _properties.resize(numOfGeos);
    _properties.front().isFirst = _properties.front().isFareBreak = true;
    _properties.back().isLast = _properties.back().isFareBreak = true;
  }

  void commonSetupForTag07Tests()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "07";
    *_taxProcessingApplTag = "07";
    _taxName->taxCode() = "C9";
    _taxName->taxPointTag() = type::TaxPointTag::Departure;
    _taxableUnits->setTag(type::TaxableUnit::Itinerary);
    *_taxPointLoc1Zone = LocZone(type::LocType::Nation, type::LocZoneText("BS"));
    *_taxPointLoc2Zone = LocZone(type::LocType::Nation, type::LocZoneText("BS"));

    delete _geoPath;
    // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL
    setUpGeosAndFlights(10);
    _geoPath->geos()[0].loc().code() = "PHL";
    _geoPath->geos()[0].loc().nation() = "US";
    _geoPath->geos()[1].loc().code() = _geoPath->geos()[2].loc().code() = "FPO";
    _geoPath->geos()[1].loc().nation() = _geoPath->geos()[2].loc().nation() = "BS";
    _geoPath->geos()[3].loc().code() = _geoPath->geos()[4].loc().code() = "MHH";
    _geoPath->geos()[3].loc().nation() = _geoPath->geos()[4].loc().nation() = "BS";
    _geoPath->geos()[5].loc().code() = _geoPath->geos()[6].loc().code() = "NAS";
    _geoPath->geos()[5].loc().nation() = _geoPath->geos()[6].loc().nation() = "BS";
    _geoPath->geos()[7].loc().code() = _geoPath->geos()[8].loc().code() = "GHB";
    _geoPath->geos()[7].loc().nation() = _geoPath->geos()[8].loc().nation() = "BS";
    _geoPath->geos()[9].loc().code() = "FLL";
    _geoPath->geos()[9].loc().nation() = "US";
    _roundTripOrOpenJaw = true;
  }

  TaxPointLoc2StopoverTagApplicator getApplicator(const TaxPointLoc2StopoverTagRule& rule)
  {
    TaxPointLoc2StopoverTagApplicator applicator(rule, *_itin, *_servicesMock);
    return applicator;
  }

  void setUpPaymentDetail(type::Index beginId, type::Index endId)
  {
    _paymentDetail.reset(new PaymentDetail(
        PaymentRuleData(*_seqNo, *_ticketedPointTag, *_taxableUnits, 0,
            type::CurrencyCode(UninitializedCode),
            type::TaxAppliesToTagInd::Blank),
        _geoPath->geos()[beginId],
        _geoPath->geos()[endId],
        *_taxName));

    _paymentDetail->roundTripOrOpenJaw() = _roundTripOrOpenJaw;
    _paymentDetail->getMutableTaxPointsProperties() = _properties;
  }

  void addOc(type::OptionalServiceTag ocType, type::Index beginId, type::Index endId)
  {
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = ocType;
    _paymentDetail->optionalServiceItems().back().setTaxPointBegin(_geoPath->geos()[beginId]);
    _paymentDetail->optionalServiceItems().back().setTaxPointLoc2(_geoPath->geos()[endId]);
    _paymentDetail->optionalServiceItems().back().setTaxPointEnd(_geoPath->geos()[endId]);
  }

  void addYqYr(type::Index begin,
               type::Index end,
               type::TaxCode code,
               type::YqYrType type,
               type::MoneyAmount amount,
               bool taxIncluded)
  {
    TaxableYqYrs& taxableYqYrs = _paymentDetail->getMutableYqYrDetails();
    taxableYqYrs._subject.push_back(TaxableYqYr(code, type, taxIncluded, amount));
    std::vector<type::Index> ids = taxableYqYrs._ids;
    ids.push_back(ids.size());
    taxableYqYrs.init(taxableYqYrs._subject, ids);
    taxableYqYrs._ranges.push_back(std::make_pair(begin, end));
  }

  void verify(const TaxPointLoc2StopoverTagApplicator& applicator,
              bool applyResult,
              bool isFailed,
              type::Index taxPointEndId)
  {
    CPPUNIT_ASSERT_EQUAL(applyResult, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(isFailed, _paymentDetail->getItineraryDetail().isFailedRule());
    CPPUNIT_ASSERT(_paymentDetail->getItineraryDetail()._data->_taxPointLoc2);
    CPPUNIT_ASSERT_EQUAL(_paymentDetail->getItineraryDetail()._data->_taxPointLoc2,
                         _paymentDetail->getItineraryDetail()._data->_taxPointEnd);
    CPPUNIT_ASSERT_EQUAL(taxPointEndId,
                         _paymentDetail->getItineraryDetail()._data->_taxPointEnd->id());
  }

  void verifyArrival(type::Index geoId,
                     bool applyResult,
                     bool isFailed,
                     type::Index taxPointEndId)
  {
    setUpPaymentDetail(geoId, geoId - 1);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2ArrivalRule),
           applyResult,
           isFailed,
           taxPointEndId);
  }

  void verifyDeparture(type::Index geoId,
                       bool applyResult,
                       bool isFailed,
                       type::Index taxPointEndId)
  {
    setUpPaymentDetail(geoId, geoId + 1);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2DepartureRule),
           applyResult,
           isFailed,
           taxPointEndId);
  }

  void verifyOc(const TaxPointLoc2StopoverTagApplicator& applicator,
                bool applyResult,
                bool isFailed,
                bool areServicesFailed,
                type::Index taxPointEndId)
  {
    verify(applicator, applyResult, isFailed, taxPointEndId);
    CPPUNIT_ASSERT_EQUAL(areServicesFailed, _paymentDetail->areAllOptionalServicesFailed());
  }

  void verifyYqYr(type::Index yqYrId,
                  bool isFailed,
                  type::Index yqYrEndId)
  {
    CPPUNIT_ASSERT(yqYrId < _paymentDetail->getYqYrDetails()._subject.size());
    CPPUNIT_ASSERT_EQUAL(isFailed, _paymentDetail->getYqYrDetails().isFailedRule(yqYrId));
    CPPUNIT_ASSERT_EQUAL(_paymentDetail->getYqYrDetails()._data[yqYrId]._taxPointLoc2,
                         _paymentDetail->getYqYrDetails()._data[yqYrId]._taxPointEnd);
    if (!isFailed)
    {
      CPPUNIT_ASSERT(_paymentDetail->getYqYrDetails()._data[yqYrId]._taxPointLoc2);
      CPPUNIT_ASSERT_EQUAL(yqYrEndId,
                           _paymentDetail->getYqYrDetails()._data[yqYrId]._taxPointEnd->id());
    }
  }

  void setOpen(int id1, int id2)
  {
    _properties[id1].isOpen = true;
    _properties[id2].isOpen = true;
  }

  void setFareBreak(int id1, int id2)
  {
    _properties[id1].isFareBreak = true;
    _properties[id2].isFareBreak = true;
  }

  void setStopover(int id1, int id2)
  {
    _properties[id1].isTimeStopover = true;
    _properties[id2].isTimeStopover = true;
  }

  void setUnticketed(int id1, int id2)
  {
    _geoPath->geos()[id1].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[id2].unticketedTransfer() = type::UnticketedTransfer::Yes;
  }

  // Stopovers
  void testStopover_Departure_apply()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setStopover(3, 4);
    verifyDeparture(0, true, false, 3);
  }

  void testStopover_Departure_applyOpen()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setOpen(3, 4);
    verifyDeparture(0, true, false, 3);
  }

  void testStopover_Arrival_apply()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setStopover(3, 4);
    verifyArrival(7, true, false, 4);
  }

  void testStopover_Arrival_applyOpen()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setOpen(3, 4);
    verifyArrival(7, true, false, 4);
  }

  // FareBreaks
  void testFareBreak_Departure_applyInDestination()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    verifyDeparture(0, true, false, 7);
  }

  void testFareBreak_Departure_applyInMiddle()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    verifyDeparture(0, true, false, 3);
  }

  void testFareBreak_Departure_applyInMiddleOpen()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    setOpen(3, 4);
    verifyDeparture(0, true, false, 3);
  }

  void testFareBreak_Departure_applyInMiddleTicketedOnly()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setFareBreak(3, 4);
    setUnticketed(3, 4);
    setFareBreak(5, 6);
    verifyDeparture(0, true, false, 5);
  }

  void testFareBreak_Arrival_applyInDestination()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    verifyArrival(7, true, false, 0);
  }

  void testFareBreak_Arrival_applyInMiddle()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    verifyArrival(7, true, false, 4);
  }

  void testFareBreak_Arrival_applyInMiddleOpen()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    setOpen(3, 4);
    verifyArrival(7, true, false, 4);
  }

  void testFareBreak_Arrival_applyInMiddleTicketedOnly()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setFareBreak(3, 4);
    setFareBreak(5, 6);
    setUnticketed(5, 6);
    verifyArrival(7, true, false, 4);
  }

  // Fare Break with TaxMatchingTag = 06 (fare break must also be stopover)
  void testFareBreak_Departure_applyInMiddle_TaxMatchingAppl06_noStop()
  {
    *_taxMatchingApplTag = "06";
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    verifyDeparture(0, false, true, 1);
  }

  void testFareBreak_Departure_applyInMiddle_TaxMatchingAppl06_isStop()
  {
    *_taxMatchingApplTag = "06";
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    setStopover(3, 4);
    verifyDeparture(0, true, false, 3);
  }

  void testFareBreak_Arrival_applyInMiddle_TaxMatchingAppl06_noStop()
  {
    *_taxMatchingApplTag = "06";
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    verifyArrival(7, false, true, 6);
  }

  void testFareBreak_Arrival_applyInMiddle_TaxMatchingAppl06_isStop()
  {
    *_taxMatchingApplTag = "06";
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    setStopover(3, 4);
    verifyArrival(7, true, false, 4);
  }

  // Furthest
  void testFurthest_Departure_apply()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    setFareBreak(5, 6);

    const std::vector<MileageService::GeoIdMile>& result1{{3, 3000},
                                                          {4, 3000},
                                                          {1, 2500},
                                                          {2, 2500},
                                                          {5, 2000},
                                                          {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 5, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyDeparture(0, true, false, 3);

    const std::vector<MileageService::GeoIdMile>& result2{{5, 1500},
                                                          {4, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 4, 5, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyDeparture(4, true, false, 5);

    const std::vector<MileageService::GeoIdMile>& result3{{7, 1500},
                                                          {6, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 6, 7, _)).Times(2).WillRepeatedly(ReturnRef(result3));
    verifyDeparture(6, true, false, 7);
  }

  void testFurthest_Departure_applyTicketedOnly()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setFareBreak(5, 6);
    setUnticketed(3, 4);
    const std::vector<MileageService::GeoIdMile>& result1{{3, 3000},
                                                          {4, 3000},
                                                          {5, 2000},
                                                          {1, 1000},
                                                          {2, 1000},
                                                          {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 5, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyDeparture(0, true, false, 5);
  }

  void testFurthest_Departure_TaxMatchingAppl03_OneWay()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "03";
    _roundTripOrOpenJaw = false;

    const std::vector<MileageService::GeoIdMile>& result1{{3, 3000},
                                                          {4, 3000},
                                                          {1, 2500},
                                                          {2, 2500},
                                                          {5, 2000},
                                                          {6, 2000},
                                                          {7, 1000},
                                                          {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 7, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyDeparture(0, true, false, 3);


    const std::vector<MileageService::GeoIdMile>& result2{{5, 1500},
                                                          {6, 1500},
                                                          {7, 1000},
                                                          {4, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 4, 7, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyDeparture(4, true, false, 5);


    const std::vector<MileageService::GeoIdMile>& result3{{7, 1500},
                                                          {6, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 6, 7, _)).Times(2).WillRepeatedly(ReturnRef(result3));
    verifyDeparture(6, true, false, 7);
  }

  void testFurthest_Departure_TaxMatchingAppl03_RoundInbound()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "03";
    _roundTripOrOpenJaw = true;
    // Turnaround in 3/4
    _properties[3].setIsSurface(true);
    _properties[4].setIsSurface(true);
    const std::vector<MileageService::GeoIdMile>& turn{{3, 3000},
                                                       {4, 3000},
                                                       {5, 2500},
                                                       {6, 2500},
                                                       {1, 2000},
                                                       {2, 2000},
                                                       {7, 0},
                                                       {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 7, _)).Times(1).WillRepeatedly(ReturnRef(turn));

    const std::vector<MileageService::GeoIdMile>& result1{{7, 1000},
                                                          {6, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 6, 7, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyDeparture(6, true, false, 7);

    const std::vector<MileageService::GeoIdMile>& result2{{5, 1000},
                                                          {6, 1000},
                                                          {7, 500},
                                                          {4, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 4, 7, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyDeparture(4, true, false, 5);
  }

  void testFurthest_Departure_TaxMatchingAppl03_RoundOutbound()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "03";
    _roundTripOrOpenJaw = true;
    // Turnaround in 5/6
    _properties[5].setIsSurface(true);
    _properties[6].setIsSurface(true);
    const std::vector<MileageService::GeoIdMile>& turn{{5, 3000},
                                                       {6, 3000},
                                                       {3, 2500},
                                                       {4, 2500},
                                                       {1, 2000},
                                                       {2, 2000},
                                                       {7, 0},
                                                       {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 7, _)).Times(1).WillRepeatedly(ReturnRef(turn));

    const std::vector<MileageService::GeoIdMile>& result1{{3, 3000},
                                                          {4, 3000},
                                                          {5, 2500},
                                                          {1, 2000},
                                                          {2, 2000},
                                                          {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 5, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyDeparture(0, true, false, 3);

    const std::vector<MileageService::GeoIdMile>& result2{{3, 1000},
                                                          {4, 1000},
                                                          {5, 500},
                                                          {2, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 2, 5, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyDeparture(2, true, false, 3);
  }

  void testFurthest_Departure_TaxMatchingAppl05_withoutSurface()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "05";
    _taxName->taxCode() = "OY";
    _taxName->taxPointTag() = type::TaxPointTag::Departure;
    _geoPath->geos()[0].loc().nation() = "DE";
    _geoPath->geos()[1].loc().nation() = "IE";
    _geoPath->geos()[2].loc().nation() = "IE";
    _geoPath->geos()[3].loc().nation() = "DE";
    _geoPath->geos()[4].loc().nation() = "DE";
    _geoPath->geos()[5].loc().nation() = "MY";
    _geoPath->geos()[6].loc().nation() = "MY";
    _geoPath->geos()[7].loc().nation() = "TH";
    _properties[3].isTimeStopover = false;
    _properties[4].isTimeStopover = false;
    _roundTripOrOpenJaw = false;

    // e.g. MUC-DUB-x/FRA-KUL-BKK - should select KUL as furthest
    const std::vector<MileageService::GeoIdMile>& result1{{5, 5000},
                                                          {6, 5000},
                                                          {7, 4000},
                                                          {1, 1000},
                                                          {2, 1000},
                                                          {3, 200},
                                                          {4, 200},
                                                          {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 7, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyDeparture(0, true, false, 5);

    // e.g. MUC-DUB-FRA-KUL-BKK - should select DUB as furthest before domestic stop (FRA)
    _properties[3].isTimeStopover = true;
    _properties[4].isTimeStopover = true;
    const std::vector<MileageService::GeoIdMile>& result2{{1, 1000},
                                                          {2, 1000},
                                                          {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 2, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyDeparture(0, true, false, 1);
  }

  void testFurthest_Departure_TaxMatchingAppl05_withSurface()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "05";
    _taxName->taxCode() = "OY";
    _taxName->taxPointTag() = type::TaxPointTag::Departure;
    _geoPath->geos()[0].loc().nation() = "DE";
    _geoPath->geos()[1].loc().nation() = "IE";
    _geoPath->geos()[2].loc().nation() = "IE";
    _geoPath->geos()[3].loc().nation() = "DE";
    _geoPath->geos()[4].loc().nation() = "DE";
    _geoPath->geos()[5].loc().nation() = "MY";
    _geoPath->geos()[6].loc().nation() = "SG";
    _geoPath->geos()[7].loc().nation() = "TH";
    _properties[5].setIsSurface(true);
    _properties[6].setIsSurface(true);
    _roundTripOrOpenJaw = false;

    // e.g. MUC-DUB-x/FRA-KUL//SIN-BKK - should select KUL as furthest (excl SIN as surf dest)
    const std::vector<MileageService::GeoIdMile>& result1{{6, 5500},
                                                          {5, 5000},
                                                          {7, 4000},
                                                          {1, 1000},
                                                          {2, 1000},
                                                          {3, 200},
                                                          {4, 200},
                                                          {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 7, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyDeparture(0, true, false, 5);

    // e.g. MUC-DUB//SNN-FRA-KUL-BKK - should select DUB as furthest (excl SNN as surf dest)
    const std::vector<MileageService::GeoIdMile>& result2{{2, 1100},
                                                          {1, 1000},
                                                          {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 2, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    _properties[1].setIsSurface(true);
    _properties[2].setIsSurface(true);
    _properties[3].isTimeStopover = true;
    _properties[4].isTimeStopover = true;
    verifyDeparture(0, true, false, 1);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_no48stops()
  {
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL

    // No >48h stops, should not match
    verifyDeparture(2, false, true, 3);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd()
  {
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL

    // Stop >48h in GHB, should match loc2 to GHB
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    verifyDeparture(2, true, false, 7);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_48stopMiddle()
  {
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL

    // Stop >48h in NAS, should match loc2 to GHB
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    _itin->flightUsages()[3].markDepartureDate(_itin->flightUsages()[3].departureDate().advance(2));
    verifyDeparture(2, true, false, 7);

    // Stop >48h in MHH, should match loc2 to GHB
    _itin->flightUsages()[2].markDepartureDate(_itin->flightUsages()[2].departureDate().advance(2));
    verifyDeparture(2, true, false, 7);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_48stopBegin()
  {
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL

    // Stop >48h in FPO, should match loc2 to GHB
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    _itin->flightUsages()[3].markDepartureDate(_itin->flightUsages()[3].departureDate().advance(2));
    _itin->flightUsages()[2].markDepartureDate(_itin->flightUsages()[2].departureDate().advance(2));
    _itin->flightUsages()[1].markDepartureDate(_itin->flightUsages()[1].departureDate().advance(2));
    verifyDeparture(2, true, false, 7);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_48stopBeforeBegin()
  {
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL

    // Stop >48h in FPO, Loc1=MHH, should not match
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    _itin->flightUsages()[3].markDepartureDate(_itin->flightUsages()[3].departureDate().advance(2));
    _itin->flightUsages()[2].markDepartureDate(_itin->flightUsages()[2].departureDate().advance(2));
    _itin->flightUsages()[1].markDepartureDate(_itin->flightUsages()[1].departureDate().advance(2));
    verifyDeparture(4, false, true, 5);

    // Stop >48h in FPO and in NAS, Loc1=MHH, should match loc2 to GHB
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    _itin->flightUsages()[3].markDepartureDate(_itin->flightUsages()[3].departureDate().advance(2));
    verifyDeparture(4, true, false, 7);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd_surfaceIntDom()
  {
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL
    _geoPath->geos()[5].loc().code() = "AUA";
    _geoPath->geos()[5].loc().nation() = "AW";
    // Now itin: PHL-x/FPO-x/MHH-AUA//NAS-x/GHB-FLL

    // Stop >48h in GHB, but behind AUA - should not match
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    verifyDeparture(2, false, true, 3);

    // Stop >48h in MHH, should match MHH
    _itin->flightUsages()[3].markDepartureDate(_itin->flightUsages()[3].departureDate().advance(2));
    _itin->flightUsages()[2].markDepartureDate(_itin->flightUsages()[2].departureDate().advance(2));
    verifyDeparture(2, true, false, 3);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd_surfaceDomInt()
  {
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL
    _geoPath->geos()[6].loc().code() = "AUA";
    _geoPath->geos()[6].loc().nation() = "AW";
    // Now itin: PHL-x/FPO-x/MHH-NAS//AUA-x/GHB-FLL

    // Stop >48h in GHB, but behind AUA - should not match
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    verifyDeparture(2, false, true, 3);

    // Stop >48h in MHH, should match NAS
    _itin->flightUsages()[3].markDepartureDate(_itin->flightUsages()[3].departureDate().advance(2));
    _itin->flightUsages()[2].markDepartureDate(_itin->flightUsages()[2].departureDate().advance(2));
    verifyDeparture(2, true, false, 5);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd_ticketedOnly()
  {
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL

    // Stop >48h in MHH
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    _itin->flightUsages()[3].markDepartureDate(_itin->flightUsages()[3].departureDate().advance(2));
    _itin->flightUsages()[2].markDepartureDate(_itin->flightUsages()[2].departureDate().advance(2));

    // Make GHB unticketed, should match to NAS
    _geoPath->geos()[7].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[8].unticketedTransfer() = type::UnticketedTransfer::Yes;
    verifyDeparture(2, true, false, 5);

    // Now make NAS unticketed, should match to GHB
    _geoPath->geos()[5].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[6].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[7].unticketedTransfer() = type::UnticketedTransfer::No;
    _geoPath->geos()[8].unticketedTransfer() = type::UnticketedTransfer::No;
    verifyDeparture(2, true, false, 7);

    // Put AUA instead of NAS, unticketed so should still match to GHB
    _geoPath->geos()[5].loc().code() = _geoPath->geos()[6].loc().code() = "AUA";
    _geoPath->geos()[5].loc().nation() = _geoPath->geos()[6].loc().nation() = "AW";
    verifyDeparture(2, true, false, 7);
  }

  void testFurthest_Departure_TaxMatchingAppl07_apply_48stopEnd_matchTickAndUntick()
  {
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedAndUnticketedPoints;
    commonSetupForTag07Tests(); // Itin: PHL-x/FPO-x/MHH-x/NAS-x/GHB-FLL

    // Stop >48h in MHH
    _itin->flightUsages()[4].markDepartureDate(_itin->flightUsages()[4].departureDate().advance(2));
    _itin->flightUsages()[3].markDepartureDate(_itin->flightUsages()[3].departureDate().advance(2));
    _itin->flightUsages()[2].markDepartureDate(_itin->flightUsages()[2].departureDate().advance(2));

    // Make GHB unticketed, should match to GHB
    _geoPath->geos()[7].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[8].unticketedTransfer() = type::UnticketedTransfer::Yes;
    verifyDeparture(2, true, false, 7);

    // Now make NAS unticketed, should still match to GHB
    _geoPath->geos()[5].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[6].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[7].unticketedTransfer() = type::UnticketedTransfer::No;
    _geoPath->geos()[8].unticketedTransfer() = type::UnticketedTransfer::No;
    verifyDeparture(2, true, false, 7);

    // Put AUA instead of NAS, should match to MHH
    _geoPath->geos()[5].loc().code() = _geoPath->geos()[6].loc().code() = "AUA";
    _geoPath->geos()[5].loc().nation() = _geoPath->geos()[6].loc().nation() = "AW";
    verifyDeparture(2, true, false, 3);
  }

  void testFurthest_Arrival_apply()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    setFareBreak(5, 6);

    const std::vector<MileageService::GeoIdMile>& result1{{6, 1000},
                                                          {7, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 7, 6, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyArrival(7, true, false, 6);

    const std::vector<MileageService::GeoIdMile>& result2{{1, 2500},
                                                          {2, 2500},
                                                          {3, 1500},
                                                          {4, 1500},
                                                          {0, 500},
                                                          {5, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 5, 0, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyArrival(5, true, false, 2);

    const std::vector<MileageService::GeoIdMile>& result3{{0, 4000},
                                                          {1, 2000},
                                                          {2, 2000},
                                                          {3, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 3, 0, _)).Times(2).WillRepeatedly(ReturnRef(result3));
    verifyArrival(3, true, false, 0);
  }

  void testFurthest_Arrival_applyTicketedOnly()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setFareBreak(5, 6);
    setUnticketed(1, 2);
    const std::vector<MileageService::GeoIdMile>& result1{{1, 5000},
                                                          {2, 5000},
                                                          {0, 4000},
                                                          {3, 2000},
                                                          {4, 2000},
                                                          {5, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 5, 0, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyArrival(5, true, false, 0);
  }

  void testFurthest_Arrival_TaxMatchingAppl03_OneWay()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "03";
    _roundTripOrOpenJaw = false;

    const std::vector<MileageService::GeoIdMile>& result1{{0, 4000},
                                                          {1, 2000},
                                                          {2, 2000},
                                                          {3, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 3, 0, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyArrival(3, true, false, 0);

    const std::vector<MileageService::GeoIdMile>& result2{{3, 1500},
                                                          {4, 1500},
                                                          {1, 1000},
                                                          {2, 1000},
                                                          {0, 500},
                                                          {5, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 5, 0, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyArrival(5, true, false, 4);

    const std::vector<MileageService::GeoIdMile>& result3{{5, 5000},
                                                          {6, 5000},
                                                          {3, 4000},
                                                          {4, 4000},
                                                          {1, 2000},
                                                          {2, 2000},
                                                          {0, 1000},
                                                          {7, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 7, 0, _)).Times(2).WillRepeatedly(ReturnRef(result3));
    verifyArrival(7, true, false, 6);
  }

  void testFurthest_Arrival_TaxMatchingAppl03_RoundInbound()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "03";
    _roundTripOrOpenJaw = true;
    // Turnaround in 1/2
    _properties[1].setIsSurface(true);
    _properties[2].setIsSurface(true);
    const std::vector<MileageService::GeoIdMile>& turn{{1, 3000},
                                                       {2, 3000},
                                                       {3, 2500},
                                                       {4, 2500},
                                                       {5, 2000},
                                                       {6, 2000},
                                                       {7, 0},
                                                       {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 7, _)).Times(1).WillRepeatedly(ReturnRef(turn));

    const std::vector<MileageService::GeoIdMile>& result1{{5, 3000},
                                                          {6, 3000},
                                                          {1, 2000},
                                                          {2, 2000},
                                                          {3, 1000},
                                                          {4, 1000},
                                                          {7, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 7, 1, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyArrival(7, true, false, 6);

    const std::vector<MileageService::GeoIdMile>& result2{{1, 2000},
                                                          {2, 2000},
                                                          {3, 1000},
                                                          {4, 1000},
                                                          {5, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 5, 1, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyArrival(5, true, false, 2);
  }

  void testFurthest_Arrival_TaxMatchingAppl03_RoundOutbound()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Furthest;
    *_taxMatchingApplTag = "03";
    _roundTripOrOpenJaw = true;
    // Turnaround in 3/4
    _properties[3].setIsSurface(true);
    _properties[4].setIsSurface(true);
    const std::vector<MileageService::GeoIdMile>& turn{{3, 3000},
                                                       {4, 3000},
                                                       {1, 2500},
                                                       {2, 2500},
                                                       {5, 2000},
                                                       {6, 2000},
                                                       {7, 0},
                                                       {0, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 0, 7, _)).Times(1).WillRepeatedly(ReturnRef(turn));

    const std::vector<MileageService::GeoIdMile>& result1{{1, 3000},
                                                          {2, 3000},
                                                          {0, 1000},
                                                          {3, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 3, 0, _)).Times(2).WillRepeatedly(ReturnRef(result1));
    verifyArrival(3, true, false, 2);

    const std::vector<MileageService::GeoIdMile>& result2{{0, 1000},
                                                          {1, 0}};
    EXPECT_CALL(*_mileageServiceMock, getMiles(_, _, 1, 0, _)).Times(2).WillRepeatedly(ReturnRef(result2));
    verifyArrival(1, true, false, 0);
  }

  // Stopover YqYr
  void testStopover_Departure_applyOnYqYr()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setStopover(3, 4);
    setUpPaymentDetail(0, 1);
    addYqYr(0, 3, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2DepartureRule), true, false, 3);
    verifyYqYr(0, false, 3);
  }

  void testStopover_Departure_applyOnYqYrPart()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setStopover(1, 2);
    setUpPaymentDetail(0, 1);
    addYqYr(0, 3, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2DepartureRule), true, false, 1);
    verifyYqYr(0, false, 1);
  }

  void testStopover_Departure_applyOnYqYrOpen()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setOpen(1, 2);
    setUpPaymentDetail(0, 1);
    addYqYr(0, 3, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2DepartureRule), true, false, 1);
    verifyYqYr(0, true, 0);
  }

  void testStopover_Arrival_applyOnYqYr()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setStopover(1, 2);
    setUpPaymentDetail(7, 6);
    addYqYr(7, 2, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2ArrivalRule), true, false, 2);
    verifyYqYr(0, false, 2);
  }

  void testStopover_Arrival_applyOnYqYrPart()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setStopover(1, 2);
    setUpPaymentDetail(7, 6);
    addYqYr(7, 0, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2ArrivalRule), true, false, 2);
    verifyYqYr(0, false, 2);
  }

  void testStopover_Arrival_applyOnYqYrOpen()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setOpen(1, 2);
    setUpPaymentDetail(7, 6);
    addYqYr(7, 0, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2ArrivalRule), true, false, 2);
    verifyYqYr(0, true, 0);
  }

  // FareBreaks YqYr
  void testFareBreak_Departure_applyOnYqYrs()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setUpPaymentDetail(0, 1);
    addYqYr(0, 3, "YQ", 'F', 10, false);
    addYqYr(0, 5, "YQ", 'F', 10, false);
    addYqYr(0, 7, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2DepartureRule), true, false, 7);
    verifyYqYr(0, false, 3);
    verifyYqYr(1, false, 5);
    verifyYqYr(2, false, 7);
  }

  void testFareBreak_Departure_applyOnYqYrsLimit()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(3, 4);
    setUpPaymentDetail(0, 1);
    addYqYr(0, 3, "YQ", 'F', 10, false);
    addYqYr(0, 5, "YQ", 'F', 10, false);
    addYqYr(0, 7, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2DepartureRule), true, false, 3);
    verifyYqYr(0, false, 3);
    verifyYqYr(1, false, 5);
    verifyYqYr(2, false, 7);
  }

  void testFareBreak_Departure_applyOnYqYrsLimitTicketedOnly()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setFareBreak(3, 4);
    setUnticketed(3, 4);
    setUnticketed(5, 6);
    setUpPaymentDetail(0, 1);
    addYqYr(0, 3, "YQ", 'F', 10, false);
    addYqYr(0, 5, "YQ", 'F', 10, false);
    addYqYr(0, 7, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2DepartureRule), true, false, 7);
    verifyYqYr(0, true, 3);
    verifyYqYr(1, true, 5);
    verifyYqYr(2, false, 7);
  }

  void testFareBreak_Arrival_applyOnYqYrs()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setUpPaymentDetail(5, 4);
    addYqYr(5, 4, "YQ", 'F', 10, false);
    addYqYr(5, 2, "YQ", 'F', 10, false);
    addYqYr(5, 0, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2ArrivalRule), true, false, 0);
    verifyYqYr(0, false, 4);
    verifyYqYr(1, false, 2);
    verifyYqYr(2, false, 0);
  }

  void testFareBreak_Arrival_applyOnYqYrsLimit()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setFareBreak(1, 2);
    setUpPaymentDetail(5, 4);
    addYqYr(5, 4, "YQ", 'F', 10, false);
    addYqYr(5, 2, "YQ", 'F', 10, false);
    addYqYr(5, 0, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2ArrivalRule), true, false, 2);
    verifyYqYr(0, false, 4);
    verifyYqYr(1, false, 2);
    verifyYqYr(2, false, 0);
  }

  void testFareBreak_Arrival_applyOnYqYrsLimitTicketedOnly()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setFareBreak(1, 2);
    setUnticketed(3, 4);
    setUpPaymentDetail(5, 4);
    addYqYr(5, 4, "YQ", 'F', 10, false);
    addYqYr(5, 2, "YQ", 'F', 10, false);
    addYqYr(5, 0, "YQ", 'F', 10, false);
    setUpRules();
    verify(getApplicator(*_taxPointLoc2ArrivalRule), true, false, 2);
    verifyYqYr(0, true, 4);
    verifyYqYr(1, false, 2);
    verifyYqYr(2, false, 0);
  }

  // OCs
  void testStopover_Departure_applyOnOC()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setUpPaymentDetail(0, 1);
    addOc(type::OptionalServiceTag::FlightRelated, 0, 3);
    addOc(type::OptionalServiceTag::BaggageCharge, 0, 5);
    setUpRules();
    verifyOc(getApplicator(*_taxPointLoc2DepartureRule), true, false, false, 7);
  }

  void testStopover_Arrival_applyOnOC()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::Stopover;
    setUpPaymentDetail(5, 4);
    addOc(type::OptionalServiceTag::FlightRelated, 5, 2);
    addOc(type::OptionalServiceTag::BaggageCharge, 5, 4);
    setUpRules();
    verifyOc(getApplicator(*_taxPointLoc2ArrivalRule), true, false, false, 0);
  }

  void testFareBreak_Departure_applyOnOC()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    setUpPaymentDetail(0, 1);
    addOc(type::OptionalServiceTag::FlightRelated, 0, 3);
    addOc(type::OptionalServiceTag::BaggageCharge, 0, 5);
    setUpRules();
    verifyOc(getApplicator(*_taxPointLoc2DepartureRule), true, false, false, 7);
  }

  void testFareBreak_Departure_applyOnOCTicketedOnly()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setUpPaymentDetail(0, 1);
    setUnticketed(3, 4);
    setUnticketed(4, 5);
    addOc(type::OptionalServiceTag::FlightRelated, 0, 3);
    addOc(type::OptionalServiceTag::BaggageCharge, 0, 5);
    setUpRules();
    verifyOc(getApplicator(*_taxPointLoc2DepartureRule), true, false, true, 7);
  }

  void testFareBreak_Departure_applyIgnoredOC()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setUpPaymentDetail(0, 1);
    setUnticketed(3, 4);
    addOc(type::OptionalServiceTag::TicketRelated, 0, 3);
    setUpRules();
    verifyOc(getApplicator(*_taxPointLoc2DepartureRule), true, false, false, 7);
  }

  void testFareBreak_Arrival_applyOnOC()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setUpPaymentDetail(5, 4);
    addOc(type::OptionalServiceTag::FlightRelated, 5, 2);
    addOc(type::OptionalServiceTag::BaggageCharge, 5, 4);
    setUpRules();
    verifyOc(getApplicator(*_taxPointLoc2ArrivalRule), true, false, false, 0);
  }

  void testFareBreak_Arrival_applyOnOCTicketedOnly()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setUpPaymentDetail(5, 4);
    setUnticketed(1, 2);
    setUnticketed(3, 4);
    addOc(type::OptionalServiceTag::FlightRelated, 5, 2);
    addOc(type::OptionalServiceTag::BaggageCharge, 5, 4);
    setUpRules();
    verifyOc(getApplicator(*_taxPointLoc2ArrivalRule), true, false, true, 0);
  }

  void testFareBreak_Arrival_applyIgnoredOC()
  {
    *_loc2StopoverTag = type::Loc2StopoverTag::FareBreak;
    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    setUpPaymentDetail(5, 4);
    setUnticketed(1, 2);
    addOc(type::OptionalServiceTag::TicketRelated, 5, 2);
    setUpRules();
    verifyOc(getApplicator(*_taxPointLoc2ArrivalRule), true, false, false, 0);
  }

private:
  std::unique_ptr<TaxPointLoc2StopoverTagRule> _taxPointLoc2ArrivalRule;
  std::unique_ptr<TaxPointLoc2StopoverTagRule> _taxPointLoc2DepartureRule;
  TaxName* _taxName;

  std::unique_ptr<type::Loc2StopoverTag> _loc2StopoverTag;
  std::unique_ptr<type::TaxPointTag> _taxPointTagArrival;
  std::unique_ptr<type::TaxPointTag> _taxPointTagDeparture;
  std::unique_ptr<type::TaxMatchingApplTag> _taxMatchingApplTag;
  std::unique_ptr<type::TaxProcessingApplTag> _taxProcessingApplTag;
  std::unique_ptr<LocZone> _taxPointLoc1Zone;
  std::unique_ptr<LocZone> _taxPointLoc2Zone;
  std::unique_ptr<TaxableUnitTagSet> _taxableUnits;

  StrictMock<MileageServiceMock>* _mileageServiceMock;
  std::unique_ptr<ServicesMock> _servicesMock;

  std::unique_ptr<Itin> _itin;
  std::unique_ptr<type::SeqNo> _seqNo;
  std::unique_ptr<type::TicketedPointTag> _ticketedPointTag;
  std::unique_ptr<PaymentDetail> _paymentDetail;
  GeoPath* _geoPath;
  std::vector<Flight> _flights;
  std::vector<FlightUsage> _flightUsages;
  std::vector<TaxPointProperties> _properties;

  bool _roundTripOrOpenJaw = false;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc2StopoverTagApplicatorTest);
} // namespace tax
