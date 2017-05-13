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

#include <boost/optional.hpp>
#include <gmock/gmock.h>

#include "Rules/JourneyLoc2DestinationTurnAroundApplicator.h"

#include "Common/Timestamp.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/MileageGetter.h"
#include "ServiceInterfaces/MileageService.h"
#include "Rules/PaymentDetail.h"
#include "Rules/JourneyLoc2DestinationTurnAroundRule.h"
#include "test/GeoPathBuilder.h"
#include "test/LocServiceMock.h"
#include "test/PaymentDetailMock.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>

using testing::_;
using testing::Return;
using testing::ReturnRef;

namespace tax
{
namespace
{

const type::AirportOrCityCode NYC("NYC"), MUN("MUN"), HAM("HAM"), PRG("PRG");

class MyLocServiceMock : public LocServiceMock
{
public:
  MOCK_CONST_METHOD3(isInLoc, bool(const type::AirportOrCityCode&, const LocZone&, const type::Vendor&));
};

class MileageServiceMock : public MileageService
{
public:
  MOCK_CONST_METHOD3(getAllMilesFromStart,
                     const std::vector<GeoIdMile>&(const GeoPath&,
                                                   const std::vector<tax::FlightUsage>&,
                                                   const type::Timestamp&));
  MOCK_CONST_METHOD3(getMileageGetter,
                     const MileageGetter&(const GeoPath&,
                                          const std::vector<FlightUsage>&,
                                          const type::Timestamp&));
};

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

class JourneyLoc2DestinationTurnAroundApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(JourneyLoc2DestinationTurnAroundApplicatorTest);

  CPPUNIT_TEST(testApplyOneWayDestinationPositive);
  CPPUNIT_TEST(testApplyOneWayDestinationOpenPositive);
  CPPUNIT_TEST(testApplyOneWayDestinationNegative);
  CPPUNIT_TEST(testApplyRoundTripDestinationPositive);
  CPPUNIT_TEST(testApplyRoundTripDestinationOpenPositive);
  CPPUNIT_TEST(testApplyRoundTripDestinationNegative);
  CPPUNIT_TEST(testApplyOpenJawDestinationPositive);
  CPPUNIT_TEST(testApplyOpenJawDestinationOpenPositive);
  CPPUNIT_TEST(testApplyOpenJawDestinationNegative);
  CPPUNIT_TEST(testApplyTurnaroundPositive);
  CPPUNIT_TEST(testApplyTurnaroundNegative);
  CPPUNIT_TEST(testApplyTurnaroundNegativeNoTurnaroundPoint);

  CPPUNIT_TEST_SUITE_END();
public:
  void setUp()
  {
    _paymentDetail.reset(new PaymentDetailMock());
    _travelDate = type::Timestamp::emptyTimestamp();
    _locService.reset(new MyLocServiceMock());
    _mileageService.reset(new MileageServiceMock());
    _itin.reset(new ItinStub());
  }

  void setUpTaxPointProperties(type::Index size)
  {
    TaxPointsProperties& taxPointProperties = _paymentDetail->getMutableTaxPointsProperties();
    taxPointProperties.resize(size);
    taxPointProperties[0].isFirst = true;
    taxPointProperties[size - 1].isLast = true;
  }

  void setUpOneWay()
  {
    _geoPath.reset(GeoPathBuilder()
                       .addGeo("KRK", "KRK", "PL", type::TaxPointTag::Departure)
                       .addGeo("MUN", "MUN", "DE", type::TaxPointTag::Arrival)
                       .addGeo("MUN", "MUN", "DE", type::TaxPointTag::Departure)
                       .addGeo("FRA", "FRA", "DE", type::TaxPointTag::Arrival)
                       .addGeo("FRA", "FRA", "DE", type::TaxPointTag::Departure)
                       .addGeo("NYC", "NYC", "US", type::TaxPointTag::Arrival)
                       .build());

    setUpTaxPointProperties(6);
    _paymentDetail->roundTripOrOpenJaw() = false;
  }

