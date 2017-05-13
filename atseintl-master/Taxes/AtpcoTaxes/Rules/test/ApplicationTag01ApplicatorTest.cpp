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

#include "Rules/ApplicationTag01Applicator.h"
#include "Rules/ApplicationTag01Rule.h"
#include "Rules/GeoUtils.h"
#include "TestServer/Facades/LocServiceServer.h"
#include "test/GeoPathMock.h"
#include "test/LocServiceMock.h"
#include "test/MileageServiceMock.h"
#include "test/PaymentDetailMock.h"

#include <memory>

using namespace std;

namespace tax
{

class ApplicationTag01ApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ApplicationTag01ApplicatorTest);

  CPPUNIT_TEST(testPOSinUS);
  CPPUNIT_TEST(testItin_OneEndInUS);
  CPPUNIT_TEST(testItin_BothEndsInBorderZone);
  CPPUNIT_TEST(testNoStopover);

  CPPUNIT_TEST(testOW_DistanceValidation_FirstStop);
  CPPUNIT_TEST(testOW_DistanceValidation_LastStop);
  //CPPUNIT_TEST(testOW_IataSegments); // TODO: Needs update in MileageServiceMock
  CPPUNIT_TEST(testOJ_DifferentOrigDest);
  CPPUNIT_TEST(testOJ_SameOrigDest_GroundSeg);
  CPPUNIT_TEST(testOJ_SameOrigDest_SurfaceSeg);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetail = new PaymentDetailMock();
    _paymentDetail->getMutableTaxPointsProperties();
    _request.reset(new Request);
    _locService = new LocServiceMock();
    _mileageServiceMock.reset(new MileageServiceMock);
    _taxPointBegin.reset(new Geo);
    _taxPointEnd.reset(new Geo);
    _rule.reset(new ApplicationTag01Rule(_vendor));
    _paymentDetail->getMutableTaxPointsProperties().resize(8);
  }

  void tearDown()
  {
    delete _paymentDetail;
    delete _locService;
  }

  void createPointOfSale(const type::AirportCode& loc)
  {
    _request->pointsOfSale().push_back(PointOfSale());
    _request->pointsOfSale().back().loc() = loc;
    _request->allItins().back().pointOfSale() = &_request->pointsOfSale().back();
  }

  void testPOSinUS()
  {
    createRequest();
    createPointOfSale("DFW");

    type::Nation nation = "US";
    _locService->setNation(nation);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == false);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == false);
  }

  void testItin_OneEndInUS()
  {
    createRequest();
    createPointOfSale("KRK");

    type::Nation nation = "PL";
    _locService->setNation(nation);

    tax::GeoPath& geoPath = _request->geoPaths()[0];
    geoPath.geos().front().loc().cityCode().fromString("XXX", 3);
    geoPath.geos().front().loc().nation().fromString("PL", 2);
    geoPath.geos().back().loc().cityCode().fromString("YYY", 3);
    geoPath.geos().back().loc().nation().fromString("US", 2);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == false);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == false);
  }

  void testItin_BothEndsInBorderZone()
  {
    createRequest();
    createPointOfSale("KRK");

    type::Nation nation = "PL";
    _locService->setNation(nation);
    _locService->add(true, 2);

    tax::GeoPath& geoPath = _request->geoPaths()[0];
    geoPath.geos().front().loc().cityCode().fromString("TIJ", 3);
    geoPath.geos().front().loc().nation().fromString("MX", 2);
    geoPath.geos().back().loc().cityCode().fromString("YYY", 3);
    geoPath.geos().back().loc().nation().fromString("CA", 2);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == false);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == false);
  }

  void testNoStopover()
  {
    createRequest();
    createPointOfSale("KRK");

    type::Nation nation = "PL";
    _locService->setNation(nation);
    _locService->add(true);
    _locService->add(false);

    tax::GeoPath& geoPath = _request->geoPaths()[0];
    geoPath.geos().front().loc().cityCode().fromString("TIJ", 3);
    geoPath.geos().front().loc().nation().fromString("MX", 2);
    geoPath.geos().back().loc().cityCode().fromString("YYY", 3);
    geoPath.geos().back().loc().nation().fromString("PL", 2);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == false);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == false);
  }

  void FeedTestData()
  {
    createPointOfSale("LON");

    type::Nation nation = "GB";
    _locService->setNation(nation);
    _locService->add(false, 2);

    tax::GeoPath& geoPath = _request->geoPaths()[0];
    geoPath.geos()[0].loc().cityCode().fromString("TIJ", 3);
    geoPath.geos()[0].loc().nation().fromString("MX", 2);

    geoPath.geos()[1].loc().cityCode().fromString("MEX", 3);
    geoPath.geos()[1].loc().nation().fromString("MX", 2);
    geoPath.geos()[2].loc().cityCode().fromString("MEX", 3);
    geoPath.geos()[2].loc().nation().fromString("MX", 2);

    geoPath.geos()[3].loc().cityCode().fromString("DFW", 3);
    geoPath.geos()[3].loc().nation().fromString("US", 2);
    geoPath.geos()[4].loc().cityCode().fromString("DFW", 3);
    geoPath.geos()[4].loc().nation().fromString("US", 2);

    geoPath.geos()[5].loc().cityCode().fromString("MIA", 3);
    geoPath.geos()[5].loc().nation().fromString("US", 2);
    geoPath.geos()[6].loc().cityCode().fromString("MIA", 3);
    geoPath.geos()[6].loc().nation().fromString("US", 2);

    geoPath.geos()[7].loc().cityCode().fromString("YYY", 3);
    geoPath.geos()[7].loc().nation().fromString("PL", 2);
  }

  void testOW_DistanceValidation_FirstStop()
  {
    createRequest();
    FeedTestData();

    _paymentDetail->getMutableTaxPointsProperties()[3].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[5].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[6].isUSTimeStopover = true;
    _mileageServiceMock->setDistance(0, 7, 5000);
    _mileageServiceMock->setDistance(0, 3, 1000);
    _mileageServiceMock->setDistance(0, 4, 1000);
    _mileageServiceMock->setDistance(5, 7, 6000);
    _mileageServiceMock->setDistance(6, 7, 6000);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == true);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == false);
  }

  void testOW_DistanceValidation_LastStop()
  {
    createRequest();
    FeedTestData();

    _paymentDetail->getMutableTaxPointsProperties()[3].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[5].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[6].isUSTimeStopover = true;
    _mileageServiceMock->setDistance(0, 7, 5000);
    _mileageServiceMock->setDistance(0, 3, 6000);
    _mileageServiceMock->setDistance(0, 4, 6000);
    _mileageServiceMock->setDistance(5, 7, 1000);
    _mileageServiceMock->setDistance(6, 7, 1000);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == true);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == false);
  }

  void testOW_IataSegments()
  {
    //TODO OW Area1/2/3 Check
    createRequest();
    FeedTestData();
    //_geoPathMock->setOriginNation("PL");
    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == false);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == false);
  }

  void testOJ_DifferentOrigDest()
  {
    createRequest();
    FeedTestData();

    _paymentDetail->getMutableTaxPointsProperties()[5].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[6].isUSTimeStopover = true;
    _mileageServiceMock->setDistance(0, 7, 900);
    _mileageServiceMock->setDistance(0, 5, 1000);
    _mileageServiceMock->setDistance(0, 6, 1000);
    _mileageServiceMock->setDistance(5, 7, 1000);
    _mileageServiceMock->setDistance(6, 7, 1000);
    _locService->add(false, 16);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);

    CPPUNIT_ASSERT((applicator.apply(*_paymentDetail)));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == true);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == true);
  }

  void testOJ_SameOrigDest_GroundSeg()
  {
    createRequest();
    FeedTestData();
    tax::GeoPath& geoPath = _request->geoPaths()[0];
    geoPath.geos()[0].loc().nation().fromString("RU", 2);
    geoPath.geos()[7].loc().nation().fromString("XU", 2);
    const_cast<Flight*>(_request->allItins()[0].flightUsages()[1].flight())->equipment() = "BUS";

    _paymentDetail->getMutableTaxPointsProperties()[5].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[6].isUSTimeStopover = true;
    _mileageServiceMock->setDistance(0, 7, 900);
    _mileageServiceMock->setDistance(0, 5, 1000);
    _mileageServiceMock->setDistance(0, 6, 1000);
    _mileageServiceMock->setDistance(5, 7, 1000);
    _mileageServiceMock->setDistance(6, 7, 1000);
    _locService->add(false, 16);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);

    CPPUNIT_ASSERT((applicator.apply(*_paymentDetail)));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == true);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == true);
  }

  void testOJ_SameOrigDest_SurfaceSeg()
  {
    createRequest();
    FeedTestData();
    tax::GeoPath& geoPath = _request->geoPaths()[0];
    geoPath.geos()[0].loc().nation().fromString("RU", 2);
    geoPath.geos()[3].loc().cityCode().fromString("ACA", 3);
    geoPath.geos()[3].loc().nation().fromString("MX", 2);
    geoPath.geos()[7].loc().nation().fromString("XU", 2);

    _paymentDetail->getMutableTaxPointsProperties()[3].setIsSurface(true);
    _paymentDetail->getMutableTaxPointsProperties()[4].setIsSurface(true);
    _paymentDetail->getMutableTaxPointsProperties()[5].isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[6].isUSTimeStopover = true;
    _mileageServiceMock->setDistance(0, 7, 900);
    _mileageServiceMock->setDistance(0, 5, 1000);
    _mileageServiceMock->setDistance(0, 6, 1000);
    _mileageServiceMock->setDistance(5, 7, 1000);
    _mileageServiceMock->setDistance(6, 7, 1000);
    _locService->add(false, 16);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);

    CPPUNIT_ASSERT((applicator.apply(*_paymentDetail)));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == true);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == true);
  }

  void testDefaultOutput()
  {
    createRequest();
    FeedTestData();
    Flight* f = new Flight();

    FlightUsage& fu =_request->allItins()[0].flightUsages()[0];
    fu.flight() = f;

    TaxPointProperties tpp,tpp2;
    tpp.isUSTimeStopover = true;
    tpp2.isUSTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties().push_back(tpp);
    _paymentDetail->getMutableTaxPointsProperties().push_back(tpp2);
    //_mileageServiceMock->pushIndex(3).pushIndex(2).pushIndex(1).pushIndex(4);

    ApplicationTag01Applicator applicator(*_rule, _request->allItins()[0], *_locService, *_mileageServiceMock, _ticketingDate);

    CPPUNIT_ASSERT((applicator.apply(*_paymentDetail)));
    CPPUNIT_ASSERT(_paymentDetail->specUS_RTOJLogic() == true);
    CPPUNIT_ASSERT(_paymentDetail->roundTripOrOpenJaw() == true);
    delete f;
  }

