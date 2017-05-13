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

#include "Rules/ConnectionsTagsApplicator.h"
#include "Rules/ConnectionsTagsRule.h"
#include "DataModel/Common/Types.h"
#include "Common/Timestamp.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Common/TaxName.h"
#include "test/MileageServiceMock.h"

#include <memory>
#include <set>

namespace tax
{

namespace
{

class ItinStub : public Itin
{
public:
  void setTurnaround(Geo* geo)
  {
    _turnaroundCalculated = true;
    _turnaroundPoint = geo;
  }
};
}

class ConnectionsTagsApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ConnectionsTagsApplicatorTest);

  CPPUNIT_TEST(testApplyAlways);
  CPPUNIT_TEST(testSurfaceSegmentAlwaysStopover);
  CPPUNIT_TEST(testSurfaceSegmentNotAlwaysStopover);
  CPPUNIT_TEST(testGroundTransportIsStopover);
  CPPUNIT_TEST(testDifferentMarketingCarrierIsStopover);
  CPPUNIT_TEST(testMultiairportCityAsStopover);
  CPPUNIT_TEST(testDomesticToInternationalAsStopover);
  CPPUNIT_TEST(testInternationalToDomesticAsStopover);
  CPPUNIT_TEST(testInternationalToInternationalAsStopover);
  CPPUNIT_TEST(testFareBreakAsStopover);
  CPPUNIT_TEST(testFurthestFareBreakAsStopover);
  CPPUNIT_TEST(testTurnaroundPointForConnectionAsStopover);
  CPPUNIT_TEST(testTurnaroundPointForConnectionAsStopoverNoGoodFareBreak);
  CPPUNIT_TEST(testTurnaroundPointForConnectionAsStopoverAfterOtherTag);
  CPPUNIT_TEST(testTurnaroundPointAsStopover);
  CPPUNIT_TEST(testTurnaroundPointAsStopover_noDoubleOccurence);
  CPPUNIT_TEST(testTurnaroundPointAsStopover_doubleOccurence);
  CPPUNIT_TEST(testTurnaroundPointAsStopover_sideTrip);
  CPPUNIT_TEST(testTurnaroundPointAsStopoverInternational);
  CPPUNIT_TEST(testTurnaroundPointAsStopoverInternationalNoGoodFareBreak);
  CPPUNIT_TEST(testTurnaroundPointAsStopoverDomestic);

  CPPUNIT_TEST_SUITE_END();

  TaxName* _taxName;