  void setUpRoundTrip()
  {
    _geoPath.reset(GeoPathBuilder()
                       .addGeo("MUN", "MUN", "DE", type::TaxPointTag::Departure)
                       .addGeo("PRG", "PRG", "CZ", type::TaxPointTag::Arrival)
                       .addGeo("PRG", "PRG", "CZ", type::TaxPointTag::Departure)
                       .addGeo("HAM", "HAM", "DE", type::TaxPointTag::Arrival)
                       .addGeo("HAM", "HAM", "DE", type::TaxPointTag::Departure)
                       .addGeo("FRA", "FRA", "DE", type::TaxPointTag::Arrival)
                       .addGeo("FRA", "FRA", "DE", type::TaxPointTag::Departure)
                       .addGeo("MUN", "MUN", "DE", type::TaxPointTag::Arrival)
                       .build());

    setUpTaxPointProperties(8);
    _paymentDetail->roundTripOrOpenJaw() = true;
    _itin->setTurnaround(&_geoPath->geos()[1]);
  }

  void setUpDomesticRoundTrip()
  {
    setUpRoundTrip();
    _geoPath->geos()[1].loc().code() = "TXL";
    _geoPath->geos()[1].loc().cityCode() = "BER";
    _geoPath->geos()[1].loc().nation() = "DE";
    _geoPath->geos()[2].loc().code() = "TXL";
    _geoPath->geos()[2].loc().cityCode() = "BER";
    _geoPath->geos()[2].loc().nation() = "DE";
    _itin->setTurnaround(&_geoPath->geos()[4]); // must overwrite turnaroundPoint with the farthest
  }

  void setUpOpenJaw()
  {
    _geoPath.reset(GeoPathBuilder()
                       .addGeo("MUN", "MUN", "DE", type::TaxPointTag::Departure)
                       .addGeo("PRG", "PRG", "CZ", type::TaxPointTag::Arrival)
                       .addGeo("PRG", "PRG", "CZ", type::TaxPointTag::Departure)
                       .addGeo("HAM", "HAM", "DE", type::TaxPointTag::Arrival)
                       .build());

    setUpTaxPointProperties(4);
    _paymentDetail->roundTripOrOpenJaw() = true;
    _itin->setTurnaround(&_geoPath->geos()[1]);
  }

  void setUpDomesticOpenJaw()
  {
    setUpOpenJaw();
    _geoPath->geos()[1].loc().code() = "TXL";
    _geoPath->geos()[1].loc().cityCode() = "BER";
    _geoPath->geos()[1].loc().nation() = "DE";
    _geoPath->geos()[2].loc().code() = "TXL";
    _geoPath->geos()[2].loc().cityCode() = "BER";
    _geoPath->geos()[2].loc().nation() = "DE";
    _itin->setTurnaround(&_geoPath->geos()[3]); // must overwrite turnaroundPoint with the farthest
  }

  void createRule()
  {
    _rule.reset(new JourneyLoc2DestinationTurnAroundRule(*_jrnyInd, _locZone, _vendor, false));
  }

  void createApplicator()
  {
    createRule();
    _itin->geoPath() = _geoPath.get();
    _applicator.reset(new JourneyLoc2DestinationTurnAroundApplicator(
        *_rule, *_itin, *_locService, *_mileageService));
  }

  void tearDown()
  {
    _applicator.reset();
    _rule.reset();
    _jrnyInd = boost::none;
    _paymentDetail.reset();
  }

