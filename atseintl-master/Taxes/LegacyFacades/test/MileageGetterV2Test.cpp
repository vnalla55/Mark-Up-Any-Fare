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
#include <gmock/gmock.h>

#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MileageSubstitution.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Flight.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/FlightUsage.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/GeoPath.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/MileageGetterV2.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"
#include "test/testdata/TestLocFactory.h"

#include <memory>

namespace tse
{

using testing::_;
using testing::DoAll;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;
using testing::SetArgReferee;
using testing::StrictMock;

class MileageGetterV2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageGetterV2Test);
  CPPUNIT_TEST(testSingleDistanceIsMaximal);
  CPPUNIT_TEST(testSingleDistanceTPM);
  CPPUNIT_TEST(testSingleDistanceTPMSubstitutionOrigin);
  CPPUNIT_TEST(testSingleDistanceTPMSubstitutionDestination);
  CPPUNIT_TEST(testSingleDistanceTPMSubstitutionBoth);
  CPPUNIT_TEST(testSingleDistanceMPM);
  CPPUNIT_TEST(testSingleDistanceMPMSubstitutionBoth);
  CPPUNIT_TEST(testSingleDistanceGCM);
  CPPUNIT_TEST(testSingleDistanceGCMSubstitutionOrigin);
  CPPUNIT_TEST(testSingleDistanceGCMSubstitutionDestination);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

  class MileageDataHandle : public DataHandleMock
  {
  public:
    MOCK_METHOD4(
        getMileage,
        const std::vector<Mileage*>&(const LocCode&, const LocCode&, const DateTime&, Indicator));

    const Loc* getLoc(const LocCode& locCode, const DateTime& date)
    {
      if (locCode == "KKK")
        return 0;
      if (locCode == "EWR")
        return TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
      if (locCode == "SFO")
        return TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");

      if (locCode == "SJC")
        return TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSJC.xml");

      return DataHandleMock::getLoc(locCode, date);
    }

    MOCK_METHOD2(getMileageSubstitution,
                 const MileageSubstitution*(const LocCode&, const DateTime&));
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();

    _trx.reset(new tse::PricingTrx());
    _request.reset(new PricingRequest());
    _trx->setRequest(_request.get());

    _mileageDataHandle.reset(new StrictMock<MileageDataHandle>());

    _geoPath.reset(new tax::GeoPath());
    _geoPath->geos().push_back(tax::Geo());
    _geoPath->geos().back().loc().code() = toTaxAirportCode(NYC);
    _geoPath->geos().back().loc().cityCode() = toTaxCityCode(NYC);
    _geoPath->geos().back().id() = 0;

    _geoPath->geos().push_back(tax::Geo());
    _geoPath->geos().back().loc().code() = toTaxAirportCode(DFW);
    _geoPath->geos().back().loc().cityCode() = toTaxCityCode(DFW);
    _geoPath->geos().back().id() = 1;
    _geoPath->geos().back().makeLast();

    _flightUsages.clear();
    _flightUsages.push_back(tax::FlightUsage());
    _flightUsages.back().flight() = new tax::Flight();
    _flightUsages.back().flight()->departureTime() = tax::type::Time(0, 0);
    _flightUsages.back().flight()->arrivalTime() = tax::type::Time(0, 0);
    _flightUsages.back().markDepartureDate(tax::type::Date(2015, 12, 12));

    _timestamp.reset(new tax::type::Timestamp(tax::type::Timestamp::emptyTimestamp()));
    _mileageGetter.reset(new MileageGetterV2(*_geoPath, _flightUsages, *_timestamp, *_trx, false));
  }

  void tearDown() { _memHandle.clear(); }

  void testSingleDistanceIsMaximal()
  {
    std::vector<Mileage*> mileages;
    addMileage(80, mileages, GlobalDirection::XX);
    addMileage(100, mileages, GlobalDirection::XX);
    addMileage(90, mileages, GlobalDirection::XX);

    EXPECT_CALL(*_mileageDataHandle, getMileage(_, _, _, _)).WillOnce(ReturnRef(mileages));

    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(100), _mileageGetter->getSingleDistance(0, 1));
  }

  void testSingleDistanceTPM()
  {
    std::vector<Mileage*> mileagesTPM;
    addMileage(10, mileagesTPM);

    EXPECT_CALL(*_mileageDataHandle, getMileage(NYC, DFW, _, TPM)).WillOnce(ReturnRef(mileagesTPM));

    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(10), _mileageGetter->getSingleDistance(0, 1));
  }

  void testSingleDistanceTPMSubstitutionOrigin()
  {
    std::vector<Mileage*> mileagesTPM, emptyMileages;
    addMileage(10, mileagesTPM);

    _geoPath->geos().front().loc().code() = toTaxAirportCode(EWR);
    _geoPath->geos().front().loc().cityCode() = toTaxCityCode(EWR);

    MileageSubstitution substitution;
    substitution.publishedLoc() = NYC;
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(EWR, _))
        .WillOnce(Return(&substitution));

    EXPECT_CALL(*_mileageDataHandle, getMileage(EWR, DFW, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(NYC, DFW, _, TPM)).WillOnce(ReturnRef(mileagesTPM));

    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(10), _mileageGetter->getSingleDistance(0, 1));
  }

  void testSingleDistanceTPMSubstitutionDestination()
  {
    std::vector<Mileage*> mileagesTPM, emptyMileages;
    addMileage(10, mileagesTPM);

    _geoPath->geos().front().loc().code() = toTaxAirportCode(DFW);
    _geoPath->geos().front().loc().cityCode() = toTaxCityCode(DFW);
    _geoPath->geos().back().loc().code() = toTaxAirportCode(EWR);
    _geoPath->geos().back().loc().cityCode() = toTaxCityCode(EWR);

    MileageSubstitution substitution;
    substitution.publishedLoc() = NYC;
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(DFW, _))
        .WillOnce(Return((MileageSubstitution*)0));
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(EWR, _))
        .WillOnce(Return(&substitution));

    EXPECT_CALL(*_mileageDataHandle, getMileage(DFW, EWR, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(DFW, NYC, _, TPM)).WillOnce(ReturnRef(mileagesTPM));

    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(10), _mileageGetter->getSingleDistance(0, 1));
  }

  void testSingleDistanceTPMSubstitutionBoth()
  {
    std::vector<Mileage*> mileagesTPM, emptyMileages;
    addMileage(10, mileagesTPM);

    _geoPath->geos().front().loc().code() = toTaxAirportCode(SJC);
    _geoPath->geos().front().loc().cityCode() = toTaxCityCode(SJC);
    _geoPath->geos().back().loc().code() = toTaxAirportCode(EWR);
    _geoPath->geos().back().loc().cityCode() = toTaxCityCode(EWR);

    MileageSubstitution substitutionEWR;
    substitutionEWR.publishedLoc() = NYC;
    MileageSubstitution substitutionSJC;
    substitutionSJC.publishedLoc() = SFO;

    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(SJC, _))
        .WillOnce(Return(&substitutionSJC));
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(EWR, _))
        .WillOnce(Return(&substitutionEWR));

    EXPECT_CALL(*_mileageDataHandle, getMileage(SJC, EWR, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SFO, EWR, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SJC, NYC, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SFO, NYC, _, TPM)).WillOnce(ReturnRef(mileagesTPM));

    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(10), _mileageGetter->getSingleDistance(0, 1));
  }

  void testSingleDistanceMPM()
  {
    std::vector<Mileage*> mileagesTPM, mileagesMPM;
    addMileage(12, mileagesMPM);

    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(NYC, _))
        .WillOnce(Return((MileageSubstitution*)0));
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(DFW, _))
        .WillOnce(Return((MileageSubstitution*)0));
    EXPECT_CALL(*_mileageDataHandle, getMileage(_, _, _, TPM)).WillOnce(ReturnRef(mileagesTPM));
    EXPECT_CALL(*_mileageDataHandle, getMileage(_, _, _, MPM)).WillOnce(ReturnRef(mileagesMPM));

    // MPM Mileage is divided by 1.2
    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(10), _mileageGetter->getSingleDistance(0, 1));
  }

  void testSingleDistanceMPMSubstitutionBoth()
  {
    std::vector<Mileage*> mileagesMPM, emptyMileages;
    addMileage(12, mileagesMPM);

    _geoPath->geos().front().loc().code() = toTaxAirportCode(SJC);
    _geoPath->geos().front().loc().cityCode() = toTaxCityCode(SJC);
    _geoPath->geos().back().loc().code() = toTaxAirportCode(EWR);
    _geoPath->geos().back().loc().cityCode() = toTaxCityCode(EWR);

    MileageSubstitution substitutionEWR;
    substitutionEWR.publishedLoc() = NYC;
    MileageSubstitution substitutionSJC;
    substitutionSJC.publishedLoc() = SFO;

    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(SJC, _))
        .WillRepeatedly(Return(&substitutionSJC));
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(EWR, _))
        .WillRepeatedly(Return(&substitutionEWR));

    EXPECT_CALL(*_mileageDataHandle, getMileage(SJC, EWR, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SFO, EWR, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SJC, NYC, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SFO, NYC, _, TPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SJC, EWR, _, MPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SFO, EWR, _, MPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SJC, NYC, _, MPM))
        .WillOnce(ReturnRef(emptyMileages));
    EXPECT_CALL(*_mileageDataHandle, getMileage(SFO, NYC, _, MPM)).WillOnce(ReturnRef(mileagesMPM));

    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(10), _mileageGetter->getSingleDistance(0, 1));
  }

  void testSingleDistanceGCM()
  {
    std::vector<Mileage*> mileagesTPM, mileagesMPM;

    MileageSubstitution* noSubstitution = 0;

    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(NYC, _))
        .WillRepeatedly(Return(noSubstitution));
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(DFW, _))
        .WillRepeatedly(Return(noSubstitution));
    EXPECT_CALL(*_mileageDataHandle, getMileage(NYC, DFW, _, TPM)).WillOnce(ReturnRef(mileagesTPM));
    EXPECT_CALL(*_mileageDataHandle, getMileage(NYC, DFW, _, MPM)).WillOnce(ReturnRef(mileagesMPM));

    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(1386), _mileageGetter->getSingleDistance(0, 1));
  }

  void testSingleDistanceGCMSubstitutionOrigin()
  {
    _geoPath->geos().front().loc().code() = toTaxAirportCode("KKK");
    _geoPath->geos().front().loc().cityCode() = toTaxCityCode("KKK");

    testSingleDistanceGCMSubstitution();
  }

  void testSingleDistanceGCMSubstitutionDestination()
  {
    _geoPath->geos().front().loc().code() = toTaxAirportCode(DFW);
    _geoPath->geos().front().loc().cityCode() = toTaxCityCode(DFW);
    _geoPath->geos().back().loc().code() = toTaxAirportCode("KKK");
    _geoPath->geos().back().loc().cityCode() = toTaxCityCode("KKK");

    testSingleDistanceGCMSubstitution();
  }

