// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Xform/StructuredRulesResponseFormatter.h"

#include "Common/XMLConstruct.h"
#include "DataModel/StructuredRuleData.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/StructuredRuleTrx.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

namespace tse
{
using namespace ::testing;

class StructuredRulesResponseFormatterTest : public Test
{
  void SetUp() override
  {
    _memHandle.create<TestConfigInitializer>();
    _calcTotals = _memHandle.create<CalcTotals>();
    Itin* itin = _memHandle.create<Itin>();
    _paxType = _memHandle.create<PaxType>();
    _farePath = _memHandle.create<FarePath>();
    _farePath->paxType() = _paxType;
    _farePath->itin() = itin;
    _calcTotals->farePath = _farePath;
  }
  void TearDown() override { _memHandle.clear(); }

protected:
  PricingTrx pricingTrx;

  CalcTotals* _calcTotals;
  FarePath* _farePath;
  TestMemHandle _memHandle;
  PaxType* _paxType;
};
TEST_F(StructuredRulesResponseFormatterTest, testEmpty)
{
  XMLConstruct construct;

  PricingUnit pu;
  _farePath->pricingUnit().push_back(&pu);
  pu.createMostRestrictivePricingUnitSFRData();

  StructuredRulesResponseFormatter::formatResponse(pricingTrx, *_calcTotals, construct);
  std::string expRes = "<PUD PUN=\"1\"/>";
  auto found = construct.getXMLData().find(expRes);
  EXPECT_TRUE(found != std::string::npos);
}

TEST_F(StructuredRulesResponseFormatterTest, testOneMinStay)
{
  XMLConstruct construct;

  PricingUnit pu;
  _farePath->pricingUnit().push_back(&pu);
  pu.createMostRestrictivePricingUnitSFRData();
  pu.getMostRestrictivePricingUnitSFRData()._minStayMap[0] =
      std::make_pair(DateTime(2016, 10, 1, 10, 20), LocCode("LON"));

  StructuredRulesResponseFormatter::formatResponse(pricingTrx, *_calcTotals, construct);

  std::string expRes = "<PUD PUN=\"1\"><MIN MSD=\"2016-10-01\" MST=\"10:20\" LOC=\"LON\"/></PUD>";
  auto found = construct.getXMLData().find(expRes);
  EXPECT_TRUE(found != std::string::npos);
}

TEST_F(StructuredRulesResponseFormatterTest, testTwoPricingUnits)
{
  XMLConstruct construct;

  PricingUnit pu;
  _farePath->pricingUnit().push_back(&pu);
  pu.createMostRestrictivePricingUnitSFRData();
  pu.getMostRestrictivePricingUnitSFRData()._minStayMap[0] =
      std::make_pair(DateTime(2016, 10, 1, 10, 20), LocCode("LON"));
  pu.getMostRestrictivePricingUnitSFRData()._minStayMap[1] =
      std::make_pair(DateTime(2016, 11, 1, 0, 20), LocCode("HAM"));

  PricingUnit pu2;
  _farePath->pricingUnit().push_back(&pu2);
  pu2.createMostRestrictivePricingUnitSFRData();
  pu2.getMostRestrictivePricingUnitSFRData()._minStayMap[0] =
      std::make_pair(DateTime(2016, 12, 1, 9, 20), LocCode("HKG"));

  StructuredRulesResponseFormatter::formatResponse(pricingTrx, *_calcTotals, construct);

  std::string expRes = "<PUD PUN=\"1\"><MIN MSD=\"2016-10-01\" MST=\"10:20\" LOC=\"LON\"/>";
  expRes += "<MIN MSD=\"2016-11-01\" MST=\"00:20\" LOC=\"HAM\"/></PUD>";
  expRes += "<PUD PUN=\"2\"><MIN MSD=\"2016-12-01\" MST=\"09:20\" LOC=\"HKG\"/></PUD>";
  auto found = construct.getXMLData().find(expRes);
  EXPECT_TRUE(found != std::string::npos);
}

TEST_F(StructuredRulesResponseFormatterTest, testMinMaxRestrictionsExist)
{
  XMLConstruct construct;

  PricingUnit pu;
  _farePath->pricingUnit().push_back(&pu);
  pu.createMostRestrictivePricingUnitSFRData();

  pu.getMostRestrictivePricingUnitSFRData()._minStayMap[0] =
      std::make_pair(DateTime(2016, 10, 1, 10, 20), LocCode("LON"));
  pu.getMostRestrictivePricingUnitSFRData()._maxStayMap[0] =
      MaxStayData(LocCode("LON"), DateTime(2017, 10, 4, 13, 20), DateTime(2017, 11, 1, 10, 20));

  pu.getMostRestrictivePricingUnitSFRData()._minStayMap[1] =
      std::make_pair(DateTime(2016, 11, 1, 0, 20), LocCode("HAM"));

  PricingUnit pu2;
  _farePath->pricingUnit().push_back(&pu2);
  pu2.createMostRestrictivePricingUnitSFRData();
  pu2.getMostRestrictivePricingUnitSFRData()._minStayMap[0] =
      std::make_pair(DateTime(2000, 2, 1, 9, 20), LocCode("HKG"));
  pu2.getMostRestrictivePricingUnitSFRData()._maxStayMap[0] =
      MaxStayData(LocCode("HKG"), DateTime(2000, 10, 1, 10, 20), DateTime(2001, 11, 1, 10, 20));

  StructuredRulesResponseFormatter::formatResponse(pricingTrx, *_calcTotals, construct);

  std::string expRes = "<PUD PUN=\"1\"><MIN MSD=\"2016-10-01\" MST=\"10:20\" LOC=\"LON\"/>";
  expRes += "<MIN MSD=\"2016-11-01\" MST=\"00:20\" LOC=\"HAM\"/>";
  expRes +=
      "<MAX MSD=\"2017-10-04\" MST=\"13:20\" LDC=\"2017-11-01\" LTC=\"10:20\" LOC=\"LON\"/></PUD>";

  expRes += "<PUD PUN=\"2\"><MIN MSD=\"2000-02-01\" MST=\"09:20\" LOC=\"HKG\"/>";
  expRes +=
      "<MAX MSD=\"2000-10-01\" MST=\"10:20\" LDC=\"2001-11-01\" LTC=\"10:20\" LOC=\"HKG\"/></PUD>";
  auto found = construct.getXMLData().find(expRes);
  EXPECT_TRUE(found != std::string::npos);
}

TEST_F(StructuredRulesResponseFormatterTest, testDataFromMultiPaxMapWithoutSFRData)
{
  XMLConstruct construct;
  StructuredRuleTrx sfrTrx;
  sfrTrx.createMultiPassangerFCMapping();
  sfrTrx.setMultiPassengerSFRRequestType();
  MultiPaxFCMapping& multiPaxFCMapping = *sfrTrx.getMultiPassengerFCMapping();

  TravelSeg* seg1 = _memHandle.create<AirSeg>();
  TravelSeg* seg2 = _memHandle.create<AirSeg>();
  TravelSeg* seg3 = _memHandle.create<AirSeg>();
  TravelSeg* seg4 = _memHandle.create<AirSeg>();
  TravelSeg* seg5 = _memHandle.create<AirSeg>();
  TravelSeg* seg6 = _memHandle.create<AirSeg>();

  std::vector<TravelSeg*> travelSegVector1 = {seg1, seg2, seg3};
  std::vector<TravelSeg*> travelSegVector2 = {seg4, seg5, seg6};

  PricingUnit pu;
  FareUsage fu1;
  FareUsage fu2;

  fu1.travelSeg() = travelSegVector1;
  fu2.travelSeg() = travelSegVector2;
  pu.fareUsage().push_back(&fu1);
  pu.fareUsage().push_back(&fu2);
  _farePath->pricingUnit().push_back(&pu);

  FareCompInfo* fc1 = _memHandle.create<FareCompInfo>();
  fc1->fareCompNumber() = 1;
  FareMarket* fm1 = _memHandle.create<FareMarket>();
  fm1->travelSeg() = travelSegVector1;
  fc1->fareMarket() = fm1;

  FareCompInfo* fc2 = _memHandle.create<FareCompInfo>();
  fc2->fareCompNumber() = 2;
  FareMarket* fm2 = _memHandle.create<FareMarket>();
  fm2->travelSeg() = travelSegVector2;
  fc2->fareMarket() = fm2;

  multiPaxFCMapping[_paxType].push_back(fc1);
  multiPaxFCMapping[_paxType].push_back(fc2);

  StructuredRulesResponseFormatter::formatResponse(sfrTrx, *_calcTotals, construct);

  ASSERT_STREQ("<FCD Q6D=\"1\" PUN=\"1\"/><FCD Q6D=\"2\" PUN=\"1\"/><PUD PUN=\"1\"/>",
               construct.getXMLData().c_str());
}
}