  void testApplyOneWayDestinationPositive()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpOneWay();
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(NYC, _, _)).WillOnce(Return(true));
    ASSERT_TRUE(_applicator->apply(*_paymentDetail));
  }

  void testApplyOneWayDestinationOpenPositive()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpOneWay();
    _paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    _paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;
    _paymentDetail->getMutableTaxPointsProperties()[3].isOpen = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isOpen = true;
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(NYC, _, _)).WillOnce(Return(true));
    ASSERT_TRUE(_applicator->apply(*_paymentDetail));
  }

  void testApplyOneWayDestinationNegative()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpOneWay();
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(NYC, _, _)).WillOnce(Return(false));
    ASSERT_TRUE(!_applicator->apply(*_paymentDetail));
  }

  void testApplyRoundTripDestinationPositive()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpRoundTrip();
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(MUN, _, _)).WillOnce(Return(true));
    ASSERT_TRUE(_applicator->apply(*_paymentDetail));
  }

  void testApplyRoundTripDestinationOpenPositive()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpRoundTrip();
    _paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    _paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;
    _paymentDetail->getMutableTaxPointsProperties()[3].isOpen = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isOpen = true;
    _itin->setTurnaround(0);
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(MUN, _, _)).WillOnce(Return(true));
    ASSERT_TRUE(_applicator->apply(*_paymentDetail));
  }

  void testApplyRoundTripDestinationNegative()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpRoundTrip();
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(MUN, _, _)).WillOnce(Return(false));
    ASSERT_TRUE(!_applicator->apply(*_paymentDetail));
  }

  void testApplyOpenJawDestinationPositive()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpOpenJaw();
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(HAM, _, _)).WillOnce(Return(true));
    ASSERT_TRUE(_applicator->apply(*_paymentDetail));
  }

  void testApplyOpenJawDestinationOpenPositive()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpOpenJaw();
    _paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    _paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;
    _itin->setTurnaround(0);
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(HAM, _, _)).WillOnce(Return(true));
    ASSERT_TRUE(_applicator->apply(*_paymentDetail));
  }

  void testApplyOpenJawDestinationNegative()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;
    setUpOpenJaw();
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(HAM, _, _)).WillOnce(Return(false));
    ASSERT_TRUE(!_applicator->apply(*_paymentDetail));
  }

  void testApplyTurnaroundPositive()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrTurnAroundForRTOrOJ;
    setUpRoundTrip();
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(PRG, _, _)).WillOnce(Return(true));
    ASSERT_TRUE(_applicator->apply(*_paymentDetail));
  }

  void testApplyTurnaroundNegative()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrTurnAroundForRTOrOJ;
    setUpRoundTrip();
    createApplicator();

    EXPECT_CALL(*_locService, isInLoc(PRG, _, _)).WillOnce(Return(false));
    ASSERT_TRUE(!_applicator->apply(*_paymentDetail));
  }

  void testApplyTurnaroundNegativeNoTurnaroundPoint()
  {
    _jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrTurnAroundForRTOrOJ;
    setUpRoundTrip();
    _itin->setTurnaround(0);
    createApplicator();

    ASSERT_TRUE(!_applicator->apply(*_paymentDetail));
  }

private:
  std::unique_ptr<JourneyLoc2DestinationTurnAroundApplicator> _applicator;
  std::unique_ptr<JourneyLoc2DestinationTurnAroundRule> _rule;
  boost::optional<type::JrnyInd> _jrnyInd;
  // next two are used only as empty stubs
  LocZone _locZone;
  type::Vendor _vendor;
  std::unique_ptr<ItinStub> _itin;
  std::unique_ptr<GeoPath> _geoPath;
  // will always be empty in the test
  // since it is used only in MileageGetter, which we mock anyway
  boost::ptr_vector<FlightUsage> _flightUsages;
  type::Timestamp _travelDate;
  std::unique_ptr<MyLocServiceMock> _locService;
  std::unique_ptr<MileageServiceMock> _mileageService;
  std::unique_ptr<PaymentDetailMock> _paymentDetail;
};
CPPUNIT_TEST_SUITE_REGISTRATION(JourneyLoc2DestinationTurnAroundApplicatorTest);
}
