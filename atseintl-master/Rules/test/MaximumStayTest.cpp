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
#include "test/include/TestMemHandle.h"
#include "Rules/MaximumStayApplication.h"
#include "DBAccess/MinStayRestriction.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/TSIInfo.h"
#include "DBAccess/ATPResNationZones.h"
#include "Fares/FareController.h"

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
    if (itemNumber == 123)
    {
      ret.front()->tsi() = 6;
      return ret;
    }
    else if (itemNumber == 3430)
    {
      ret.front()->tsi() = 52;
      return ret;
    }
    return DataHandleMock::getGeoRuleItem(vendor, itemNumber);
  }
  const TSIInfo* getTSI(int key)
  {
    if (key == 6 || key == 52)
    {
      TSIInfo* ret = _memHandle.create<TSIInfo>();
      ret->tsi() = key;
      ret->geoRequired() = ' ';
      ret->geoNotType() = ' ';
      ret->geoOut() = ' ';
      ret->geoItinPart() = ' ';
      ret->geoCheck() = 'O';
      ret->loopOffset() = 0;
      ret->loopToSet() = 0;
      ret->scope() = 'A';
      ret->type() = 'O';
      if (key == 6)
      {
        ret->loopDirection() = 'F';
        ret->loopMatch() = 'S';
        ret->matchCriteria().push_back((TSIInfo::TSIMatchCriteria)'R');
      }
      else
      {
        ret->loopDirection() = 'B';
        ret->loopMatch() = 'O';
      }
      return ret;
    }
    return DataHandleMock::getTSI(key);
  }
  const std::vector<ATPResNationZones*>& getATPResNationZones(const NationCode& key)
  {
    std::vector<ATPResNationZones*>& ret = *_memHandle.create<std::vector<ATPResNationZones*> >();
    ret.push_back(_memHandle.create<ATPResNationZones>());
    if (key == "BR")
    {
      ret.front()->zones().push_back("0000170");
      ret.front()->zones().push_back("0000171");
      return ret;
    }
    else if (key == "FR")
    {
      ret.front()->zones().push_back("0000210");
      return ret;
    }
    else if (key == "ES")
    {
      ret.front()->zones().push_back("0000210");
      ret.front()->zones().push_back("0000211");
      return ret;
    }

    return DataHandleMock::getATPResNationZones(key);
  }
};

struct FareMarketDataA : public RuleControllerDataAccess
{
  FareMarketDataA(PricingTrx& transaction, Itin* itinerary, PaxTypeFare& ptFare)
    : _trx(transaction), _itin(itinerary), _paxTypeFare(ptFare)
  {
  }

  Itin* itin() { return _itin; }
  PaxTypeFare& paxTypeFare() const { return getBaseOrPaxTypeFare(_paxTypeFare); }
  PricingTrx& trx() { return _trx; }

protected:
  PricingTrx& _trx;
  Itin* _itin;
  PaxTypeFare& _paxTypeFare;
};
}
class MaximumStayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MaximumStayTest);
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

      DateTime emptyDate;
      // Step 2. Create Rule data
      //
      MaxStayRestriction maxStayRule;
      maxStayRule.maxStayDate() = emptyDate;
      maxStayRule.geoTblItemNoFrom() = 3;
      maxStayRule.geoTblItemNoTo() = 4;
      maxStayRule.maxStay() = "000";
      maxStayRule.maxStayUnit() = "D";
      maxStayRule.returnTrvInd() = 'C';
      maxStayRule.tod() = -1;

      // Step3 . Create Locations
      Loc loc0;
      loc0.loc() = "DFW";
      Loc loc1;
      loc1.loc() = "LGA";

      MaximumStayApplication maxStayApp;

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

      FareMarketDataA fmDA(trx, &itin, paxTypeFare1);
      maxStayApp.setRuleDataAccess(&fmDA);

      Record3ReturnTypes retVal = maxStayApp.validate(trx, itin, paxTypeFare1, &maxStayRule, fm1);

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

      retVal = maxStayApp.validate(trx, itin, paxTypeFare2, &maxStayRule, fm2);

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
      trx.diagnostic().diagnosticType() = Diagnostic307;
      trx.diagnostic().activate();

      DateTime emptyDate;
      // Step 2. Create Rule data
      //
      MaxStayRestriction maxStayRule;
      maxStayRule.vendor() = "ATP";
      maxStayRule.itemNo() = 11069;
      maxStayRule.maxStayDate() = emptyDate;
      maxStayRule.geoTblItemNoFrom() = 123;
      maxStayRule.geoTblItemNoTo() = 3430;
      maxStayRule.maxStay() = "001";
      maxStayRule.maxStayUnit() = "M";

      PricingUnit* pricingUnit = TestPricingUnitFactory::create(
          "/vobs/atseintl/Rules/test/data/MaximumStayPricingUnit.xml");

      const FarePath farePath;
      const FareUsage* fareUsage = pricingUnit->fareUsage()[0];

      MaximumStayApplication maxStayApp;

      Record3ReturnTypes retVal =
          maxStayApp.validate(trx, &maxStayRule, farePath, *pricingUnit, *fareUsage);

      CPPUNIT_ASSERT(retVal == PASS);
    }
    catch (std::exception& ex) {}
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MaximumStayTest);
}