private:
  TestMemHandle _memHandle;
  std::unique_ptr<StrictMock<MileageDataHandle>> _mileageDataHandle;
  std::unique_ptr<PricingTrx> _trx;
  std::unique_ptr<PricingRequest> _request;
  std::unique_ptr<tax::GeoPath> _geoPath;
  std::vector<tax::FlightUsage> _flightUsages;
  std::unique_ptr<MileageGetterV2> _mileageGetter;
  std::unique_ptr<tax::type::Timestamp> _timestamp;

  static const LocCode NYC;
  static const LocCode EWR;
  static const LocCode DFW;
  static const LocCode SJC;
  static const LocCode SFO;

  void addMileage(int miles,
                  std::vector<Mileage*>& mileages,
                  GlobalDirection globalDir = GlobalDirection::AP)
  {
    mileages.push_back(_memHandle.create<Mileage>());
    mileages.back()->mileage() = miles;
    mileages.back()->globaldir() = globalDir;
  }

  void testSingleDistanceGCMSubstitution()
  {
    std::vector<Mileage*> mileagesTPM, mileagesMPM;

    MileageSubstitution* noSubstitution = 0;
    MileageSubstitution substitutionKKK;
    substitutionKKK.publishedLoc() = NYC;

    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(LocCode("KKK"), _))
        .WillRepeatedly(Return(&substitutionKKK));
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(NYC, _))
        .WillRepeatedly(Return(noSubstitution));
    EXPECT_CALL(*_mileageDataHandle, getMileageSubstitution(DFW, _))
        .WillRepeatedly(Return(noSubstitution));
    EXPECT_CALL(*_mileageDataHandle, getMileage(_, _, _, TPM))
        .WillRepeatedly(ReturnRef(mileagesTPM));
    EXPECT_CALL(*_mileageDataHandle, getMileage(_, _, _, MPM))
        .WillRepeatedly(ReturnRef(mileagesMPM));

    CPPUNIT_ASSERT_EQUAL(tax::type::Miles(1386), _mileageGetter->getSingleDistance(0, 1));
  }
};

const LocCode MileageGetterV2Test::NYC = "NYC";
const LocCode MileageGetterV2Test::EWR = "EWR";
const LocCode MileageGetterV2Test::DFW = "DFW";
const LocCode MileageGetterV2Test::SJC = "SJC";
const LocCode MileageGetterV2Test::SFO = "SFO";

CPPUNIT_TEST_SUITE_REGISTRATION(MileageGetterV2Test);

} // namespace tse
