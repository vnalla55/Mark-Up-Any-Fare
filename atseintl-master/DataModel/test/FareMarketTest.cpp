//-------------------------------------------------------------------
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Common/DateTime.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/CarrierPreference.h"
#include "DataModel/AirSeg.h"
#include "DataModel/SurfaceSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DBAccess/PaxTypeInfo.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/RexPricingTrx.h"
#include "test/include/TestConfigInitializer.h"

#include "test/testdata/TestLocFactory.h"

using namespace std;

namespace tse
{
class FareMarketTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareMarketTest);
  CPPUNIT_TEST(testOperaterEqualEqualEmpty);
  CPPUNIT_TEST(testOperaterEqualEqual);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualOrigin);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualDestination);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualBoardMultiCity);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualOffMultiCity);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualGlobalDirection);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualGoverningCarrier);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualTravelSeg);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualSideTripTravelSeg);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualPaxTypeBucket);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualMarriedSegs);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualFlowMarket);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualAvailBreaks);
  CPPUNIT_TEST(testOperaterEqualEqualNotEqualTravelDate);
  CPPUNIT_TEST(testClone);
  CPPUNIT_TEST(testSetCat19PaxFlagsForAdult);
  CPPUNIT_TEST(testSetCat19PaxFlagsForChild);
  CPPUNIT_TEST(testSetCat19PaxFlagsForInfant);
  CPPUNIT_TEST(testFbcUsageSetterAndGetter);

  CPPUNIT_TEST(testSetCarrierInAirSegments);
  CPPUNIT_TEST(testRecreateTravelSegments);

  CPPUNIT_TEST(testIsApplicableForPbb);
  CPPUNIT_TEST(testIsApplicableForPbbDiffBc);
  CPPUNIT_TEST(testIsApplicableForPbbAllEmpty);
  CPPUNIT_TEST(testIsApplicableForPbbOneEmpty);
  CPPUNIT_TEST(testHasBrandCode);
  CPPUNIT_TEST(testHasBrandCodeAllBcEmpty);
  CPPUNIT_TEST(testGetBrandCode);
  CPPUNIT_TEST(testIsApplicableForPbbFail);

  CPPUNIT_TEST(testStatusForBrand);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memH.create<TestConfigInitializer>(); }
  void tearDown() { _memH.clear(); }

  void testOperaterEqualEqualEmpty()
  {
    FareMarket fm1;
    FareMarket fm2;
    CPPUNIT_ASSERT(fm1 == fm2);
  }

  void testOperaterEqualEqual()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    CPPUNIT_ASSERT(fm1 == fm2);
  }

  void testOperaterEqualEqualNotEqualOrigin()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualDestination()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualBoardMultiCity()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.boardMultiCity() = "JFK";
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualOffMultiCity()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.offMultiCity() = "JFK";
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualGlobalDirection()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.offMultiCity() = GlobalDirection::AF;
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualGoverningCarrier()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.governingCarrier() = "BA";
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualTravelSeg()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    AirSeg airSeg;
    fm2.travelSeg().push_back(&airSeg);
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualSideTripTravelSeg()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.sideTripTravelSeg().clear();
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualPaxTypeBucket()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.paxTypeCortege().clear();
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualMarriedSegs()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.setHasAllMarriedSegs(false);
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualFlowMarket()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.setFlowMarket(false);
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualAvailBreaks()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.availBreaks().clear();
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testOperaterEqualEqualNotEqualTravelDate()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    fm2.travelDate() = DateTime(2009, 3, 13);
    CPPUNIT_ASSERT(!(fm1 == fm2));
  }

  void testClone()
  {
    FareMarket fm1, fm2;
    setupEqualFareMarkets(fm1, fm2);
    FareMarket fm3;
    fm2.clone(fm3);
    CPPUNIT_ASSERT(fm3 == fm2);
  }

  void testSetCat19PaxFlagsForAdult()
  {
    FareMarket fm;
    PaxType paxType;
    PaxTypeInfo paxTypeInfo;
    paxType.paxTypeInfo() = &paxTypeInfo;
    paxTypeInfo.childInd() = 'N';
    paxTypeInfo.infantInd() = 'N';
    paxTypeInfo.initPsgType();
    fm.setCat19PaxFlags(&paxType);
    CPPUNIT_ASSERT(!(fm.isChildNeeded()));
    CPPUNIT_ASSERT(!(fm.isInfantNeeded()));
  }

  void testSetCat19PaxFlagsForChild()
  {
    FareMarket fm;
    PaxType paxType;
    PaxTypeInfo paxTypeInfo;
    paxType.paxTypeInfo() = &paxTypeInfo;
    paxTypeInfo.childInd() = 'Y';
    paxTypeInfo.infantInd() = 'N';
    paxTypeInfo.initPsgType();
    fm.setCat19PaxFlags(&paxType);
    CPPUNIT_ASSERT(fm.isChildNeeded());
    CPPUNIT_ASSERT(!(fm.isInfantNeeded()));
  }

  void testSetCat19PaxFlagsForInfant()
  {
    FareMarket fm;
    PaxType paxType;
    PaxTypeInfo paxTypeInfo;
    paxType.paxTypeInfo() = &paxTypeInfo;
    paxTypeInfo.childInd() = 'N';
    paxTypeInfo.infantInd() = 'Y';
    paxTypeInfo.initPsgType();
    fm.setCat19PaxFlags(&paxType);
    CPPUNIT_ASSERT(!(fm.isChildNeeded()));
    CPPUNIT_ASSERT(fm.isInfantNeeded());
  }

  void testFbcUsageSetterAndGetter()
  {
    FareMarket fm;
    fm.fbcUsage() = FILTER_FBC;
    CPPUNIT_ASSERT_EQUAL(FILTER_FBC, fm.fbcUsage());
  }

  void setupEqualFareMarkets(FareMarket& fm1, FareMarket& fm2)
  {
    fm1.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    fm2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    fm1.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");
    fm2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");
    fm1.boardMultiCity() = "LON";
    fm2.boardMultiCity() = "LON";
    fm1.offMultiCity() = "BOS";
    fm2.offMultiCity() = "BOS";
    fm1.setGlobalDirection(GlobalDirection::EU);
    fm2.setGlobalDirection(GlobalDirection::EU);
    fm1.governingCarrier() = "AA";
    fm2.governingCarrier() = "AA";
    fm1.governingCarrierPref() = &_cp;
    fm2.governingCarrierPref() = &_cp;
    fm1.travelSeg().push_back(&_airSeg);
    fm2.travelSeg().push_back(&_airSeg);
    fm1.sideTripTravelSeg().push_back(fm1.travelSeg());
    fm2.sideTripTravelSeg().push_back(fm2.travelSeg());
    fm1.paxTypeCortege().push_back(_paxTypeCortege);
    fm2.paxTypeCortege().push_back(_paxTypeCortege);
    fm1.setHasAllMarriedSegs(true);
    fm2.setHasAllMarriedSegs(true);
    fm1.setFlowMarket(true);
    fm2.setFlowMarket(true);
    fm1.availBreaks().push_back(true);
    fm2.availBreaks().push_back(true);
    fm1.travelDate() = DateTime(2009, 3, 12);
    fm2.travelDate() = DateTime(2009, 3, 12);
  }

  AirSeg* populateTravelSegments(FareMarket& fm)
  {
    fm.travelSeg().push_back(_memH(new SurfaceSeg));
    fm.travelSeg().push_back(_memH(new ArunkSeg));
    AirSeg* as = _memH(new AirSeg);
    fm.travelSeg().push_back(as);
    return as;
  }

  void testSetCarrierInAirSegments()
  {
    FareMarket fm;
    AirSeg& as = *populateTravelSegments(fm);
    as.setMarketingCarrierCode("CM");
    as.setOperatingCarrierCode("CM");

    CarrierCode expect = "AA";
    fm.setCarrierInAirSegments(expect);

    CPPUNIT_ASSERT_EQUAL(expect, as.marketingCarrierCode());
    CPPUNIT_ASSERT_EQUAL(expect, as.operatingCarrierCode());
  }

  template <typename T>
  void assertSeg(TravelSeg* expect, TravelSeg* current)
  {
    CPPUNIT_ASSERT(expect != current);
    CPPUNIT_ASSERT(dynamic_cast<T*>(expect));
    CPPUNIT_ASSERT(dynamic_cast<T*>(current));
  }

  void testRecreateTravelSegments()
  {
    FareMarket fm;
    populateTravelSegments(fm);
    std::vector<TravelSeg*> segs = fm.travelSeg();

    DataHandle dh;
    fm.recreateTravelSegments(dh);

    assertSeg<SurfaceSeg>(segs[0], fm.travelSeg()[0]);
    assertSeg<ArunkSeg>(segs[1], fm.travelSeg()[1]);
    assertSeg<AirSeg>(segs[2], fm.travelSeg()[2]);
  }

  void testGetterSetter()
  {
    FareMarket fm;
    CPPUNIT_ASSERT(!fm.rexBaseTrx());
    RexPricingTrx rexTrx;
    fm.rexBaseTrx() = &rexTrx;
    RexBaseTrx* rexBase = dynamic_cast<RexBaseTrx*>(&rexTrx);
    CPPUNIT_ASSERT_EQUAL(rexBase, fm.rexBaseTrx());
  }

  void testIsApplicableForPbb()
  {
    setUpForBrandTests();
    CPPUNIT_ASSERT(_fm.isApplicableForPbb());
  }

  void testIsApplicableForPbbDiffBc()
  {
   setUpForBrandTestsDiffBc();
   CPPUNIT_ASSERT(!_fm.isApplicableForPbb());
  }

  void testIsApplicableForPbbAllEmpty()
  {
    setUpForBrandTestsAllBcEmpty();
    CPPUNIT_ASSERT(_fm.isApplicableForPbb());
  }

  void testIsApplicableForPbbOneEmpty()
  {
    setUpForBrandTestsOneBcEmpty();
    CPPUNIT_ASSERT(!_fm.isApplicableForPbb());
  }

  void testIsApplicableForPbbFail()
  {
    try
    {
      _fm.isApplicableForPbb();
    }
    catch (const ErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("SYSTEM EXCEPTION"), e.message());
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::SYSTEM_EXCEPTION, e.code());
      return;
    }
    CPPUNIT_ASSERT(false);
  }

  void testHasBrandCode()
  {
    setUpForBrandTests();
    CPPUNIT_ASSERT(_fm.hasBrandCode());
  }

  void testHasBrandCodeAllBcEmpty()
  {
    setUpForBrandTestsAllBcEmpty();
    CPPUNIT_ASSERT(!_fm.hasBrandCode());
  }

  void testGetBrandCode()
  {
    setUpForBrandTests();
    const BrandCode& brand = _fm.getBrandCode();
    CPPUNIT_ASSERT(brand == "BC");
  }

  void setUpForBrandTests()
  {
    for (int idx=0; idx<4; ++idx)
    {
      AirSeg* seg = _memH(new AirSeg);
      seg->setBrandCode("BC");
      _fm.travelSeg().push_back(seg);
    }
  }

  void setUpForBrandTestsDiffBc()
  {
    setUpForBrandTests();
    _fm.travelSeg()[1]->setBrandCode("NC");
  }

  void setUpForBrandTestsAllBcEmpty()
  {
    for (int idx=0; idx<4; ++idx)
      _fm.travelSeg().push_back(_memH(new AirSeg));
  }

  void setUpForBrandTestsOneBcEmpty()
  {
    setUpForBrandTests();
    _fm.travelSeg()[1]->setBrandCode("");
  }

  void testStatusForBrand()
  {
    FareMarket fm;
    BrandCode brand = "BC";
    // IbfErrorMessage::IBF_EM_NO_FARE_FOUND
    // IbfErrorMessage::IBF_EM_NOT_AVAILABLE
    // IbfErrorMessage::IBF_EM_NOT_OFFERED
    // IbfErrorMessage::IBF_EM_EARLY_DROP
    // IbfErrorMessage::IBF_EM_NO_FARE_FILED
    // IbfErrorMessage::IBF_EM_NOT_SET

    fm.updateStatusForBrand(brand, Direction::ORIGINAL, IbfErrorMessage::IBF_EM_NOT_AVAILABLE);
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NOT_AVAILABLE,
                         fm.getStatusForBrand(brand, Direction::ORIGINAL));
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NO_FARE_FILED,
                         fm.getStatusForBrand(brand, Direction::REVERSED));
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NOT_AVAILABLE,
                         fm.getStatusForBrand(brand, Direction::BOTHWAYS));

    fm.updateStatusForBrand(brand, Direction::REVERSED, IbfErrorMessage::IBF_EM_NO_FARE_FOUND);
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NOT_AVAILABLE,
                         fm.getStatusForBrand(brand, Direction::ORIGINAL));
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NO_FARE_FOUND,
                         fm.getStatusForBrand(brand, Direction::REVERSED));
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NO_FARE_FOUND,
                         fm.getStatusForBrand(brand, Direction::BOTHWAYS));

    fm.forceStatusForBrand(brand, Direction::REVERSED, IbfErrorMessage::IBF_EM_NOT_OFFERED);
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NOT_AVAILABLE,
                         fm.getStatusForBrand(brand, Direction::ORIGINAL));
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NOT_OFFERED,
                         fm.getStatusForBrand(brand, Direction::REVERSED));
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NO_FARE_FOUND, // should not be updated
                         fm.getStatusForBrand(brand, Direction::BOTHWAYS));

    fm.forceStatusForBrand(brand, Direction::BOTHWAYS, IbfErrorMessage::IBF_EM_NOT_SET);
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NOT_SET,
                         fm.getStatusForBrand(brand, Direction::ORIGINAL));
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NOT_SET,
                         fm.getStatusForBrand(brand, Direction::REVERSED));
    CPPUNIT_ASSERT_EQUAL(IbfErrorMessage::IBF_EM_NOT_SET,
                         fm.getStatusForBrand(brand, Direction::BOTHWAYS));
  }

private:
  AirSeg _airSeg;
  PaxTypeBucket _paxTypeCortege;
  CarrierPreference _cp;
  TestMemHandle _memH;
  FareMarket _fm;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareMarketTest);
}
