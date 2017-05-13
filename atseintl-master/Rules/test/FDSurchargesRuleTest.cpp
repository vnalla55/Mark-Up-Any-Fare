#include "test/include/CppUnitHelperMacros.h"

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "DBAccess/DataHandle.h"
#include "Common/DateTime.h"

#include <iostream>
#include <time.h>
#include <vector>
#include <set>
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "Rules/RuleConst.h"
#include "Rules/SurchargesRule.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayInfo.h"
#include "Common/FareDisplayUtil.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/SurchargesInfo.h"
#include "DataModel/SurchargeData.h"
#include "Rules/FDSurchargesRule.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/LocKey.h"
#include "Rules/RuleUtil.h"
#include "DBAccess/CarrierPreference.h"

#include "test/testdata/TestLocFactory.h"
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>

using namespace tse;
using namespace std;

namespace tse
{
class FDSurchargesRuleTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(FDSurchargesRuleTest);

  CPPUNIT_TEST(testCheckValidDataNoOptions);
  CPPUNIT_TEST(testCheckValidDataNoSurcharges);
  CPPUNIT_TEST(testCheckValidDataNoFarePath);
  CPPUNIT_TEST(testCheckValidDataNoPricingUnit);
  CPPUNIT_TEST(testCheckValidDataNoFareUsage);
  CPPUNIT_TEST(testCheckValidData);
  CPPUNIT_TEST(testBuildInboundFareMarket);
  CPPUNIT_TEST(testUpdateFareInfoEmptySurchargeData);
  CPPUNIT_TEST(testUpdateFareInfo);

  CPPUNIT_TEST_SUITE_END();

public:
  void testCheckValidDataNoOptions()
  {
    FDSurchargesRule fdsr;
    FareDisplayTrx fdTrx;
    FarePath* fp = 0;
    PricingUnit* pu = 0;
    FareUsage* fu = 0;

    fdsr.checkValidData(fdTrx, fp, pu, fu);
  }

  void testCheckValidDataNoSurcharges()
  {
    FDSurchargesRule fdsr;
    FareDisplayTrx fdTrx;
    FarePath* fp = 0;
    PricingUnit* pu = 0;
    FareUsage* fu = 0;
    FareDisplayOptions options;
    fdTrx.setOptions(&options);

    fdsr.checkValidData(fdTrx, fp, pu, fu);
  }

  void testCheckValidDataNoFarePath()
  {
    FDSurchargesRule fdsr;
    FareDisplayTrx fdTrx;
    FarePath* fp = 0;
    PricingUnit* pu = 0;
    FareUsage* fu = 0;
    FareDisplayOptions options;
    fdTrx.setOptions(&options);
    Indicator fuel = 'F';
    options.surchargeTypes().push_back(fuel);

    fdsr.checkValidData(fdTrx, fp, pu, fu);
  }

  void testCheckValidDataNoPricingUnit()
  {
    FDSurchargesRule fdsr;
    FareDisplayTrx fdTrx;
    FarePath fp;
    PricingUnit* pu = 0;
    FareUsage* fu = 0;
    FareDisplayOptions options;
    fdTrx.setOptions(&options);
    Indicator fuel = 'F';
    options.surchargeTypes().push_back(fuel);

    fdsr.checkValidData(fdTrx, &fp, pu, fu);
  }

  void testCheckValidDataNoFareUsage()
  {
    FDSurchargesRule fdsr;
    FareDisplayTrx fdTrx;
    FarePath fp;
    PricingUnit pu;
    FareUsage* fu = 0;
    FareDisplayOptions options;
    fdTrx.setOptions(&options);
    Indicator fuel = 'F';
    options.surchargeTypes().push_back(fuel);

    fdsr.checkValidData(fdTrx, &fp, &pu, fu);
  }

  void testCheckValidData()
  {
    FDSurchargesRule fdsr;
    FareDisplayTrx fdTrx;
    FareDisplayOptions options;
    fdTrx.setOptions(&options);
    Indicator fuel = 'F';
    options.surchargeTypes().push_back(fuel);
    FarePath fp;
    PricingUnit pu;
    FareUsage fu;

    fdsr.checkValidData(fdTrx, &fp, &pu, &fu);
  }

