#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "Rules/RuleConst.h"
#include "DataModel/PricingUnit.h"
#include "test/testdata/TestPricingUnitFactory.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TSIInfo.h"
#include "test/include/TestMemHandle.h"
#include "Rules/MinimumStayApplication.h"
#include "DBAccess/MinStayRestriction.h"

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<GeoRuleItem*>& getGeoRuleItem(const VendorCode& vendor, int itemNumber)
  {
    std::vector<GeoRuleItem*>& ret = *_memHandle.create<std::vector<GeoRuleItem*> >();
    ret.push_back(_memHandle.create<GeoRuleItem>());
    if (itemNumber == 4)
    {
      ret.front()->tsi() = 5;
      return ret;
    }
    return DataHandleMock::getGeoRuleItem(vendor, itemNumber);
  }
  const TSIInfo* getTSI(int key)
  {
    if (key == 5)
    {
      TSIInfo* ret = _memHandle.create<TSIInfo>();
      ret->tsi() = key;
      ret->geoRequired() = ' ';
      ret->geoNotType() = ' ';
      ret->geoOut() = ' ';
      ret->geoItinPart() = ' ';
      ret->geoCheck() = 'D';
      ret->loopDirection() = 'B';
      ret->loopOffset() = 0;
      ret->loopToSet() = 1;
      ret->loopMatch() = 'F';
      ret->scope() = 'A';
      ret->type() = 'O';
      // ret->matchCriteria().push_back((TSIInfo::TSIMatchCriteria) 'S');
      return ret;
    }
    return NULL;
  }
};
}

class MinimumStayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MinimumStayTest);
  CPPUNIT_TEST(testFareMarket);
  CPPUNIT_TEST(testPricingUnit);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void testFareMarket()
  {
    try
    {
      // Step 1. Create Trx and Itin
      //
      PricingTrx trx;

      trx.setOptions(_memHandle.create<PricingOptions>());

      Itin itin;
      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();
      std::vector<TravelSeg*>& itinTravelSegs = itin.travelSeg();

      trx.diagnostic().diagnosticType() = Diagnostic306;
      trx.diagnostic().activate();

      DateTime testDate(2004, tse::Aug, 10, 22, 15, 34);

      // Step 2. Create Rule data
      //
      MinStayRestriction minStayRule;
      minStayRule.minStayDate() = neg_infin;
      minStayRule.geoTblItemNoFrom() = 1234;
      minStayRule.geoTblItemNoTo() = 5678;
      minStayRule.minStay() = "TUE";
      minStayRule.minStayUnit() = "02";

      // Step3 . Create Locations
      Loc loc0;
      loc0.loc() = "DFW";
      Loc loc1;
      loc1.loc() = "LGA";

      MinimumStayApplication minStayApp;

      // Step 4. Create Travel Segs
      //

      // Step 5. Create Fare Market
      //
      FareMarket fm1;
      fm1.origin() = (&loc0);
      fm1.destination() = (&loc1);
      fm1.geoTravelType() = GeoTravelType::Domestic;
      fm1.direction() = FMDirection::OUTBOUND;

      // Step 6. Create Air Travel Segs
      //
      AirSeg airSeg0;
      DateTime date1(2004, tse::Aug, 25);
      DateTime departureDate(date1, boost::posix_time::hours(11) + boost::posix_time::minutes(15));
      airSeg0.departureDT() = departureDate;
      airSeg0.origin() = &loc0;
      airSeg0.destination() = &loc1;
      airSeg0.segmentOrder() = 0;

      // Step 7. Associate Travel Segs to Itin and FareMarket
      //
      itinTravelSegs.push_back(&airSeg0);
      fm1.travelSeg().push_back(&airSeg0);

      // Step 8. Create Fare
      //
      Fare fare1;
      FareInfo fareInfo;
      fareInfo._market1 = "DFW";
      fareInfo._market2 = "LGA";
      fareInfo._directionality = FROM;
      fare1.initialize(Fare::FS_Domestic, &fareInfo, fm1);

      // Step 9. Create PaxType Fare
      //
      PaxTypeFare paxTypeFare1;
      paxTypeFare1.setFare(&fare1);
      paxTypeFare1.fareMarket() = &fm1;

      // Associate fareMarket to itin
      //
      fareMarkets.push_back(&fm1);

      Record3ReturnTypes retVal = minStayApp.validate(trx, itin, paxTypeFare1, &minStayRule, fm1);

      // Step 5. Create Fare Market
      //
      FareMarket fm2;
      fm2.origin() = (&loc1);
      fm2.destination() = (&loc0);
      fm2.geoTravelType() = GeoTravelType::Domestic;
      // fm2.direction() = FMDirection::INBOUND;

      // Step 6. Create Air Travel Segs
      //
      AirSeg airSeg1;
      DateTime date2(2004, tse::Aug, 10);
      DateTime departureDate2(date2, boost::posix_time::hours(11) + boost::posix_time::minutes(15));
      airSeg1.departureDT() = departureDate2;
      airSeg1.origin() = &loc1;
      airSeg1.destination() = &loc0;
      airSeg0.segmentOrder() = 1;

      // Step 7. Associate Travel Segs to Itin and FareMarket
      //
      itinTravelSegs.push_back(&airSeg1);
      fm2.travelSeg().push_back(&airSeg1);

      // Step 8. Create Fare
      //
      Fare fare2;
      FareInfo fareInfo2;
      fareInfo2._market1 = "LGA";
      fareInfo2._market2 = "DFW";
      fareInfo2._directionality = TO;
      fare2.initialize(Fare::FS_Domestic, &fareInfo2, fm2);

      // Step 9. Create PaxType Fare
      //
      PaxTypeFare paxTypeFare2;
      paxTypeFare2.setFare(&fare2);
      paxTypeFare2.fareMarket() = &fm2;

      fareMarkets.push_back(&fm2);

      retVal = minStayApp.validate(trx, itin, paxTypeFare2, &minStayRule, fm2);

      CPPUNIT_ASSERT(retVal == PASS);
    }
    catch (std::exception& ex) {}
  }

  void testPricingUnit()
  {
    try
    {
      // Step 1. Create Trx and Itin
      //
      PricingTrx trx;
      trx.setOptions(_memHandle.create<PricingOptions>());
      trx.diagnostic().diagnosticType() = Diagnostic306;
      trx.diagnostic().activate();

      DateTime emptyDate;
      // Step 2. Create Rule data
      //
      MinStayRestriction minStayRule;
      minStayRule.vendor() = "ATP";
      minStayRule.itemNo() = 43481;
      minStayRule.minStayDate() = emptyDate;
      minStayRule.geoTblItemNoFrom() = 4;
      minStayRule.geoTblItemNoTo() = 3;
      minStayRule.minStay() = "001";
      minStayRule.minStayUnit() = "D";

      PricingUnit* pricingUnit = TestPricingUnitFactory::create(
          "/vobs/atseintl/Rules/test/data/MinimumStayPricingUnit.xml");

      const FarePath farePath;
      const FareUsage* fareUsage = pricingUnit->fareUsage()[0];

      MinimumStayApplication minStayApp;

      Record3ReturnTypes retVal =
          minStayApp.validate(trx, &minStayRule, farePath, *pricingUnit, *fareUsage);

      CPPUNIT_ASSERT(retVal == PASS);
    }
    catch (std::exception& ex) {}
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MinimumStayTest);
}