private:

  void createRequest()
  {
    //create geoPaths, geos
    const uint32_t geosNum = 8;
    _request->geoPaths().resize(1);
    for (uint32_t j = 0; j < geosNum; j++)
    {
      _request->geoPaths()[0].geos().push_back(Geo());
      _request->geoPaths()[0].geos().back().id() = j;
      _request->geoPaths()[0].geos().back().loc().tag() = (j % 2 == 0)
          ? type::TaxPointTag::Departure
          : type::TaxPointTag::Arrival;
    }

    //create flights
    const uint32_t flightsNum = 4;
    _request->flights().resize(flightsNum);

    //create itins
    type::Index flightIndex = 0;

    _request->allItins().push_back(Itin());
    _request->itins().push_back(&_request->allItins().back());
    Itin& itin = _request->allItins()[0];

    itin.id() = 0;
    itin.pointOfSaleRefId() = 0;

    //create flightUsages
    itin.flightUsages().clear();
    itin.travelOriginDate() = type::Date(2013, 7, 16);
    itin.geoPath() = &_request->geoPaths().back();
    const uint32_t flightUsageNum = 4;
    for (uint32_t i = 0; i < flightUsageNum; i++)
    {
      itin.flightUsages().push_back(FlightUsage());
      Flight* f = new Flight();
      f->equipment() = std::string("787");
      itin.flightUsages().back().flight() = f;
      incrementIndex(flightsNum, flightIndex);
    }
  }

  void incrementIndex(const type::Index& maxValue, type::Index& index)
  {
    index = (index < maxValue - 1) ? (index + 1) : 0;
  }

  PaymentDetailMock* _paymentDetail;
  LocServiceMock* _locService;
  std::unique_ptr<ApplicationTag01Rule> _rule;
  std::unique_ptr<MileageServiceMock> _mileageServiceMock;
  std::unique_ptr<Geo> _taxPointBegin;
  std::unique_ptr<Geo> _taxPointEnd;
  std::unique_ptr<Request> _request;
  type::Timestamp _ticketingDate;
  type::Vendor _vendor;

};

CPPUNIT_TEST_SUITE_REGISTRATION(ApplicationTag01ApplicatorTest);
} // namespace tax
