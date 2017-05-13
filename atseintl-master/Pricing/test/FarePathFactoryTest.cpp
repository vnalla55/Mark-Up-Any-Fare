//----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//----------------------------------------------------------------------------

#include "Common/BookingCodeUtil.h"
#include "Common/ClassOfService.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/MarriedCabin.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Combinations.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/PUPath.h"
#include "Pricing/test/FactoriesConfigStub.h"
#include "Pricing/test/PricingMockDataBuilder.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class FPFDataHandleMock : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<MarriedCabin*>& getMarriedCabins(const CarrierCode& carrier,
                                                     const BookingCode& premiumCabin,
                                                     const DateTime& versionDate)
  {
    // static std::vector<MarriedCabin*> ret;

    std::vector<MarriedCabin*>* ret = _memHandle.create<std::vector<MarriedCabin*> >();

    if ("A3" == premiumCabin && "AA" == carrier)
    {
      ret->push_back(makeMarriedCabin("KRK", "LON", "AA", "Z3"));
    }
    else if ("B3" == premiumCabin && "AA" == carrier)
    {
      ret->push_back(makeMarriedCabin("KRK", "LON", "AA", "Z6", '2'));
    }
    else if ("C3" == premiumCabin && "AA" == carrier)
    {
      ret->push_back(makeMarriedCabin("KRK", "LON", "AA", "Z6", '3'));
    }
    else if ("D3" == premiumCabin && "AA" == carrier)
    {
      ret->push_back(makeMarriedCabin("KRK", "LON", "AA", "Z6", '4'));
    }
    else if ("E3" == premiumCabin && "AA" == carrier)
    {
      ret->push_back(makeMarriedCabin("KRK", "LON", "AA", "Z6", ' '));
    }
    else if ("F3" == premiumCabin && "AA" == carrier)
    {
      ret->push_back(makeMarriedCabin("KRK", "LON", "AA", "X3", '1'));
      ret->push_back(makeMarriedCabin("BBB", "LON", "AA", "Z9", '1'));
      ret->push_back(makeMarriedCabin("KRK", "LON", "AA", "Y6", '1'));
    }

    return (*ret);
  }

  ~FPFDataHandleMock() { _memHandle.clear(); }

private:
  MarriedCabin* makeMarriedCabin(const LocCode& loc1,
                                 const LocCode& loc2,
                                 const CarrierCode& carrier,
                                 const BookingCode& mCabine,
                                 Indicator direct = '1')
  {
    MarriedCabin* cm = _memHandle.create<MarriedCabin>();
    cm->marriedCabin() = mCabine;
    cm->directionality() = direct;
    cm->loc1().loc() = loc1;
    cm->loc1().locType() = LOCTYPE_CITY;
    cm->loc2().loc() = loc2;
    cm->carrier() = carrier;
    return cm;
  }
};

class FarePathFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FarePathFactoryTest);

  CPPUNIT_TEST(testFindPUFactoryIdx);
  CPPUNIT_TEST(testFailedFareExistsInPU);
  CPPUNIT_TEST(testRecalculatePriority);
  CPPUNIT_TEST(testReplaceWithNewPU);

  CPPUNIT_TEST(testFppqPriorityIsSameAsPupqPriorityWhenSamePriority);
  CPPUNIT_TEST(testFppqPriorityChangesToLowerWhenPupqPriorityHasLowerPriority);
  CPPUNIT_TEST(testFppqPriorityNoChangeWhenPupqPriorityHasHigherPriority);
  CPPUNIT_TEST(testHigherPriorityIsOnTopWhenHigherPriorityFppqItemIsPushedLast);
  CPPUNIT_TEST(testHigherPriorityIsOnTopWhenHigherPriorityFppqItemIsPushedFirst);

  CPPUNIT_TEST(testGetFareUsageReturn0WhenTvlSegNotFound);
  CPPUNIT_TEST(testGetFareUsageReturnFirstFareUsageForFirstTravelSeg);
  CPPUNIT_TEST(testGetFareUsageReturnSecondFareUsageForSecondTravelSeg);
  CPPUNIT_TEST(testGetFareUsageReturnThirdFareUsageForThirdTravelSeg);

  CPPUNIT_TEST(testDifferentialDataReturnZeroWhenFuHasNoDiffPlusUps);
  CPPUNIT_TEST(testDifferentialDataReturnZeroIfDiffDataFail);
  CPPUNIT_TEST(testDifferentialDataReturnZeroIfTvlSegNotFound);
  CPPUNIT_TEST(testDifferentialDataReturnDiffDataFromFu);

  CPPUNIT_TEST(testCheckPremiumMarriageA);
  CPPUNIT_TEST(testCheckPremiumMarriageA2);
  CPPUNIT_TEST(testCheckPremiumMarriageB);
  CPPUNIT_TEST(testCheckPremiumMarriageC);
  CPPUNIT_TEST(testCheckPremiumMarriageD);
  CPPUNIT_TEST(testCheckPremiumMarriageE);
  CPPUNIT_TEST(testCheckPremiumMarriageF);

  CPPUNIT_SKIP_TEST(testCheckEarlyEOECombinabilityA);

  // ValidatingCxr Project
  CPPUNIT_TEST(testRemoveTaggedValCxr);
  CPPUNIT_TEST(testGetTaggedFarePathWithValCxr);

  CPPUNIT_TEST(testMergeFarePathWithEqualComponents_NoMergeDiffComponents);
  CPPUNIT_TEST(testMergeFarePathWithEqualComponents_NoMergeDiffTotal);
  CPPUNIT_TEST(testMergeFarePathWithEqualComponents_MergeAll);
  CPPUNIT_TEST(testMergeFarePathWithEqualComponents_MergeFew);
  CPPUNIT_TEST(testFindLowestFaerPath_SameAmount);
  CPPUNIT_TEST(testFindLowestFaerPath_DiffAmountLastLowest);
  CPPUNIT_TEST(testFindLowestFaerPath_DiffAmountMiddleLowest);
  CPPUNIT_TEST(testProcessFarePathClones_FPPQItemNotValid);
  CPPUNIT_TEST(testProcessFarePathClones_FPPQItemValidLower);
  CPPUNIT_TEST(testProcessFarePathClones_FPPQItemValidHigher);
  CPPUNIT_TEST(testIsFarePathValidForCorpIDFare_NoCorpIDFare);
  CPPUNIT_TEST(testIsFarePathValidForCorpIDFare_OneCorpIDFare);
  CPPUNIT_TEST(testIsFarePathValidForCorpIDFare_AllCorpIDFare);
  CPPUNIT_TEST(testIsFarePathValidForFFG_XOFareON);
  CPPUNIT_TEST(testIsFarePathValidForFFG_XOFareOFF);

  CPPUNIT_TEST_SUITE_END();