  void testBuildInboundFareMarket()
  {
    FDSurchargesRule fdsr;
    FareMarket fm;
    FareMarket ibfm;
    CarrierPreference cp;
    CarrierPreference::dummyData(cp);

    fm.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    fm.offMultiCity() = "LC1";
    fm.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    fm.boardMultiCity() = "LC2";
    fm.setGlobalDirection(GlobalDirection::PV);
    fm.direction() = FMDirection::OUTBOUND;
    fm.geoTravelType() = GeoTravelType::Domestic;
    fm.governingCarrierPref() = &cp;
    fm.travelBoundary() = FMTravelBoundary::TravelWithinUSCA;
    fm.governingCarrier() = "BA";
    fm.travelDate() = DateTime(2009, 1, 1, 1, 0, 0);

    fdsr.buildInboundFareMarket(&fm, &ibfm);

    CPPUNIT_ASSERT_EQUAL(fm.destination(), ibfm.origin());
    CPPUNIT_ASSERT_EQUAL(fm.origin(), ibfm.destination());
    CPPUNIT_ASSERT_EQUAL(fm.offMultiCity(), ibfm.boardMultiCity());
    CPPUNIT_ASSERT_EQUAL(fm.boardMultiCity(), ibfm.offMultiCity());
    CPPUNIT_ASSERT_EQUAL(fm.getGlobalDirection(), ibfm.getGlobalDirection());
    CPPUNIT_ASSERT(FMDirection::INBOUND == ibfm.direction());
    CPPUNIT_ASSERT(fm.geoTravelType() == ibfm.geoTravelType());
    CPPUNIT_ASSERT_EQUAL(fm.governingCarrierPref(), ibfm.governingCarrierPref());
    CPPUNIT_ASSERT_EQUAL(fm.governingCarrier(), ibfm.governingCarrier());
    CPPUNIT_ASSERT_EQUAL(fm.travelDate(), ibfm.travelDate());
    CPPUNIT_ASSERT_EQUAL(fm.travelBoundary().value(), ibfm.travelBoundary().value());
  }

  void testUpdateFareInfoEmptySurchargeData()
  {
    FDSurchargesRule fdsr;
    FareDisplayTrx fdTrx;
    FareDisplayOptions options;
    fdTrx.setOptions(&options);
    FareDisplayInfo fdInfo;

    vector<SurchargeData*> emptyVector;
    vector<SurchargeData*> retSurchargeData;

    fdsr.updateFareInfo(fdTrx, retSurchargeData, emptyVector);
    CPPUNIT_ASSERT(retSurchargeData.empty());
  }

  void testUpdateFareInfo()
  {
    FDSurchargesRule fdsr;
    FareDisplayTrx fdTrx;
    FareDisplayOptions options;
    fdTrx.setOptions(&options);
    FareDisplayInfo fdInfo;

    vector<SurchargeData*> surchargeData;
    vector<SurchargeData*> retSurchargeData;

    options.surchargeTypes().push_back('A');
    options.surchargeTypes().push_back('B');
    options.surchargeTypes().push_back('A');
    options.surchargeTypes().push_back('C');
    options.surchargeTypes().push_back('D');

    SurchargeData sd1, sd2, sd3, sd4;
    sd1.surchargeType() = 'Z';
    sd2.surchargeType() = 'D';
    sd3.surchargeType() = 'A';
    sd4.surchargeType() = 'Y';

    surchargeData.push_back(&sd1);
    surchargeData.push_back(&sd2);
    surchargeData.push_back(&sd3);
    surchargeData.push_back(&sd4);

    fdsr.updateFareInfo(fdTrx, retSurchargeData, surchargeData);
    CPPUNIT_ASSERT(retSurchargeData.size() == 2);

    vector<SurchargeData*>::iterator itBegin = retSurchargeData.begin();
    vector<SurchargeData*>::iterator itEnd = retSurchargeData.end();
    vector<SurchargeData*>::iterator itResult;

    itResult = find(itBegin, itEnd, &sd1);
    CPPUNIT_ASSERT(itResult == itEnd);

    itResult = find(itBegin, itEnd, &sd2);
    CPPUNIT_ASSERT(itResult != itEnd);

    itResult = find(itBegin, itEnd, &sd3);
    CPPUNIT_ASSERT(itResult != itEnd);

    itResult = find(itBegin, itEnd, &sd4);
    CPPUNIT_ASSERT(itResult == itEnd);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FDSurchargesRuleTest);
}
