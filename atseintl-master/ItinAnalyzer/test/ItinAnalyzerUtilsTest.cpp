//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include <iostream>
#include <vector>

#include "test/include/TestMemHandle.h"
#include "test/LocGenerator.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/ErrorResponseException.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "ItinAnalyzer/test/ItinTestUtil.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
namespace tse
{
using namespace itinanalyzerutils;
using namespace ::testing;

std::ostream& operator<<(std::ostream& out, const FareMarket& fm)
{
  out << "<FareMarket travel segments:";
  for (std::vector<TravelSeg*>::const_iterator iter = fm.travelSeg().begin();
       iter != fm.travelSeg().end();
       ++iter)
  {
    out << " " << *iter;
  }
  out << ">";
  return out;
}

std::ostream& operator<<(std::ostream& out, const std::set<FareMarket*>& markets)
{
  out << "Fare markets:" << std::endl;
  for (std::set<FareMarket*>::const_iterator iter = markets.begin(); iter != markets.end(); ++iter)
  {
    out << *(*iter) << std::endl;
  }
  out << "." << std::endl;
  return out;
}

class MockItinAnalyzerServiceWrapper
{
public:
  MOCK_CONST_METHOD3(setupAndStoreFareMarket,
                     bool(PricingTrx& trx, Itin& itin, FareMarket* newFareMarket));
};

class StructreRuleTrxMock : public PricingTrx
{
public:
  MOCK_CONST_METHOD0(getMultiPassengerFCMapping, MultiPaxFCMapping*());
  MOCK_CONST_METHOD0(isMultiPassengerSFRRequestType, bool());
  MOCK_METHOD0(setMultiPassengerSFRRequestType, void());
};

class ItinAnalyzerUtilsTest : public Test
{
public:
  void SetUp() override
  {
    _arunkSeg = _memHandle.create<ArunkSeg>();
    _airSeg = _memHandle.create<AirSeg>();
    _itin = _memHandle.create<Itin>();
    _pricingTrx = _memHandle.create<PricingTrx>();
  }

  void TearDown() override { _memHandle.clear(); }

  void prepareMultiPaxFCMap(MultiPaxFCMapping& multiPaxFCMapping)
  {
    std::vector<TravelSeg*> travelSegVector1 = {
        _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>()};
    std::vector<TravelSeg*> travelSegVector2 = {
        _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>()};
    std::vector<TravelSeg*> travelSegVector3 = {
        _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>()};
    std::vector<TravelSeg*> travelSegVector4 = {
        _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>()};

    PaxType* paxType1 = _memHandle.create<PaxType>();
    FareCompInfo* PT1_fc1 = _memHandle.create<FareCompInfo>();
    FareMarket* PT1_fm1 = _memHandle.create<FareMarket>();
    PT1_fm1->travelSeg() = travelSegVector1;
    PT1_fc1->fareMarket() = PT1_fm1;

    FareCompInfo* PT1_fc2 = _memHandle.create<FareCompInfo>();
    FareMarket* PT1_fm2 = _memHandle.create<FareMarket>();
    PT1_fm2->travelSeg() = travelSegVector2;
    PT1_fc2->fareMarket() = PT1_fm2;

    multiPaxFCMapping[paxType1].push_back(PT1_fc1);
    multiPaxFCMapping[paxType1].push_back(PT1_fc2);

    PaxType* paxType2 = _memHandle.create<PaxType>();
    FareCompInfo* PT2_fc1 = _memHandle.create<FareCompInfo>();
    FareMarket* PT2_fm1 = _memHandle.create<FareMarket>();
    PT2_fm1->travelSeg() = travelSegVector3;
    PT2_fc1->fareMarket() = PT2_fm1;

    FareCompInfo* PT2_fc2 = _memHandle.create<FareCompInfo>();
    FareMarket* PT2_fm2 = _memHandle.create<FareMarket>();
    PT2_fm2->travelSeg() = travelSegVector4;
    PT2_fc2->fareMarket() = PT2_fm2;

    multiPaxFCMapping[paxType2].push_back(PT2_fc1);
    multiPaxFCMapping[paxType2].push_back(PT2_fc2);
  }