public:
  void prepareFlights()
  {
    std::vector<Geo>& geos = _geoPath->geos();

    _flights.clear();
    for (int flight = 0; flight < 5; flight++)
    {
      geos.push_back(Geo());
      geos.push_back(Geo());

      const type::Index geoId = flight * 2;
      geos[geoId].id() = geoId;
      geos[geoId + 1].id() = geoId + 1;

      _flightUsages.push_back(FlightUsage());
      _flights.push_back(new Flight());
      _flightUsages[flight].flight() = &_flights[flight];
    }

    _paymentDetail->getMutableTaxPointsProperties().resize(10);

    geos[0].loc().cityCode() = "KRK";
    geos[0].loc().code() = "KRK";
    geos[0].loc().nation() = "PL";
    geos[1].loc().cityCode() = "DFW";
    geos[1].loc().code() = "DFW";
    geos[1].loc().nation() = "US";
    geos[2].loc().cityCode() = "DFW";
    geos[2].loc().code() = "DAL";
    geos[2].loc().nation() = "US";
    geos[3].loc().cityCode() = "FRA";
    geos[3].loc().code() = "FRA";
    geos[3].loc().nation() = "DE";
    geos[4].loc().cityCode() = "FRA";
    geos[4].loc().code() = "FRA";
    geos[4].loc().nation() = "DE";
    geos[5].loc().cityCode() = "SFO";
    geos[5].loc().code() = "SFO";
    geos[5].loc().nation() = "US";
    geos[6].loc().code() = "SFO";
    geos[6].loc().cityCode() = "SFO";
    geos[6].loc().nation() = "US";
    geos[7].loc().cityCode() = "LON";
    geos[7].loc().code() = "LHR";
    geos[7].loc().nation() = "GB";
    geos[8].loc().cityCode() = "LON";
    geos[8].loc().code() = "LHR";
    geos[8].loc().nation() = "GB";
    geos[9].loc().cityCode() = "KRK";
    geos[9].loc().code() = "KRK";
    geos[9].loc().nation() = "PL";

    geos[0].loc().tag() = type::TaxPointTag::Departure;
    geos[1].loc().tag() = type::TaxPointTag::Arrival;
    geos[2].loc().tag() = type::TaxPointTag::Departure;
    geos[3].loc().tag() = type::TaxPointTag::Arrival;
    geos[4].loc().tag() = type::TaxPointTag::Departure;
    geos[5].loc().tag() = type::TaxPointTag::Arrival;
    geos[6].loc().tag() = type::TaxPointTag::Departure;
    geos[7].loc().tag() = type::TaxPointTag::Arrival;
    geos[8].loc().tag() = type::TaxPointTag::Departure;
    geos[9].loc().tag() = type::TaxPointTag::Arrival;

    geos[0].setNext(&geos[1]);
    geos[1].setPrev(&geos[0]);
    geos[1].setNext(&geos[2]);
    geos[2].setPrev(&geos[1]);
    geos[2].setNext(&geos[3]);
    geos[3].setPrev(&geos[2]);
    geos[3].setNext(&geos[4]);
    geos[4].setPrev(&geos[3]);
    geos[4].setNext(&geos[5]);
    geos[5].setPrev(&geos[4]);
    geos[5].setNext(&geos[6]);
    geos[6].setPrev(&geos[5]);
    geos[6].setNext(&geos[7]);
    geos[7].setPrev(&geos[6]);
    geos[7].setNext(&geos[8]);
    geos[8].setPrev(&geos[7]);
    geos[8].setNext(&geos[9]);
    geos[9].setPrev(&geos[8]);
    geos[9].makeLast();
  }

  std::shared_ptr<ConnectionsTagsApplicator> createApplicator()
  {
    _connectionsTagsRule.reset(new ConnectionsTagsRule(_connectionsTags, false, false));
    return std::shared_ptr<ConnectionsTagsApplicator>(new ConnectionsTagsApplicator(
        *_connectionsTagsRule, *_itin, type::Timestamp(), *_mileageServiceMock));
  }

  void setUp()
  {
    const Geo geo1 = Geo();
    Geo geo2 = Geo();
    _taxName = new TaxName();
    _taxName->taxPointTag() = type::TaxPointTag::Departure;
    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(type::SeqNo(),
                                          type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                                          noUnits,
                                          0,
                                          type::CurrencyCode(UninitializedCode),
                                          type::TaxAppliesToTagInd::Blank),
                          geo1,
                          geo2,
                          *_taxName));
    _geoPath.reset(new GeoPath);
    _geoPath->id() = 0;
    _mileageServiceMock.reset(new MileageServiceMock);

    _flightUsages.clear();
    prepareFlights();

    _itin.reset(new ItinStub());
    _itin->geoPath() = _geoPath.get();
    _itin->flightUsages() = _flightUsages;

    _connectionsTags.clear();
  }

  void tearDown()
  {
    delete _taxName;
  }

  void testApplyAlways()
  {
    CPPUNIT_ASSERT_EQUAL(true, createApplicator()->apply(*_paymentDetail));
  }

  void testGroundTransportIsStopover()
  {
    _connectionsTags.insert(type::ConnectionsTag::GroundTransport);

    _flights[0].equipment() = "301";
    _flights[1].equipment() = "BUS";
    _flights[2].equipment() = "101";

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testDifferentMarketingCarrierIsStopover()
  {
    _connectionsTags.insert(type::ConnectionsTag::DifferentMarketingCarrier);

    _flights[0].marketingCarrier() = "LH";
    _flights[1].marketingCarrier() = "LH";
    _flights[2].marketingCarrier() = "AA";

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testSurfaceSegmentAlwaysStopover()
  {
    std::vector<Geo>& geos = _geoPath->geos();
    geos[2].loc().cityCode() = "DAL";
    _paymentDetail->getMutableTaxPointsProperties()[1].setIsSurface(true);
    _paymentDetail->getMutableTaxPointsProperties()[2].setIsSurface(true);
    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(0).isSurfaceStopover());
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(1).isSurfaceStopover());
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(2).isSurfaceStopover());
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isSurfaceStopover());
  }

  void testSurfaceSegmentNotAlwaysStopover()
  {
    _connectionsTagsRule.reset(new ConnectionsTagsRule(_connectionsTags, false, true));

    std::vector<Geo>& geos = _geoPath->geos();
    geos[2].loc().cityCode() = "DAL";
    _paymentDetail->getMutableTaxPointsProperties()[1].setIsSurface(true);
    _paymentDetail->getMutableTaxPointsProperties()[2].setIsSurface(true);
    ConnectionsTagsApplicator(*_connectionsTagsRule, *_itin, type::Timestamp(),
        *_mileageServiceMock).apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(0).isSurfaceStopover());
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isSurfaceStopover());
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isSurfaceStopover());
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isSurfaceStopover());
  }

  void testMultiairportCityAsStopover()
  {
    _connectionsTags.insert(type::ConnectionsTag::Multiairport);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(0).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
  }

  void testDomesticToInternationalAsStopover()
  {
    std::vector<Geo>& geos = _geoPath->geos();
    geos[0].loc().cityCode() = "NYC";
    geos[0].loc().code() = "NYC";
    geos[0].loc().nation() = "US";

    _connectionsTags.insert(type::ConnectionsTag::DomesticToInternational);
    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(0).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
  }

  void testInternationalToDomesticAsStopover()
  {
    std::vector<Geo>& geos = _geoPath->geos();
    geos[3].loc().cityCode() = "NYC";
    geos[3].loc().code() = "NYC";
    geos[3].loc().nation() = "US";

    _connectionsTags.insert(type::ConnectionsTag::InternationalToDomestic);
    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(0).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
  }

  void testInternationalToInternationalAsStopover()
  {
    _connectionsTags.insert(type::ConnectionsTag::InternationalToInternational);
    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(0).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
  }

  void testFareBreakAsStopover()
  {
    _connectionsTags.insert(type::ConnectionsTag::FareBreak);
    _paymentDetail->getMutableTaxPointsProperties().at(1).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(2).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = false;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = false;

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testFurthestFareBreakAsStopover()
  {
    _connectionsTags.insert(type::ConnectionsTag::FurthestFareBreak);
    _mileageServiceMock->pushIndex(4).pushIndex(3).pushIndex(2).pushIndex(1);
    _paymentDetail->getMutableTaxPointsProperties().at(1).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(2).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testTurnaroundPointForConnectionAsStopover()
  {
    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPointForConnection);
    _mileageServiceMock->pushIndex(4).pushIndex(3).pushIndex(2).pushIndex(1);
    _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;
    _itin->setTurnaround(&_geoPath->geos()[3]);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testTurnaroundPointForConnectionAsStopoverNoGoodFareBreak()
  {
    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPointForConnection);
    _mileageServiceMock->pushIndex(4).pushIndex(3).pushIndex(2).pushIndex(1);
    _itin->setTurnaround(0);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testTurnaroundPointForConnectionAsStopoverAfterOtherTag()
  {
    _connectionsTags.insert(type::ConnectionsTag::Multiairport);
    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(4).isExtendedStopover);

    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPointForConnection);
    _mileageServiceMock->pushIndex(4).pushIndex(3).pushIndex(2).pushIndex(1);
    _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;
    _itin->setTurnaround(&_geoPath->geos()[3]);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testTurnaroundPointAsStopover()
  {
    _paymentDetail->roundTripOrOpenJaw() = true;
    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPoint);
    _mileageServiceMock->pushIndex(4).pushIndex(3).pushIndex(2).pushIndex(1);
    _paymentDetail->getMutableTaxPointsProperties().at(1).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(2).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;
    _itin->setTurnaround(&_geoPath->geos()[3]);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testTurnaroundPointAsStopover_noDoubleOccurence()
  {
    _paymentDetail->roundTripOrOpenJaw() = true;
    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPoint);
    _mileageServiceMock->pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);

    _geoPath->geos()[3].loc().cityCode() = "DFW";
    _geoPath->geos()[3].loc().code() = "DAL";
    _geoPath->geos()[3].loc().nation() = "US";
    _geoPath->geos()[4].loc().cityCode() = "DFW";
    _geoPath->geos()[4].loc().code() = "DAL";
    _geoPath->geos()[4].loc().nation() = "US";

    _paymentDetail->getMutableTaxPointsProperties().at(1).isFareBreak = false;
    _paymentDetail->getMutableTaxPointsProperties().at(2).isFareBreak = false;
    _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;
    _itin->setTurnaround(&_geoPath->geos()[3]);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

  void testTurnaroundPointAsStopover_doubleOccurence()
  {
    _paymentDetail->roundTripOrOpenJaw() = true;
    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPoint);
    _mileageServiceMock->pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);

    _geoPath->geos()[3].loc().cityCode() = "DFW";
    _geoPath->geos()[3].loc().code() = "DAL";
    _geoPath->geos()[3].loc().nation() = "US";
    _geoPath->geos()[4].loc().cityCode() = "DFW";
    _geoPath->geos()[4].loc().code() = "DAL";
    _geoPath->geos()[4].loc().nation() = "US";

    _paymentDetail->getMutableTaxPointsProperties().at(1).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(2).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;
    _itin->setTurnaround(&_geoPath->geos()[3]);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }
/*
 *  0 -- 1 2 -- 3 8 -- 9
 *              4 7
 *              | |
 *              5 6
 */
  void testTurnaroundPointAsStopover_sideTrip()
  {
    _paymentDetail->roundTripOrOpenJaw() = true;
    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPoint);
    _mileageServiceMock->pushIndex(1).pushIndex(2).pushIndex(5).pushIndex(6);
    _paymentDetail->getMutableTaxPointsProperties().at(0).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(1).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(2).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(5).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(6).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(7).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(9).isFareBreak = true;
    _itin->setTurnaround(&_geoPath->geos()[1]);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(5).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(6).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(7).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(8).isExtendedStopover);
  }

  void testTurnaroundPointAsStopoverInternational()
  {
    std::vector<Geo>& geos = _geoPath->geos();
    geos[3].loc().cityCode() = "WAW";
    geos[3].loc().code() = "WAW";
    geos[3].loc().nation() = "PL";
    geos[4].loc().cityCode() = "WAW";
    geos[4].loc().code() = "WAW";
    geos[4].loc().nation() = "PL";
    _paymentDetail->roundTripOrOpenJaw() = true;
    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPoint);
    _mileageServiceMock->pushIndex(4).pushIndex(3).pushIndex(6).pushIndex(5).pushIndex(8).pushIndex(7);
    _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(5).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(6).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(7).isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties().at(8).isFareBreak = true;
    _itin->setTurnaround(&_geoPath->geos()[5]);

    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT(! _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
    CPPUNIT_ASSERT(_paymentDetail->taxPointsProperties().at(5).isExtendedStopover);
    CPPUNIT_ASSERT(_paymentDetail->taxPointsProperties().at(6).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(7).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(8).isExtendedStopover);
  }

  void testTurnaroundPointAsStopoverInternationalNoGoodFareBreak()
  {
    std::vector<Geo>& geos = _geoPath->geos();
    geos[3].loc().cityCode() = "WAW";
    geos[3].loc().code() = "WAW";
    geos[3].loc().nation() = "PL";
    geos[4].loc().cityCode() = "WAW";
    geos[4].loc().code() = "WAW";
    geos[4].loc().nation() = "PL";
    _paymentDetail->roundTripOrOpenJaw() = true;
    _connectionsTags.insert(type::ConnectionsTag::TurnaroundPoint);
    _mileageServiceMock->pushIndex(4).pushIndex(3).pushIndex(6).pushIndex(5).pushIndex(8).pushIndex(7);
    _itin->setTurnaround(0);
    createApplicator()->apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
    CPPUNIT_ASSERT(! _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(5).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(6).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(7).isExtendedStopover);
    CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(8).isExtendedStopover);
  }

  void testTurnaroundPointAsStopoverDomestic()
  {
	  std::vector<Geo>& geos = _geoPath->geos();
	  geos[0].loc().cityCode() = "NYC";
	  geos[0].loc().code() = "EWR";
	  geos[0].loc().nation() = "US";
	  geos[3].loc().cityCode() = "MIA";
	  geos[3].loc().code() = "MIA";
	  geos[3].loc().nation() = "US";
	  geos[4].loc().cityCode() = "MIA";
	  geos[4].loc().code() = "MIA";
	  geos[4].loc().nation() = "US";
	  geos[7].loc().cityCode() = "WSG";
	  geos[7].loc().code() = "DCA";
	  geos[7].loc().nation() = "US";
	  geos[8].loc().cityCode() = "WSG";
	  geos[8].loc().code() = "DCA";
	  geos[8].loc().nation() = "US";
	  geos[9].loc().cityCode() = "NYC";
	  geos[9].loc().code() = "NYC";
	  geos[9].loc().nation() = "US";
	  _paymentDetail->roundTripOrOpenJaw() = true;
	  _connectionsTags.insert(type::ConnectionsTag::TurnaroundPoint);
	  _mileageServiceMock->pushIndex(4).pushIndex(3).pushIndex(2).pushIndex(1).pushIndex(6).pushIndex(5);
	  _paymentDetail->getMutableTaxPointsProperties().at(1).isFareBreak = true;
	  _paymentDetail->getMutableTaxPointsProperties().at(2).isFareBreak = true;
	  _paymentDetail->getMutableTaxPointsProperties().at(3).isFareBreak = true;
	  _paymentDetail->getMutableTaxPointsProperties().at(4).isFareBreak = true;
	  _paymentDetail->getMutableTaxPointsProperties().at(5).isFareBreak = true;
	  _paymentDetail->getMutableTaxPointsProperties().at(6).isFareBreak = true;
    _itin->setTurnaround(&_geoPath->geos()[3]);

	  createApplicator()->apply(*_paymentDetail);

	  CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(1).isExtendedStopover);
	  CPPUNIT_ASSERT(!_paymentDetail->taxPointsProperties().at(2).isExtendedStopover);
	  CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(3).isExtendedStopover);
	  CPPUNIT_ASSERT( _paymentDetail->taxPointsProperties().at(4).isExtendedStopover);
  }

private:
  std::unique_ptr<PaymentDetail> _paymentDetail;
  std::unique_ptr<ItinStub> _itin;
  std::unique_ptr<GeoPath> _geoPath;
  std::unique_ptr<MileageServiceMock> _mileageServiceMock;
  std::vector<FlightUsage> _flightUsages;
  boost::ptr_vector<Flight> _flights;

  std::unique_ptr<ConnectionsTagsRule> _connectionsTagsRule;

  std::set<type::ConnectionsTag> _connectionsTags;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConnectionsTagsApplicatorTest);
} // namespace tax