public:
  FarePathFactoryTest() {}

  void setUp()
  {
    _memHandle.create<FPFDataHandleMock>();
    _memHandle.create<TestConfigInitializer>();

    _factory = new FarePathFactory(_factoriesConfig);
    _farePath = new FarePath;

    _pu1 = new PricingUnit;
    _fu11 = new FareUsage;
    _ptf11 = new PaxTypeFare;
    _fu12 = new FareUsage;
    _ptf12 = new PaxTypeFare;
    _pu2 = new PricingUnit;
    _fu21 = new FareUsage;
    _ptf21 = new PaxTypeFare;
    _fu22 = new FareUsage;
    _ptf22 = new PaxTypeFare;

    _fu11->paxTypeFare() = _ptf11;
    _fu12->paxTypeFare() = _ptf12;
    _fu21->paxTypeFare() = _ptf21;
    _fu22->paxTypeFare() = _ptf22;
    _pu1->fareUsage().push_back(_fu11);
    _pu1->fareUsage().push_back(_fu12);
    _pu2->fareUsage().push_back(_fu21);
    _pu2->fareUsage().push_back(_fu22);
    _farePath->pricingUnit().push_back(_pu1);
    _farePath->pricingUnit().push_back(_pu2);

    _fppqItem = new FPPQItem;
    _pupqItem1 = new PUPQItem;
    _pupqItem2 = new PUPQItem;

    _pupqItem1->pricingUnit() = _pu1;
    _pupqItem2->pricingUnit() = _pu2;
    _fppqItem->pupqItemVect().push_back(_pupqItem1);
    _fppqItem->pupqItemVect().push_back(_pupqItem2);
    _fppqItem->puIndices().push_back(0);
    _fppqItem->puIndices().push_back(0);
    _fppqItem->farePath() = _farePath;

    _trx = PricingMockDataBuilder::getPricingTrx();
    Itin* itin = addItin();
    _trx->itin().push_back(itin);
    _farePath->itin() = itin;

    PUPath* puPath = PricingMockDataBuilder::getPUPath(*_trx);
    PricingMockDataBuilder::addRTPUToMainTrip(
        *_trx, *puPath, itin->fareMarket()[0], itin->fareMarket()[3]);
    PricingMockDataBuilder::addRTPUToMainTrip(
        *_trx, *puPath, itin->fareMarket()[1], itin->fareMarket()[2]);
    puPath->eoePUAvailable().push_back(true);
    puPath->eoePUAvailable().push_back(true);
    _factory->puPath() = puPath;
    _factory->_trx = _trx;

    _org = new Loc();
    _org->loc() = "SYD";
    _dest = new Loc();
    _dest->loc() = "AKL";

    _fareInfo = new FareInfo();
    _fareInfo->directionality() = BOTH;
    _fareInfo->market1() = "SYD";
    _fareInfo->market1() = "AKL";

    _fare = new Fare();
    _fare->initialize(
        Fare::FareState::FS_Domestic, _fareInfo, *(itin->fareMarket()[0]), nullptr, nullptr);

    _ptf11->setFare(_fare);
    _ptf12->setFare(_fare);
    _ptf21->setFare(_fare);
    _ptf22->setFare(_fare);
  }



  Itin* addItin()
  {
    Itin* itin = PricingMockDataBuilder::addItin(*_trx);

    // DFW-AA-NYC-BA-LON-BA-NYC-AA-DFW
    Loc* loc1 = PricingMockDataBuilder::getLoc(*_trx, "DFW");
    Loc* loc2 = PricingMockDataBuilder::getLoc(*_trx, "NYC");
    Loc* loc3 = PricingMockDataBuilder::getLoc(*_trx, "LON");

    TravelSeg* tSeg1 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "AA", loc1, loc2, 1);
    TravelSeg* tSeg2 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "BA", loc2, loc3, 2);

    TravelSeg* tSeg3 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "BA", loc3, loc2, 3);
    TravelSeg* tSeg4 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "AA", loc2, loc1, 4);

    FareMarket* fm1 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "AA", loc1, loc2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg1, *fm1);

    FareMarket* fm2 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "BA", loc2, loc3);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg2, *fm2);

    FareMarket* fm3 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "BA", loc3, loc2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg3, *fm3);

    FareMarket* fm4 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "AA", loc2, loc1);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg1, *fm1);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg2, *fm2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg3, *fm3);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg4, *fm4);

    return itin;
  }

  void tearDown()
  {
    _memHandle.clear();
    delete _factory;
    delete _ptf11;
    delete _ptf12;
    delete _ptf21;
    delete _ptf22;
    delete _fu11;
    delete _fu12;
    delete _fu21;
    delete _fu22;
    delete _pu1;
    delete _pu2;
    delete _pupqItem1;
    delete _pupqItem2;
    delete _fppqItem;
    delete _fare;
    delete _fareInfo;
    delete _org;
    delete _dest;
    delete _trx;
  }

  void prepareFarePath(FarePath& fp, const FareType& ft1, const FareType& ft2)
  {
    DataHandle& dataHandle = _factory->trx()->dataHandle();

    PaxTypeFare* fare1, *fare2;
    dataHandle.get(fare1);
    dataHandle.get(fare2);

    FareClassAppInfo* info1, *info2;
    dataHandle.get(info1);
    dataHandle.get(info2);
    info1->_fareType = ft1;
    info2->_fareType = ft2;

    (FareClassAppInfo*&)(fare1->fareClassAppInfo()) = info1;
    (FareClassAppInfo*&)(fare2->fareClassAppInfo()) = info2;

    FareUsage* fu1, *fu2;
    dataHandle.get(fu1);
    dataHandle.get(fu2);

    fu1->paxTypeFare() = fare1;
    fu2->paxTypeFare() = fare2;

    PricingUnit* pu1, *pu2;
    dataHandle.get(pu1);
    dataHandle.get(pu2);

    pu1->fareUsage().push_back(fu1);
    pu2->fareUsage().push_back(fu2);

    fp.pricingUnit().push_back(pu1);
    fp.pricingUnit().push_back(pu2);
  }

  void addFareType(NoPNRPricingTrx::FareTypes::FTGroup grp, FareType& ft)
  {
    NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(_factory->trx());
    NoPNRPricingTrx::FareTypes& fareTypes = noPNRTrx->fareTypes();

    //    DataHandle& dataHandle = noPNRTrx->dataHandle();

    fareTypes._fareTypes.insert(
        std::pair<FareType, NoPNRPricingTrx::FareTypes::FareTypeGroup>(ft, grp));
  }

  void testFindPUFactoryIdx()
  {
    DiagCollector diag;

    // FareUsage does not exists in the FarePath
    {
      FareUsage* failedSourceFareUsage = new FareUsage;
      FareUsage* failedTargetFareUsage = new FareUsage;
      int32_t idx1 = -1;
      int32_t idx2 = -1;
      farepathutils::findPUFactoryIdx(
          *_farePath, failedSourceFareUsage, failedTargetFareUsage, idx1, idx2);
          CPPUNIT_ASSERT_EQUAL(idx1, -1);
      CPPUNIT_ASSERT_EQUAL(idx2, -1);
      delete failedSourceFareUsage;
      delete failedTargetFareUsage;
    }

    // FareUsage exists in the FarePath
    {
      int32_t idx1 = -1;
      int32_t idx2 = -1;
      FareUsage* failedSourceFareUsage = _fu11;
      FareUsage* failedTargetFareUsage = _fu22;

      farepathutils::findPUFactoryIdx(
          *_farePath, failedSourceFareUsage, failedTargetFareUsage, idx1, idx2);
      CPPUNIT_ASSERT_EQUAL(0, idx1);
      CPPUNIT_ASSERT_EQUAL(1, idx2);
    }

    // Only one FareUsage exists in the FarePath
    {

      int32_t idx1 = -1;
      int32_t idx2 = -1;
      FareUsage* failedSourceFareUsage = new FareUsage;
      FareUsage* failedTargetFareUsage = _fu22;

      farepathutils::findPUFactoryIdx(
          *_farePath, failedSourceFareUsage, failedTargetFareUsage, idx1, idx2);
      CPPUNIT_ASSERT_EQUAL(-1, idx1);
      CPPUNIT_ASSERT_EQUAL(1, idx2);
      delete failedSourceFareUsage;
    }

    // Only one FareUsage exists in the FarePath
    {

      int32_t idx1 = -1;
      int32_t idx2 = -1;
      FareUsage* failedSourceFareUsage = new FareUsage;
      FareUsage* failedTargetFareUsage = _fu11;

      farepathutils::findPUFactoryIdx(
          *_farePath, failedSourceFareUsage, failedTargetFareUsage, idx1, idx2);
      CPPUNIT_ASSERT_EQUAL(-1, idx1);
      CPPUNIT_ASSERT_EQUAL(0, idx2);
      delete failedSourceFareUsage;
    }
  }

  void testFailedFareExistsInPU()
  {
    bool exists = farepathutils::failedFareExistsInPU(_ptf21, *_pu2);
    CPPUNIT_ASSERT_EQUAL(true, exists);

    exists = farepathutils::failedFareExistsInPU(_ptf11, *_pu2);
    CPPUNIT_ASSERT_EQUAL(false, exists);
  }

  void testRecalculatePriority()
  {
    PUPQItem* pupqItem = 0;
    _trx->dataHandle().get(pupqItem);
    PricingUnit* pu = 0;
    _trx->dataHandle().get(pu);
    pupqItem->pricingUnit() = pu;
    pupqItem->mutablePriorityStatus().setFarePriority(PRIORITY_LOW);

    CPPUNIT_ASSERT_EQUAL(DEFAULT_PRIORITY, _fppqItem->priorityStatus().farePriority());

    PUPQItem* prevPUPQItem = _fppqItem->pupqItemVect()[0];
    _fppqItem->pupqItemVect()[0] = pupqItem;
    _factory->recalculatePriority(*_fppqItem);

    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, _fppqItem->priorityStatus().farePriority());
    _fppqItem->pupqItemVect()[0] = prevPUPQItem;
  }

  void testReplaceWithNewPU()
  {
    PUPQItem* pupqItem = 0;
    _trx->dataHandle().get(pupqItem);
    PricingUnit* pu = 0;
    _trx->dataHandle().get(pu);
    pupqItem->pricingUnit() = pu;
    pupqItem->mutablePriorityStatus().setFarePriority(PRIORITY_LOW);

    _factory->replaceWithNewPU(*_fppqItem, pupqItem, 0, 1);

    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, _fppqItem->priorityStatus().farePriority());
    CPPUNIT_ASSERT_EQUAL(pupqItem, _fppqItem->pupqItemVect()[0]);
  }

  void testFppqPriorityIsSameAsPupqPriorityWhenSamePriority()
  {
    _pupqItem1->mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
    _fppqItem->mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
    farepathutils::setPriority(*_trx, *_pupqItem1, *_fppqItem, _fppqItem->farePath()->itin());
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, _fppqItem->priorityStatus().negotiatedFarePriority());
  }

  void testFppqPriorityChangesToLowerWhenPupqPriorityHasLowerPriority()
  {
    _pupqItem1->mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
    _fppqItem->mutablePriorityStatus().setNegotiatedFarePriority(DEFAULT_PRIORITY);
    farepathutils::setPriority(*_trx, *_pupqItem1, *_fppqItem, _fppqItem->farePath()->itin());
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, _fppqItem->priorityStatus().negotiatedFarePriority());
  }

  void testFppqPriorityNoChangeWhenPupqPriorityHasHigherPriority()
  {
    _pupqItem1->mutablePriorityStatus().setNegotiatedFarePriority(DEFAULT_PRIORITY);
    _fppqItem->mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
    farepathutils::setPriority(*_trx, *_pupqItem1, *_fppqItem, _fppqItem->farePath()->itin());
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, _fppqItem->priorityStatus().negotiatedFarePriority());
  }

  void testHigherPriorityIsOnTopWhenHigherPriorityFppqItemIsPushedLast()
  {
    FPPQItem fppqItem1, fppqItem2, fppqItem3;
    FarePath farePath1, farePath2, farePath3;
    createFppqItem(fppqItem1, farePath1, PRIORITY_LOW);
    createFppqItem(fppqItem2, farePath2, PRIORITY_LOW);
    createFppqItem(fppqItem3, farePath3, DEFAULT_PRIORITY);
    _factoriesConfig.setSearchAlwaysLowToHigh(true);
    _factory->pqPush(&fppqItem1);
    _factory->pqPush(&fppqItem2);
    _factory->pqPush(&fppqItem3);
    assertPopFromPriorityStack(DEFAULT_PRIORITY);
    assertPopFromPriorityStack(PRIORITY_LOW);
    assertPopFromPriorityStack(PRIORITY_LOW);
  }

  void testHigherPriorityIsOnTopWhenHigherPriorityFppqItemIsPushedFirst()
  {
    FPPQItem fppqItem1, fppqItem2;
    FarePath farePath1, farePath2;
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    createFppqItem(fppqItem2, farePath2, PRIORITY_LOW);
    _factoriesConfig.setSearchAlwaysLowToHigh(true);
    _factory->pqPush(&fppqItem1);
    _factory->pqPush(&fppqItem2);
    assertPopFromPriorityStack(DEFAULT_PRIORITY);
    assertPopFromPriorityStack(PRIORITY_LOW);
  }

  void testGetFareUsageReturn0WhenTvlSegNotFound()
  {
    putTravelSegsInFareUsages();
    FareUsage* expectedFareUsage = 0;
    FareUsage* actualFareUsage = 0;
    uint16_t expectedTvlSegIndex = 0;
    uint16_t actualTvlSegIndex = 0;
    AirSeg airSeg;
    actualFareUsage = farepathutils::getFareUsage(*_farePath, &airSeg, actualTvlSegIndex);
    CPPUNIT_ASSERT_EQUAL(expectedFareUsage, actualFareUsage);
    CPPUNIT_ASSERT_EQUAL(expectedTvlSegIndex, actualTvlSegIndex);
  }

  void testGetFareUsageReturnFirstFareUsageForFirstTravelSeg()
  {
    putTravelSegsInFareUsages();
    FareUsage* expectedFareUsage = _fu11;
    FareUsage* actualFareUsage;
    uint16_t expectedTvlSegIndex = 0;
    uint16_t actualTvlSegIndex = 0;
    actualFareUsage =
        farepathutils::getFareUsage(*_farePath, _trx->itin()[0]->travelSeg()[0], actualTvlSegIndex);
    CPPUNIT_ASSERT_EQUAL(expectedFareUsage, actualFareUsage);
    CPPUNIT_ASSERT_EQUAL(expectedTvlSegIndex, actualTvlSegIndex);
  }

  void testGetFareUsageReturnSecondFareUsageForSecondTravelSeg()
  {
    putTravelSegsInFareUsages();
    FareUsage* expectedFareUsage = _fu12;
    FareUsage* actualFareUsage;
    uint16_t expectedTvlSegIndex = 0;
    uint16_t actualTvlSegIndex = 0;
    actualFareUsage =
        farepathutils::getFareUsage(*_farePath, _trx->itin()[0]->travelSeg()[1], actualTvlSegIndex);
    CPPUNIT_ASSERT_EQUAL(expectedFareUsage, actualFareUsage);
    CPPUNIT_ASSERT_EQUAL(expectedTvlSegIndex, actualTvlSegIndex);
  }

  void testGetFareUsageReturnThirdFareUsageForThirdTravelSeg()
  {
    putTravelSegsInFareUsages();
    FareUsage* expectedFareUsage = _fu21;
    FareUsage* actualFareUsage;
    uint16_t expectedTvlSegIndex = 0;
    uint16_t actualTvlSegIndex = 0;
    actualFareUsage =
        farepathutils::getFareUsage(*_farePath, _trx->itin()[0]->travelSeg()[2], actualTvlSegIndex);
    CPPUNIT_ASSERT_EQUAL(expectedFareUsage, actualFareUsage);
    CPPUNIT_ASSERT_EQUAL(expectedTvlSegIndex, actualTvlSegIndex);
  }

  void testDifferentialDataReturnZeroWhenFuHasNoDiffPlusUps()
  {
    putTravelSegsInFareUsages();
    DifferentialData* expectedDiff = 0;
    CPPUNIT_ASSERT_EQUAL(expectedDiff,
                         farepathutils::differentialData(_fu11, _trx->itin()[0]->travelSeg()[0]));
  }

  void testDifferentialDataReturnZeroIfDiffDataFail()
  {
    putTravelSegsInFareUsages();
    putDifferentialDataInFu(_fu11);
    _fu11->differentialPlusUp()[0]->status() = DifferentialData::SC_FAILED;
    DifferentialData* expectedDiff = 0;
    CPPUNIT_ASSERT_EQUAL(expectedDiff,
                         farepathutils::differentialData(_fu11, _trx->itin()[0]->travelSeg()[0]));
  }

  void testDifferentialDataReturnZeroIfTvlSegNotFound()
  {
    putTravelSegsInFareUsages();
    putDifferentialDataInFu(_fu11);
    DifferentialData* expectedDiff = 0;
    CPPUNIT_ASSERT_EQUAL(expectedDiff,
                         farepathutils::differentialData(_fu11, _trx->itin()[0]->travelSeg()[1]));
  }

  void testDifferentialDataReturnDiffDataFromFu()
  {
    putTravelSegsInFareUsages();
    putDifferentialDataInFu(_fu11);
    DifferentialData* expectedDiff = _fu11->differentialPlusUp()[0];
    CPPUNIT_ASSERT_EQUAL(expectedDiff,
                         farepathutils::differentialData(_fu11, _trx->itin()[0]->travelSeg()[0]));
  }

  FareMarket::RetrievalInfo* createRetrievalInfo(FareMarket::FareRetrievalFlags flag)
  {
    FareMarket::RetrievalInfo* info = _memHandle.insert(new FareMarket::RetrievalInfo);
    info->_flag = flag;
    return info;
  }
  void setUpAllRetrievalInfos(FareMarket::FareRetrievalFlags flag)
  {
    RefundPricingTrx* rtrx = _memHandle.insert(new RefundPricingTrx);
    rtrx->trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    _factory->_trx = rtrx;

    _ptf11->retrievalInfo() = createRetrievalInfo(flag);
    _ptf12->retrievalInfo() = createRetrievalInfo(flag);
    _ptf21->retrievalInfo() = createRetrievalInfo(flag);
    _ptf22->retrievalInfo() = createRetrievalInfo(flag);
  }

  AirSeg* makeAirSeg(const LocCode& boardMultiCity,
                     const LocCode& offMultiCity,
                     const CarrierCode& carrier,
                     LocCode origAirport = "",
                     LocCode destAirport = "")
  {
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc(boardMultiCity, DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc(offMultiCity, DateTime::localTime());

    if ("" == origAirport)
      origAirport = boardMultiCity;
    if ("" == destAirport)
      destAirport = offMultiCity;

    AirSeg* as = _memHandle.create<AirSeg>();
    as->origin() = const_cast<Loc*>(loc1);
    as->destination() = const_cast<Loc*>(loc2);
    as->boardMultiCity() = boardMultiCity;
    as->offMultiCity() = offMultiCity;
    as->origAirport() = origAirport;
    as->destAirport() = destAirport;
    as->carrier() = carrier;
    return as;
  }

  void testCheckPremiumMarriageA()
  {
    AirSeg* prevAirSeg = makeAirSeg("KRK", "LON", "AA");

    CPPUNIT_ASSERT(BookingCodeUtil::premiumMarriage(*_trx, *prevAirSeg, "A3", "Z3"));
  }

  void testCheckPremiumMarriageA2()
  {
    AirSeg* prevAirSeg = makeAirSeg("AAA", "NYC", "AA");

    CPPUNIT_ASSERT(!BookingCodeUtil::premiumMarriage(*_trx, *prevAirSeg, "A3", "Z3"));
  }

  void testCheckPremiumMarriageB()
  {
    AirSeg* prevAirSeg = makeAirSeg("BBB", "KRK", "AA");

    CPPUNIT_ASSERT(BookingCodeUtil::premiumMarriage(*_trx, *prevAirSeg, "B3", "Z6"));
  }

  void testCheckPremiumMarriageC()
  {
    AirSeg* prevAirSeg = makeAirSeg("KRK", "BBB", "AA");

    CPPUNIT_ASSERT(BookingCodeUtil::premiumMarriage(*_trx, *prevAirSeg, "C3", "Z6"));
  }

  void testCheckPremiumMarriageD()
  {
    AirSeg* prevAirSeg = makeAirSeg("BBB", "KRK", "AA");

    CPPUNIT_ASSERT(BookingCodeUtil::premiumMarriage(*_trx, *prevAirSeg, "D3", "Z6"));
  }

  void testCheckPremiumMarriageE()
  {
    AirSeg* prevAirSeg = makeAirSeg("BBB", "KRK", "AA");

    CPPUNIT_ASSERT(BookingCodeUtil::premiumMarriage(*_trx, *prevAirSeg, "E3", "Z6"));
  }

  void testCheckPremiumMarriageF()
  {
    AirSeg* prevAirSeg = makeAirSeg("BBB", "KRK", "AA");

    CPPUNIT_ASSERT(BookingCodeUtil::premiumMarriage(*_trx, *prevAirSeg, "F3", "Z9"));
  }

  /*
   *  bool
      5608                 : FarePathFactory::foundFlow(FarePath& fpath,
      5609                 :                            TravelSeg* startTvlseg,
      5610               0 :                            uint16_t& flowLen)
   *
   */
  FareMarket*
  makeFareMarket(AirSeg* as1, AirSeg* as2 = NULL, AirSeg* as3 = NULL, AirSeg* as4 = NULL)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg().push_back(as1);
    if (NULL != as2)
      fm->travelSeg().push_back(as2);
    if (NULL != as3)
      fm->travelSeg().push_back(as3);
    if (NULL != as4)
      fm->travelSeg().push_back(as4);

    return fm;
  }

  Itin*
  generateItinTravelSeg(AirSeg* as1, AirSeg* as2 = NULL, AirSeg* as3 = NULL, AirSeg* as4 = NULL)
  {
    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(as1);
    if (NULL != as2)
      itin->travelSeg().push_back(as2);
    if (NULL != as3)
      itin->travelSeg().push_back(as3);
    if (NULL != as4)
      itin->travelSeg().push_back(as4);

    return itin;
  }

  void testRemoveTaggedValCxr()
  {
    CarrierCode defValCxr("EK");
    CarrierCode set1 [] = {"AA", "AB", "DL", "EK", "BA"};
    std::vector<CarrierCode> valCxr1(&set1[0], &set1[0]+5);

    // Primary FarePath
    FarePath fp1;
    fp1.setTotalNUCAmount(100);
    fp1.validatingCarriers()=valCxr1;

    // Tagged FarePath 1
    FarePath taggedFp1;
    taggedFp1.setTotalNUCAmount(100);
    CarrierCode set2 [] = {"AB", "DL"};
    std::vector<CarrierCode> valCxr2(&set2[0], &set2[0]+2);
    taggedFp1.validatingCarriers()=valCxr2;

    // Tagged FarePath 2
    FarePath taggedFp2;
    taggedFp2.setTotalNUCAmount(100);
    CarrierCode set3 [] = {"EK", "BA"};
    std::vector<CarrierCode> valCxr3(&set3[0], &set3[0]+2);
    taggedFp2.validatingCarriers()=valCxr3;

    fp1.gsaClonedFarePaths().push_back(&taggedFp1);
    fp1.gsaClonedFarePaths().push_back(&taggedFp2);

    ValidatingCxrUtil::removeTaggedValCxr(fp1, fp1.gsaClonedFarePaths(), *_trx);
    CPPUNIT_ASSERT(fp1.validatingCarriers().size()==1);
    CPPUNIT_ASSERT(fp1.validatingCarriers().front()==std::string("AA"));
  }

  void testGetTaggedFarePathWithValCxr()
  {
    CarrierCode defValCxr("EK");
    CarrierCode set1 [] = {"AA", "AB", "DL", "EK", "BA"};
    std::vector<CarrierCode> valCxr1(&set1[0], &set1[0]+5);

    // Primary FarePath
    FarePath fp1;
    fp1.setTotalNUCAmount(100);
    fp1.validatingCarriers()=valCxr1;

    // Tagged FarePath 1
    FarePath taggedFp1;
    taggedFp1.setTotalNUCAmount(100);
    CarrierCode set2 [] = {"AB", "DL"};
    std::vector<CarrierCode> valCxr2(&set2[0], &set2[0]+2);
    taggedFp1.validatingCarriers()=valCxr2;

    // Tagged FarePath 2
    FarePath taggedFp2;
    taggedFp2.setTotalNUCAmount(100);
    CarrierCode set3 [] = {"EK", "BA"};
    std::vector<CarrierCode> valCxr3(&set3[0], &set3[0]+2);
    taggedFp2.validatingCarriers()=valCxr3;

    fp1.gsaClonedFarePaths().push_back(&taggedFp1);
    fp1.gsaClonedFarePaths().push_back(&taggedFp2);

    FarePath* primaryFp = ValidatingCxrUtil::getTaggedFarePathWithValCxr(defValCxr, fp1);
    CPPUNIT_ASSERT(primaryFp != 0);
    CPPUNIT_ASSERT(primaryFp == &taggedFp2);
    CPPUNIT_ASSERT(primaryFp->validatingCarriers().front() == defValCxr);
  }

  void setFareUsageAmount(FareUsage& fu, MoneyAmount amt)
  {
    fu.surchargeAmt()=amt;
    fu.surchargeAmtUnconverted()=amt;
    fu.transferAmt()=amt;
    fu.transferAmtUnconverted()=amt;
    fu.stopOverAmt()=amt;
    fu.stopOverAmtUnconverted()=amt;
    fu.absorptionAdjustment()=amt;
    fu.differentialAmt()=amt;
    fu.minFarePlusUpAmt()=amt;
    fu.netCat35NucAmount()=amt;
    fu.setDiscAmount(amt);
  }

  void setFppqItem(FPPQItem& fppqItem, MoneyAmount amt1, MoneyAmount amt2)
  {
    PricingUnit* pu = _memHandle.create<PricingUnit>();

    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf2 = _memHandle.create<PaxTypeFare>();
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    setFareUsageAmount(*fu1, amt1);
    setFareUsageAmount(*fu2, amt2);

    pu->fareUsage().push_back(fu1);
    pu->fareUsage().push_back(fu2);

    fppqItem.farePath()->pricingUnit().push_back(pu);
  }

  PaxType* createPaxType(PaxTypeCode code)
  {
    PaxType* pt = _memHandle.create<PaxType>();
    pt->paxType() = code;
    //pt->paxTypeInfo() = _memHandle.create<PaxTypeInfo>();
    return pt;
  }

  // Total is same but components are different
  void testMergeFarePathWithEqualComponents_NoMergeDiffComponents()
  {
    //CarrierCode set1 [] = {"AA", "AB", "DL", "EK", "BA"};

    FPPQItem fppqItem1, fppqItem2, fppqItem3;
    FarePath farePath1, farePath2, farePath3;

    //FppqItem 1
    farePath1.validatingCarriers().push_back("AA");
    farePath1.validatingCarriers().push_back("AB");
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    setFppqItem(fppqItem1, 5.00, 5.00);

    //FppqItem 2
    farePath2.validatingCarriers().push_back("DL");
    farePath2.validatingCarriers().push_back("EK");
    createFppqItem(fppqItem2, farePath2, DEFAULT_PRIORITY);
    setFppqItem(fppqItem2, 10.00, 10.00);

    //FppqItem 3
    farePath3.validatingCarriers().push_back("BA");
    createFppqItem(fppqItem3, farePath3, DEFAULT_PRIORITY);
    setFppqItem(fppqItem3, 15.00, 15.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(&fppqItem1);
    clonedFPPQItems.push_back(&fppqItem2);
    clonedFPPQItems.push_back(&fppqItem3);

    size_t size = clonedFPPQItems.size();
    ValidatingCxrUtil::mergeFarePathWithEqualComponents(clonedFPPQItems, _factory->_storage);
    CPPUNIT_ASSERT(size==clonedFPPQItems.size());
  }

  // Total is different but components are same
  void testMergeFarePathWithEqualComponents_NoMergeDiffTotal()
  {
    FPPQItem fppqItem1, fppqItem2, fppqItem3;
    FarePath farePath1, farePath2, farePath3;

    //FppqItem 1
    farePath1.validatingCarriers().push_back("AA");
    farePath1.validatingCarriers().push_back("AB");
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(10);
    setFppqItem(fppqItem1, 10.00, 10.00);

    //FppqItem 2
    farePath2.validatingCarriers().push_back("DL");
    farePath2.validatingCarriers().push_back("EK");
    createFppqItem(fppqItem2, farePath2, DEFAULT_PRIORITY);
    farePath2.setTotalNUCAmount(50);
    setFppqItem(fppqItem2, 10.00, 10.00);

    //FppqItem 3
    farePath3.validatingCarriers().push_back("BA");
    createFppqItem(fppqItem3, farePath3, DEFAULT_PRIORITY);
    farePath3.setTotalNUCAmount(100);
    setFppqItem(fppqItem3, 10.00, 10.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(&fppqItem1);
    clonedFPPQItems.push_back(&fppqItem2);
    clonedFPPQItems.push_back(&fppqItem3);

    size_t size = clonedFPPQItems.size();
    ValidatingCxrUtil::mergeFarePathWithEqualComponents(clonedFPPQItems, _factory->_storage);
    CPPUNIT_ASSERT(size==clonedFPPQItems.size());
  }

  void testMergeFarePathWithEqualComponents_MergeAll()
  {
    FPPQItem* fppqItem1 = _factory->constructFPPQItem();
    FPPQItem* fppqItem2 = _factory->constructFPPQItem();
    FPPQItem* fppqItem3 = _factory->constructFPPQItem();

    FarePath* farePath1 = _factory->constructFarePath();
    FarePath* farePath2 = _factory->constructFarePath();
    FarePath* farePath3 = _factory->constructFarePath();

    //FppqItem 1
    farePath1->validatingCarriers().push_back("AA");
    farePath1->validatingCarriers().push_back("AB");
    createFppqItem(*fppqItem1, *farePath1, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem1, 10.00, 10.00);

    //FppqItem 2
    farePath2->validatingCarriers().push_back("DL");
    farePath2->validatingCarriers().push_back("EK");
    createFppqItem(*fppqItem2, *farePath2, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem2, 10.00, 10.00);

    //FppqItem 3
    farePath3->validatingCarriers().push_back("BA");
    createFppqItem(*fppqItem3, *farePath3, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem3, 10.00, 10.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(fppqItem1);
    clonedFPPQItems.push_back(fppqItem2);
    clonedFPPQItems.push_back(fppqItem3);

    ValidatingCxrUtil::mergeFarePathWithEqualComponents(clonedFPPQItems, _factory->_storage);

    CPPUNIT_ASSERT(1==clonedFPPQItems.size());
    CPPUNIT_ASSERT(5==farePath1->validatingCarriers().size());
    CPPUNIT_ASSERT(farePath1->validatingCarriers()[0]=="AA");
    CPPUNIT_ASSERT(farePath1->validatingCarriers()[1]=="AB");
    CPPUNIT_ASSERT(farePath1->validatingCarriers()[2]=="DL");
    CPPUNIT_ASSERT(farePath1->validatingCarriers()[3]=="EK");
    CPPUNIT_ASSERT(farePath1->validatingCarriers()[4]=="BA");
  }

  void testMergeFarePathWithEqualComponents_MergeFew()
  {
    FPPQItem* fppqItem1 = _factory->constructFPPQItem();
    FPPQItem* fppqItem2 = _factory->constructFPPQItem();
    FPPQItem* fppqItem3 = _factory->constructFPPQItem();

    FarePath* farePath1 = _factory->constructFarePath();
    FarePath* farePath2 = _factory->constructFarePath();
    FarePath* farePath3 = _factory->constructFarePath();

    //FppqItem 1
    farePath1->validatingCarriers().push_back("AA");
    farePath1->validatingCarriers().push_back("AB");
    createFppqItem(*fppqItem1, *farePath1, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem1, 5.00, 5.00);

    //FppqItem 2
    farePath2->validatingCarriers().push_back("DL");
    farePath2->validatingCarriers().push_back("EK");
    createFppqItem(*fppqItem2, *farePath2, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem2, 10.00, 10.00);

    //FppqItem 3
    farePath3->validatingCarriers().push_back("BA");
    createFppqItem(*fppqItem3, *farePath3, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem3, 5.00, 5.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(fppqItem1);
    clonedFPPQItems.push_back(fppqItem2);
    clonedFPPQItems.push_back(fppqItem3);

    ValidatingCxrUtil::mergeFarePathWithEqualComponents(clonedFPPQItems, _factory->_storage);

    CPPUNIT_ASSERT(2==clonedFPPQItems.size());
    CPPUNIT_ASSERT(3==farePath1->validatingCarriers().size());
    CPPUNIT_ASSERT(farePath1->validatingCarriers()[0]=="AA");
    CPPUNIT_ASSERT(farePath1->validatingCarriers()[1]=="AB");
    CPPUNIT_ASSERT(farePath1->validatingCarriers()[2]=="BA");
  }

  // All item with same total amount
  void testFindLowestFaerPath_SameAmount()
  {
    FPPQItem* fppqItem1 = _factory->constructFPPQItem();
    FPPQItem* fppqItem2 = _factory->constructFPPQItem();
    FPPQItem* fppqItem3 = _factory->constructFPPQItem();

    FarePath* farePath1 = _factory->constructFarePath();
    FarePath* farePath2 = _factory->constructFarePath();
    FarePath* farePath3 = _factory->constructFarePath();

    //FppqItem 1
    createFppqItem(*fppqItem1, *farePath1, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem1, 5.00, 5.00);

    //FppqItem 2
    createFppqItem(*fppqItem2, *farePath2, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem2, 10.00, 10.00);

    //FppqItem 3
    createFppqItem(*fppqItem3, *farePath3, DEFAULT_PRIORITY);
    setFppqItem(*fppqItem3, 15.00, 15.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(fppqItem1);
    clonedFPPQItems.push_back(fppqItem2);
    clonedFPPQItems.push_back(fppqItem3);

    FPPQItem* fppqItem = ValidatingCxrUtil::findLowestFarePath(clonedFPPQItems);

    CPPUNIT_ASSERT(fppqItem == 0 || fppqItem == clonedFPPQItems.front());
  }

  // Diff total amount, Last Lowest
  void testFindLowestFaerPath_DiffAmountLastLowest()
  {
    FPPQItem* fppqItem1 = _factory->constructFPPQItem();
    FPPQItem* fppqItem2 = _factory->constructFPPQItem();
    FPPQItem* fppqItem3 = _factory->constructFPPQItem();

    FarePath* farePath1 = _factory->constructFarePath();
    FarePath* farePath2 = _factory->constructFarePath();
    FarePath* farePath3 = _factory->constructFarePath();

    //FppqItem 1
    createFppqItem(*fppqItem1, *farePath1, DEFAULT_PRIORITY);
    farePath1->setTotalNUCAmount(100);
    setFppqItem(*fppqItem1, 5.00, 5.00);

    //FppqItem 2
    createFppqItem(*fppqItem2, *farePath2, DEFAULT_PRIORITY);
    farePath2->setTotalNUCAmount(50);
    setFppqItem(*fppqItem2, 10.00, 10.00);

    //FppqItem 3
    createFppqItem(*fppqItem3, *farePath3, DEFAULT_PRIORITY);
    farePath2->setTotalNUCAmount(10);
    setFppqItem(*fppqItem3, 15.00, 15.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(fppqItem1);
    clonedFPPQItems.push_back(fppqItem2);
    clonedFPPQItems.push_back(fppqItem3);

    FPPQItem* fppqItem = ValidatingCxrUtil::findLowestFarePath(clonedFPPQItems);
    CPPUNIT_ASSERT(fppqItem != 0 || fppqItem == clonedFPPQItems.back());
  }

  void testFindLowestFaerPath_DiffAmountMiddleLowest()
  {
    FPPQItem fppqItem1, fppqItem2, fppqItem3;
    FarePath farePath1, farePath2, farePath3;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(100);
    setFppqItem(fppqItem1, 5.00, 5.00);

    //FppqItem 2
    createFppqItem(fppqItem2, farePath2, DEFAULT_PRIORITY);
    farePath2.setTotalNUCAmount(5);
    setFppqItem(fppqItem2, 10.00, 10.00);

    //FppqItem 3
    createFppqItem(fppqItem3, farePath3, DEFAULT_PRIORITY);
    farePath2.setTotalNUCAmount(10);
    setFppqItem(fppqItem3, 15.00, 15.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(&fppqItem1);
    clonedFPPQItems.push_back(&fppqItem2);
    clonedFPPQItems.push_back(&fppqItem3);

    FPPQItem* fppqItem = ValidatingCxrUtil::findLowestFarePath(clonedFPPQItems);

    CPPUNIT_ASSERT(fppqItem != 0 || fppqItem == &fppqItem2);
  }

  void testProcessFarePathClones_FPPQItemNotValid()
  {
    FPPQItem fppqItem1, fppqItem2;
    FarePath farePath1, farePath2;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(100);
    fppqItem1.isValid()=false;
    setFppqItem(fppqItem1, 5.00, 5.00);

    //FppqItem 2
    createFppqItem(fppqItem2, farePath2, DEFAULT_PRIORITY);
    farePath2.setTotalNUCAmount(5);
    setFppqItem(fppqItem2, 10.00, 10.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(&fppqItem2);
    FPPQItem* fppqItem = &fppqItem1;
    _factory->processFarePathClones(fppqItem, clonedFPPQItems);

    CPPUNIT_ASSERT(fppqItem == &fppqItem2);
  }

  void testProcessFarePathClones_FPPQItemValidLower()
  {
    FPPQItem fppqItem1, fppqItem2;
    FarePath farePath1, farePath2;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(50);
    setFppqItem(fppqItem1, 10.00, 10.00);

    //FppqItem 2
    createFppqItem(fppqItem2, farePath2, DEFAULT_PRIORITY);
    farePath2.setTotalNUCAmount(100);
    setFppqItem(fppqItem2, 10.00, 10.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(&fppqItem2);
    FPPQItem *fppqItem = &fppqItem1;
    _factory->processFarePathClones(fppqItem, clonedFPPQItems);

    CPPUNIT_ASSERT(fppqItem == &fppqItem1);
  }

  void testProcessFarePathClones_FPPQItemValidHigher()
  {
    FPPQItem fppqItem1, fppqItem2;
    FarePath farePath1, farePath2;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(100);
    setFppqItem(fppqItem1, 10.00, 10.00);

    //FppqItem 2
    createFppqItem(fppqItem2, farePath2, DEFAULT_PRIORITY);
    farePath2.setTotalNUCAmount(50);
    setFppqItem(fppqItem2, 10.00, 10.00);

    std::list<FPPQItem*> clonedFPPQItems;
    clonedFPPQItems.push_back(&fppqItem2);
    FPPQItem *fppqItem = &fppqItem1;
    _factory->processFarePathClones(fppqItem, clonedFPPQItems);

    CPPUNIT_ASSERT(fppqItem == &fppqItem2);
  }

  void testIsFarePathValidForCorpIDFare_NoCorpIDFare()
  {
    FPPQItem fppqItem1;
    FarePath farePath1;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(100);
    setFppqItem(fppqItem1, 500.00, 500.00);

    //Set both fare usage with no Corp ID Fare
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->setMatchedCorpID(false);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->setMatchedCorpID(false);

    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(false, 2);
    CPPUNIT_ASSERT_EQUAL(false, _factory->isFarePathValidForCorpIDFare(fppqItem1, 2));//XC OFF

    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(true, 2);
    CPPUNIT_ASSERT_EQUAL(false, _factory->isFarePathValidForCorpIDFare(fppqItem1, 2));//XC ON
  }

  void testIsFarePathValidForCorpIDFare_OneCorpIDFare()
  {
    FPPQItem fppqItem1;
    FarePath farePath1;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(100);
    setFppqItem(fppqItem1, 500.00, 500.00);

    //Set both fare usage with 1 Corp ID Fare
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->setMatchedCorpID(true);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->setMatchedCorpID(false);

    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(false, 2);
    CPPUNIT_ASSERT_EQUAL(true, _factory->isFarePathValidForCorpIDFare(fppqItem1, 2));//XC OFF

    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(true, 2);
    CPPUNIT_ASSERT_EQUAL(false, _factory->isFarePathValidForCorpIDFare(fppqItem1, 2));//XC ON
  }

  void testIsFarePathValidForCorpIDFare_AllCorpIDFare()
  {
    FPPQItem fppqItem1;
    FarePath farePath1;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(100);
    setFppqItem(fppqItem1, 500.00, 500.00);

    //Set both fare usage with All Corp ID Fare
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->setMatchedCorpID(true);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->setMatchedCorpID(true);

    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(true, 2);
    CPPUNIT_ASSERT_EQUAL(true, _factory->isFarePathValidForCorpIDFare(fppqItem1, 2));//XC OFF

    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(true, 2);
    CPPUNIT_ASSERT_EQUAL(true, _factory->isFarePathValidForCorpIDFare(fppqItem1, 2));//XC ON
  }

  void testIsFarePathValidForFFG_XOFareON()
  {
    FPPQItem fppqItem1;
    FarePath farePath1;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(100);
    setFppqItem(fppqItem1, 500.00, 500.00);

    //Set the requested paxType = ADULT
    fppqItem1.farePath()->paxType() = createPaxType(ADULT);

    //Create a FFG (GroupID=2) with P20="T" (XO ON)
    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXOFares('T', 2);

    //Set both the fare usage paxType = ADULT
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->actualPaxType() = createPaxType(ADULT);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->actualPaxType() = createPaxType(ADULT);

    CPPUNIT_ASSERT_EQUAL(true, _factory->isFarePathValidForXOFare(fppqItem1, 2));

    //Set one fare usage with ADULT and the other to CHILD
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->actualPaxType() = createPaxType(ADULT);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->actualPaxType() = createPaxType(CHILD);
    CPPUNIT_ASSERT_EQUAL(false, _factory->isFarePathValidForXOFare(fppqItem1, 2));

    //Set one fare usage with CHILD and the other to INFANT
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->actualPaxType() = createPaxType(CHILD);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->actualPaxType() = createPaxType(INFANT);
    CPPUNIT_ASSERT_EQUAL(false, _factory->isFarePathValidForXOFare(fppqItem1, 2));
  }

  void testIsFarePathValidForFFG_XOFareOFF()
  {
    FPPQItem fppqItem1;
    FarePath farePath1;

    //FppqItem 1
    createFppqItem(fppqItem1, farePath1, DEFAULT_PRIORITY);
    farePath1.setTotalNUCAmount(100);
    setFppqItem(fppqItem1, 500.00, 500.00);

    //Set the requested paxType = ADULT
    fppqItem1.farePath()->paxType() = createPaxType(ADULT);

    //Create a FFG (GroupID=2) with P20="F" (XO OFF)
    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXOFares('F', 2);

    //Set both the fare usage paxType = ADULT
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->actualPaxType() = createPaxType(ADULT);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->actualPaxType() = createPaxType(ADULT);

    CPPUNIT_ASSERT_EQUAL(true, _factory->isFarePathValidForXOFare(fppqItem1, 2));

    //Set one fare usage with ADULT and the other to CHILD
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->actualPaxType() = createPaxType(ADULT);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->actualPaxType() = createPaxType(CHILD);
    CPPUNIT_ASSERT_EQUAL(true, _factory->isFarePathValidForXOFare(fppqItem1, 2));

    //Set one fare usage with CHILD and the other to INFANT
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->actualPaxType() = createPaxType(CHILD);
    fppqItem1.farePath()->pricingUnit()[0]->fareUsage()[1]->paxTypeFare()->actualPaxType() = createPaxType(INFANT);
    CPPUNIT_ASSERT_EQUAL(false, _factory->isFarePathValidForXOFare(fppqItem1, 2));
  }

  PricingUnitFactory* makePricingUnitFactory()
  {
    // return _memHandle.create<PricingUnitFactory>();
    return new PricingUnitFactory();
  }

  PricingUnit* makePricingUnit() { return new PricingUnit(); }

  /*
   *
   * bool
  FarePathFactory::checkEarlyEOECombinability(const bool initStage,
                                     FPPQItem& fppqItem,
                                           const uint16_t xPoint,
   */
  void testCheckEarlyEOECombinabilityA()
  {
    FPPQItem fppqItem;
    FarePath* _farePath = _memHandle.create<FarePath>();

    std::vector<PricingUnit*>* pricingUnit = _memHandle.create<std::vector<PricingUnit*> >();
    pricingUnit->push_back(makePricingUnit());
    pricingUnit->push_back(makePricingUnit());

    _farePath->pricingUnit() = *pricingUnit;

    fppqItem.farePath() = _farePath;

    DiagCollector diagIn;

    std::vector<PricingUnitFactory*>* puf = _memHandle.create<std::vector<PricingUnitFactory*> >();
    puf->push_back(makePricingUnitFactory());
    puf->push_back(makePricingUnitFactory());
    puf->push_back(makePricingUnitFactory());
    _factory->allPUF() = *puf;

    Combinations* comb = _memHandle.create<Combinations>();
    _factory->combinations() = comb;

    CPPUNIT_ASSERT(_factory->checkEarlyEOECombinability(false, fppqItem, 1, diagIn));
  }

  void setsUpForRexRebookExchangeReissue(RexPricingTrx& trx)
  {
    putTravelSegsInFareUsages();
    _farePath->rebookClassesExists() = true;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    ExcItin* excItin = static_cast<ExcItin*>(_trx->itin()[0]);
    trx.exchangeItin().push_back(excItin);
  }

  void putTravelSegsInFareUsages()
  {
    Itin* itin = _trx->itin()[0];
    itin->travelSeg()[0]->setBookingCode("Y");
    itin->travelSeg()[1]->setBookingCode("Y");
    itin->travelSeg()[2]->setBookingCode("Y");
    itin->travelSeg()[3]->setBookingCode("Y");

    _fu11->travelSeg().push_back(itin->travelSeg()[0]);
    _fu12->travelSeg().push_back(itin->travelSeg()[1]);
    _fu21->travelSeg().push_back(itin->travelSeg()[2]);
    _fu22->travelSeg().push_back(itin->travelSeg()[3]);

    _fu11->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
    _fu12->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
    _fu21->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
    _fu22->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
    _fu11->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
    _fu12->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
    _fu21->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
    _fu22->bookingCodeStatus() = PaxTypeFare::BKS_PASS;

    PaxTypeFare* ptf11 = _memHandle.create<PaxTypeFare>();
    ptf11->fareMarket() = _trx->itin()[0]->fareMarket()[0];
    PaxTypeFare* ptf12 = _memHandle.create<PaxTypeFare>();
    ptf12->fareMarket() = _trx->itin()[0]->fareMarket()[1];
    PaxTypeFare* ptf21 = _memHandle.create<PaxTypeFare>();
    ptf21->fareMarket() = _trx->itin()[0]->fareMarket()[2];
    PaxTypeFare* ptf22 = _memHandle.create<PaxTypeFare>();
    ptf22->fareMarket() = _trx->itin()[0]->fareMarket()[3];
    _fu11->paxTypeFare() = ptf11;
    _fu12->paxTypeFare() = ptf12;
    _fu21->paxTypeFare() = ptf21;
    _fu22->paxTypeFare() = ptf22;

    _fu11->segmentStatus().resize(1);
    _fu12->segmentStatus().resize(1);
    _fu21->segmentStatus().resize(1);
    _fu22->segmentStatus().resize(1);
    _fu11->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu11->segmentStatus()[0]._bkgCodeReBook = "Q";
    _fu12->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu12->segmentStatus()[0]._bkgCodeReBook = "Q";
    _fu21->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu21->segmentStatus()[0]._bkgCodeReBook = "Q";
    _fu22->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu22->segmentStatus()[0]._bkgCodeReBook = "Q";
  }

  void putDifferentialDataInFu(FareUsage* fu)
  {
    DifferentialData* diffData = _memHandle.create<DifferentialData>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->segmentStatus().resize(1);
    ptf->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    ptf->segmentStatus()[0]._bkgCodeReBook = "D";
    diffData->fareHigh() = ptf;
    diffData->travelSeg() = fu->travelSeg();
    diffData->status() = DifferentialData::SC_PASSED;
    fu->differentialPlusUp().push_back(diffData);
  }

  void fillInClassOfService(std::vector<ClassOfService*>& vec, ClassOfService& cos)
  {
    cos.bookingCode() = "Q";
    cos.numSeats() = 2;
    vec.push_back(&cos);
  }

  void prepareTrx()
  {
    PricingOptions* options = _memHandle.create<PricingOptions>();
    options->applyJourneyLogic() = true;
    options->journeyActivatedForPricing() = true;
    options->journeyActivatedForShopping() = true;
    _trx->setOptions(options);
    PricingRequest* req = _memHandle.create<PricingRequest>();
    req->lowFareRequested() = YES;
    _trx->setRequest(req);
    Billing* billing = _memHandle.create<Billing>();
    billing->actionCode() = "WPNC";
    _trx->billing() = billing;
    _factory->trx() = _trx;
  }
  void assertPopFromPriorityStack(PRIORITY expectedPriority)
  {
    CPPUNIT_ASSERT_EQUAL(expectedPriority, _factory->pqTop()->priorityStatus().negotiatedFarePriority());
    _factory->pqPop();
  }

  void createFppqItem(FPPQItem& fppqItem, FarePath& farePath, PRIORITY priority)
  {
    fppqItem.farePath() = &farePath;
    farePath.setTotalNUCAmount(100);
    fppqItem.mutablePriorityStatus().setNegotiatedFarePriority(priority);
  }

  template <typename T>
  T* create()
  {
    return _memHandle.create<T>();
  }

  FareMarket* createFareMarketWithCarrierPreference(Indicator noApplyCombTag1AndTag3 = ' ')
  {
    FareMarket* fm = create<FareMarket>();
    fm->governingCarrierPref() = createCarrierPreference(noApplyCombTag1AndTag3);
    return fm;
  }

  void setFareAndFareInfo(PaxTypeFare* ptf)
  {
    ptf->setFare(create<Fare>());
    ptf->fare()->setFareInfo(create<FareInfo>());
  }

  CarrierPreference* createCarrierPreference(Indicator noApplyCombTag1AndTag3 = ' ')
  {
    CarrierPreference* cp = create<CarrierPreference>();
    cp->noApplycombtag1and3() = noApplyCombTag1AndTag3;
    return cp;
  }

  test::FactoriesConfigStub _factoriesConfig;

  FarePathFactory* _factory;
  PricingTrx* _trx;
  FarePath* _farePath;
  PricingUnit* _pu1;
  PricingUnit* _pu2;
  FareUsage* _fu11;
  FareUsage* _fu12;
  FareUsage* _fu21;
  FareUsage* _fu22;
  PaxTypeFare* _ptf11;
  PaxTypeFare* _ptf12;
  PaxTypeFare* _ptf21;
  PaxTypeFare* _ptf22;
  Loc* _org;
  Loc* _dest;
  FareInfo* _fareInfo;
  Fare* _fare;
  FPPQItem* _fppqItem;
  PUPQItem* _pupqItem1;
  PUPQItem* _pupqItem2;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FarePathFactoryTest);
}