  void prepareMultiPaxFCMapWithRedundantFM(MultiPaxFCMapping& multiPaxFCMapping)
  {
    std::vector<TravelSeg*> travelSegVector1 = {
        _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>()};
    std::vector<TravelSeg*> travelSegVector2 = {
        _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>()};

    PaxType* paxType1 = _memHandle.create<PaxType>();
    FareCompInfo* PT1_fc1 = _memHandle.create<FareCompInfo>();
    FareMarket* PT1_fm1 = _memHandle.create<FareMarket>();
    PT1_fm1->travelSeg() = travelSegVector1;
    PT1_fc1->fareMarket() = PT1_fm1;

    FareCompInfo* PT1_fc2 = _memHandle.create<FareCompInfo>();
    FareMarket* PT1_fm2 = _memHandle.create<FareMarket>();
    PT1_fm2->travelSeg() = travelSegVector2;
    PT1_fc2->fareMarket() = PT1_fm2;

    multiPaxFCMapping[paxType1].push_back(PT1_fc1);
    multiPaxFCMapping[paxType1].push_back(PT1_fc2);

    PaxType* paxType2 = _memHandle.create<PaxType>();
    FareCompInfo* PT2_fc1 = _memHandle.create<FareCompInfo>();
    FareMarket* PT2_fm1 = _memHandle.create<FareMarket>();
    PT2_fm1->travelSeg() = travelSegVector1;
    PT2_fc1->fareMarket() = PT2_fm1;

    FareCompInfo* PT2_fc2 = _memHandle.create<FareCompInfo>();
    FareMarket* PT2_fm2 = _memHandle.create<FareMarket>();
    PT2_fm2->travelSeg() = travelSegVector2;
    PT2_fc2->fareMarket() = PT2_fm2;

    multiPaxFCMapping[paxType2].push_back(PT2_fc1);
    multiPaxFCMapping[paxType2].push_back(PT2_fc2);
  }
  TestMemHandle _memHandle;
  ArunkSeg* _arunkSeg;
  AirSeg* _airSeg;
  PricingTrx* _pricingTrx;
  Itin* _itin;
};

TEST_F(ItinAnalyzerUtilsTest, isFareMarketThruTest1)
  {
    TestItin ti(_memHandle);
    ti.addLeg(66, 1);
    FareMarket* fm = ti.buildFareMarket(0, 0);

    ti.populateItin(*_itin);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));
  }

  TEST_F(ItinAnalyzerUtilsTest, isFareMarketThruTest2)
  {
    TestItin ti(_memHandle);
    ti.addLeg(66, 1);
    ti.addLeg(77, 1);
    ti.populateItin(*_itin);

    FareMarket* fm = ti.buildFareMarket(0, 0);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(1, 1);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(0, 1);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));
  }

  TEST_F(ItinAnalyzerUtilsTest, isFareMarketThruTest3)
  {
    TestItin ti(_memHandle);
    ti.addLeg(66, 2);
    ti.addLeg(77, 3);
    ti.addLeg(88, 1);
    ti.populateItin(*_itin);

    FareMarket* fm = ti.buildFareMarket(0, 0);
    ASSERT_EQ(false, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(5, 5);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(3, 3);
    ASSERT_EQ(false, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(0, 1);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(2, 4);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(0, 5);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(1, 5);
    ASSERT_EQ(false, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(2, 5);
    ASSERT_EQ(true, isThruFareMarket(*fm, *_itin));

    fm = ti.buildFareMarket(3, 4);
    ASSERT_EQ(false, isThruFareMarket(*fm, *_itin));
  }

  TEST_F(ItinAnalyzerUtilsTest, gatherUniqueMarketsForItinsTest)
  {
    const int FARE_MARKETS_BASE_SZ = 8;
    const int UNIQUE_FM_TEST_ITINS_NR = 6;

    // Make a base of unique fare markets
    vector<FareMarket*> base;
    for (int i = 0; i < FARE_MARKETS_BASE_SZ; ++i)
    {
      base.push_back(_memHandle.create<FareMarket>());
    }

    // Make some itineraries with "random"
    // fare markets inside
    vector<Itin*> itins;
    for (int i = 0; i < UNIQUE_FM_TEST_ITINS_NR; ++i)
    {
      itins.push_back(_memHandle.create<Itin>());
    }

    itins[0]->fareMarket().push_back(base[0]);
    itins[0]->fareMarket().push_back(base[2]);
    itins[0]->fareMarket().push_back(base[4]);
    itins[0]->fareMarket().push_back(base[6]);

    itins[1]->fareMarket().push_back(base[7]);
    itins[1]->fareMarket().push_back(base[5]);
    itins[1]->fareMarket().push_back(base[3]);
    itins[1]->fareMarket().push_back(base[1]);

    itins[2]->fareMarket().push_back(base[4]);
    itins[2]->fareMarket().push_back(base[1]);
    itins[2]->fareMarket().push_back(base[7]);
    itins[2]->fareMarket().push_back(base[0]);
    itins[2]->fareMarket().push_back(base[2]);

    itins[3]->fareMarket().push_back(base[6]);
    itins[3]->fareMarket().push_back(base[5]);

    itins[5]->fareMarket().push_back(base[3]);
    itins[5]->fareMarket().push_back(base[2]);
    itins[5]->fareMarket().push_back(base[7]);
    itins[5]->fareMarket().push_back(base[1]);

    // The response set
    std::set<const FareMarket*> response;
    collectUniqueMarketsForItins(itins, response);

    // The set to compare
    // made explicitly from the base vector
    std::set<const FareMarket*> compare(base.begin(), base.end());

    ASSERT_TRUE(response == compare);
  }

  TEST_F(ItinAnalyzerUtilsTest, filterBrandsForISTest)
  {
    const int BRANDS = 20;

    FareMarket fm1, fm2, fm3;
    BrandProgram program;
    BrandInfo brands[BRANDS];

    for (int index = 0; index < BRANDS; ++index)
    {
      std::stringstream tmp;
      tmp << char(int('A') + int(index / 10)) << (index % 10);
      brands[index].brandCode() = tmp.str();
      _pricingTrx->brandProgramVec().push_back(std::make_pair(&program, &(brands[index])));
      if (index % 2)
        fm1.brandProgramIndexVec().push_back(index);
      if (index % 3)
        fm2.brandProgramIndexVec().push_back(index);
      if ((index % 2) || (index % 3))
        fm3.brandProgramIndexVec().push_back(index);

      if (index % 4)
        _pricingTrx->getMutableBrandsFilterForIS().insert(brands[index].brandCode());
    }
    _pricingTrx->fareMarket().push_back(&fm1);
    _pricingTrx->fareMarket().push_back(&fm2);
    _pricingTrx->fareMarket().push_back(&fm3);

    _pricingTrx->setTrxType(PricingTrx::IS_TRX);

    // for BRANDS = 20
    // trx
    //      index:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
    // brand code: A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 B0 B1 B2 B3 B4 B5 B6 B7 B8 B9
    // trx filter
    // brand code:    A1 A2 A3    A5 A6 A7    A9 B0 B1    B3 B4 B5    B7 B8 B9
    // fare markets
    //        fm1:     1     3     5     7     9    11    13    15    17    19
    //        fm2:     1  2     4  6     7  8    10 11    13 14    16 17    19
    //        fm3:     1  2  3  4  5     7  8  9 10 11    13 14 15 16 17    19
    //
    // expected result:
    // trx
    //      index:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
    // brand code: A1 A2 A3 A5 A6 A7 A9 B0 B1 B3 B4 B5 B7 B8 B9
    // fare markets
    //        fm1:  0     2  3     5  6     8  9    11 12    14
    //        fm2:  0  1     3     5     7  8  9 10    12    14
    //        fm3:  0  1  2  3     5  6  7  8  9 10 11 12    14

    // check condition for brands = 20
    ASSERT_EQ((size_t)20, _pricingTrx->brandProgramVec().size());
    ASSERT_EQ((size_t)3, _pricingTrx->fareMarket().size());
    ASSERT_EQ((size_t)10, fm1.brandProgramIndexVec().size());
    ASSERT_EQ((size_t)13, fm2.brandProgramIndexVec().size());
    ASSERT_EQ((size_t)16, fm3.brandProgramIndexVec().size());
    ASSERT_EQ((size_t)15, _pricingTrx->getBrandsFilterForIS().size());

    itinanalyzerutils::filterBrandsForIS(*_pricingTrx);

    ASSERT_EQ((size_t)15, _pricingTrx->brandProgramVec().size());
    BrandCode expectedBrandCodes[15] = {"A1", "A2", "A3", "A5", "A6", "A7", "A9",
                                        "B0", "B1", "B3", "B4", "B5", "B7", "B8", "B9"};
    int index = 0;
    for (BrandCode& b: expectedBrandCodes)
      ASSERT_EQ(b, _pricingTrx->brandProgramVec()[index++].second->brandCode());

    ASSERT_EQ((size_t)10, fm1.brandProgramIndexVec().size());
    ASSERT_EQ((size_t)10, fm2.brandProgramIndexVec().size());
    ASSERT_EQ((size_t)13, fm3.brandProgramIndexVec().size());
    int expectedFm1BrandIndices[10] = {0, 2, 3, 5, 6, 8, 9, 11, 12, 14};
    int expectedFm2BrandIndices[10] = {0, 1, 3, 5, 7, 8, 9, 10, 12, 14};
    int expectedFm3BrandIndices[13] = {0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 14};
    index = 0;
    for (int i: expectedFm1BrandIndices)
      ASSERT_EQ(i, fm1.brandProgramIndexVec()[index++]);
    index = 0;
    for (int i: expectedFm2BrandIndices)
      ASSERT_EQ(i, fm2.brandProgramIndexVec()[index++]);
    index = 0;
    for (int i: expectedFm3BrandIndices)
      ASSERT_EQ(i, fm3.brandProgramIndexVec()[index++]);
  }

  TEST_F(ItinAnalyzerUtilsTest, assignLegsToSegments)
  {
    LocGenerator locGenerator;
    AirSeg seg1,seg2,seg3,seg4,seg5,seg6,seg7;
    Loc *loc1, *loc2, *loc3, *loc4, *loc5, *loc6, *loc7;
    loc1 = locGenerator.createLoc('N', 50, 3, 53, 'E', 19, 56, 42, "KRK");
    loc2 = locGenerator.createLoc('N', 52, 13, 47, 'E', 21, 0, 44, "WAW");
    loc3 = locGenerator.createLoc('N', 51, 30, 26, 'E', 0, 7, 40, "LON");
    loc4 = locGenerator.createLoc('N', 40, 25, 0, 'E', -3, 42, 14, "MAD");
    loc5 = locGenerator.createLoc('N', 40, 42, 52, 'E', -74, 0, 22, "NYC");
    loc6 = locGenerator.createLoc('N', 25, 47, 20, 'E', -80, 13, 35, "MIA");
    loc7 = locGenerator.createLoc('N', 34, 3, 8, 'E', -118, 14, 37, "LAX");

    seg1.origin() = loc1;
    seg1.destination() = loc2;
    seg1.departureDT() = DateTime(2016, 11, 1, 0, 0, 1);
    seg1.arrivalDT() = DateTime(2016, 11, 1, 2, 0, 1);
    seg1.setBrandCode("SV");

    seg2.origin() = loc2;
    seg2.destination() = loc3;
    seg2.departureDT() = DateTime(2016, 11, 1, 5, 0, 1);
    seg2.arrivalDT() = DateTime(2016, 11, 1, 8, 0, 1);
    seg2.setBrandCode("SV");

    // stopover
    seg3.origin() = loc3;
    seg3.destination() = loc4;
    seg3.departureDT() = DateTime(2016, 11, 3, 0, 0, 1);
    seg3.arrivalDT() = DateTime(2016, 11, 3, 2, 0, 1);
    seg3.setBrandCode("SV");

    // change of brand
    seg4.origin() = loc4;
    seg4.destination() = loc5;
    seg4.departureDT() = DateTime(2016, 11, 3, 12, 0, 1);
    seg4.arrivalDT() = DateTime(2016, 11, 3, 19, 0, 1);
    seg4.setBrandCode("FL");

    // stopover
    seg5.origin() = loc5;
    seg5.destination() = loc6;
    seg5.departureDT() = DateTime(2016, 11, 5, 0, 0, 1);
    seg5.arrivalDT() = DateTime(2016, 11, 5, 2, 0, 1);
    seg5.setBrandCode("FL");

    seg6.origin() = loc6;
    seg6.destination() = loc7;
    seg6.departureDT() = DateTime(2016, 11, 5, 5, 0, 1);
    seg6.arrivalDT() = DateTime(2016, 11, 5, 9, 0, 1);
    seg6.setBrandCode("FL");

    // split by furthest point (LAX)
    seg7.origin() = loc7;
    seg7.destination() = loc5; // coming back to NYC
    seg7.departureDT() = DateTime(2016, 11, 5, 11, 0, 1);
    seg7.arrivalDT() = DateTime(2016, 11, 5, 23, 0, 1);
    seg7.setBrandCode("FL");

    std::vector<TravelSeg*> segments = { &seg1, &seg2, &seg3, &seg4, &seg5, &seg6, &seg7 };
    itinanalyzerutils::assignLegsToSegments(segments, _pricingTrx->dataHandle(), 0);
    ASSERT_TRUE(seg1.legId() == 0);
    ASSERT_TRUE(seg2.legId() == 0);
    ASSERT_TRUE(seg3.legId() == 1);
    ASSERT_TRUE(seg4.legId() == 2);
    ASSERT_TRUE(seg5.legId() == 3);
    ASSERT_TRUE(seg6.legId() == 3);
    ASSERT_TRUE(seg7.legId() == 4);
  }

  TEST_F(ItinAnalyzerUtilsTest, assignLegsToSegments_splitByFurthestPointOnly)
  {
    LocGenerator locGenerator;
    AirSeg seg1,seg2,seg3,seg4;
    Loc *loc1, *loc2, *loc3, *loc4;
    loc1 = locGenerator.createLoc('N', 50, 3, 53, 'E', 19, 56, 42, "KRK");
    loc2 = locGenerator.createLoc('N', 52, 13, 47, 'E', 21, 0, 44, "WAW");
    loc3 = locGenerator.createLoc('N', 51, 30, 26, 'E', 0, 7, 40, "LON");
    loc4 = locGenerator.createLoc('N', 40, 42, 52, 'E', -74, 0, 22, "NYC");

    seg1.origin() = loc1;
    seg1.destination() = loc2;
    seg1.departureDT() = DateTime(2016, 11, 1, 0, 0, 1);
    seg1.arrivalDT() = DateTime(2016, 11, 1, 2, 0, 1);

    seg2.origin() = loc2;
    seg2.destination() = loc3;
    seg2.departureDT() = DateTime(2016, 11, 1, 5, 0, 1);
    seg2.arrivalDT() = DateTime(2016, 11, 1, 8, 0, 1);

    // turnaround point at destination of this segment
    seg3.origin() = loc3;
    seg3.destination() = loc4;
    seg3.departureDT() = DateTime(2016, 11, 1, 8, 0, 1);
    seg3.arrivalDT() = DateTime(2016, 11, 1, 20, 0, 1);

    seg4.origin() = loc4;
    seg4.destination() = loc1;
    seg4.departureDT() = DateTime(2016, 11, 2, 1, 0, 1);
    seg4.arrivalDT() = DateTime(2016, 11, 2, 11, 0, 1);

    std::vector<TravelSeg*> segments = { &seg1, &seg2, &seg3, &seg4 };
    itinanalyzerutils::assignLegsToSegments(segments, _pricingTrx->dataHandle(), 0);
    ASSERT_TRUE(seg1.legId() == 0);
    ASSERT_TRUE(seg2.legId() == 0);
    ASSERT_TRUE(seg3.legId() == 0);
    ASSERT_TRUE(seg4.legId() == 1);
  }

  TEST_F(ItinAnalyzerUtilsTest, assignLegsToSegments_oneSegmentOnly)
  {
    LocGenerator locGenerator;
    AirSeg seg1;
    Loc *loc1, *loc2;
    loc1 = locGenerator.createLoc('N', 50, 3, 53, 'E', 19, 56, 42, "KRK");
    loc2 = locGenerator.createLoc('N', 52, 13, 47, 'E', 21, 0, 44, "WAW");

    seg1.origin() = loc1;
    seg1.destination() = loc2;
    seg1.departureDT() = DateTime(2016, 11, 1, 0, 0, 1);
    seg1.arrivalDT() = DateTime(2016, 11, 1, 2, 0, 1);

    std::vector<TravelSeg*> segments = { &seg1 };
    itinanalyzerutils::assignLegsToSegments(segments, _pricingTrx->dataHandle(), 0);
    ASSERT_TRUE(seg1.legId() == 0);
  }

  TEST_F(ItinAnalyzerUtilsTest, removeRedundantFareMarket_similarFareMarketsTvlSegs)
  {
    MultiPaxFCMapping multiPaxFCMapping;

    prepareMultiPaxFCMapWithRedundantFM(multiPaxFCMapping);

    itinanalyzerutils::removeRedundantFareMarket(multiPaxFCMapping);

    std::unordered_set<FareMarket*> uniqueFareMarketList;
    for (auto& mapElem : multiPaxFCMapping)
      for (auto* fc : mapElem.second)
        uniqueFareMarketList.insert(fc->fareMarket());

    // There should be only 2 fare markets, because we removed 2
    ASSERT_TRUE(uniqueFareMarketList.size() == 2);

    auto PT1_fc1 = (*multiPaxFCMapping.begin()).second[0];
    auto PT2_fc1 = (*(++multiPaxFCMapping.begin())).second[0];
    auto PT1_fc2 = (*multiPaxFCMapping.begin()).second[1];
    auto PT2_fc2 = (*(++multiPaxFCMapping.begin())).second[1];

    ASSERT_TRUE(PT1_fc1->fareMarket() == PT2_fc1->fareMarket());
    ASSERT_TRUE(PT1_fc2->fareMarket() == PT2_fc2->fareMarket());
  }

  TEST_F(ItinAnalyzerUtilsTest, removeRedundantFareMarket_differentFareMarketsTvlSegs)
  {
    MultiPaxFCMapping multiPaxFCMapping;
    prepareMultiPaxFCMap(multiPaxFCMapping);

    itinanalyzerutils::removeRedundantFareMarket(multiPaxFCMapping);

    std::unordered_set<FareMarket*> uniqueFareMarketList;
    for (auto& mapElem : multiPaxFCMapping)
      for (auto* fc : mapElem.second)
        uniqueFareMarketList.insert(fc->fareMarket());

    // We expect that any fare markets was removed
    ASSERT_TRUE(uniqueFareMarketList.size() == 4);
  }

  // arg type =  (FareMarket*)
  MATCHER_P(isFareBasisesOnFareMarket, fbCodeList, "")
  {
    auto& fbCodes = arg->getMultiPaxUniqueFareBasisCodes();
    for (auto& fbCode : fbCodeList)
    {
      if (fbCodes.find(fbCode) == fbCodes.end())
        return false;
    }
    return true;
  }

  TEST_F(ItinAnalyzerUtilsTest, initializeFareMarketsFromFareComponents)
  {
    MultiPaxFCMapping multiPaxFCMapping;

    prepareMultiPaxFCMapWithRedundantFM(multiPaxFCMapping);

    itinanalyzerutils::removeRedundantFareMarket(multiPaxFCMapping);

    std::unordered_set<FareMarket*> uniqueFareMarketList;
    for (auto& mapElem : multiPaxFCMapping)
      for (auto* fc : mapElem.second)
        uniqueFareMarketList.insert(fc->fareMarket());

    // There should be only 2 fare markets, because we removed 2
    ASSERT_TRUE(uniqueFareMarketList.size() == 2);

    auto PT1_fc1 = (*(multiPaxFCMapping.begin())).second[0];
    auto PT2_fc1 = (*(++multiPaxFCMapping.begin())).second[0];
    auto PT1_fc2 = (*(multiPaxFCMapping.begin())).second[1];
    auto PT2_fc2 = (*(++multiPaxFCMapping.begin())).second[1];

    PT1_fc1->fareBasisCode() = "FareBasis1";
    PT1_fc2->fareBasisCode() = "FareBasis2";
    PT2_fc1->fareBasisCode() = "FareBasis3";
    PT2_fc2->fareBasisCode() = "FareBasis4";

    StructreRuleTrxMock sfrTrxMock;
    sfrTrxMock.itin().push_back(_itin);
    MockItinAnalyzerServiceWrapper wrapperMock;

    EXPECT_CALL(sfrTrxMock, getMultiPassengerFCMapping()).WillOnce(Return(&multiPaxFCMapping));

    // We expect to store 2 Fare Basis codes on first Fare Market
    std::vector<std::string> fbList1 = {std::string("FareBasis1"), std::string("FareBasis3")};
    EXPECT_CALL(wrapperMock, setupAndStoreFareMarket(_, _, isFareBasisesOnFareMarket(fbList1)))
        .Times(1);

    // We expect to store 2 Fare Basis codes on second Fare Market
    std::vector<std::string> fbList2 = {std::string("FareBasis2"), std::string("FareBasis4")};
    EXPECT_CALL(wrapperMock, setupAndStoreFareMarket(_, _, isFareBasisesOnFareMarket(fbList2)))
        .Times(1);

    itinanalyzerutils::initializeFareMarketsFromFareComponents(sfrTrxMock, &wrapperMock);
  }

} // namespace tse
