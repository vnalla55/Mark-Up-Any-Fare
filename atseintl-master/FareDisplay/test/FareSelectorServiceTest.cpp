#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"

#define private public
#include "FareDisplay/FareSelectorService.h"
#undef private

#include "DataModel/StructuredRuleTrx.h"
#include "Server/TseServer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/PaxTypeFareBuilder.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <string>

namespace tse
{

using namespace ::testing;

class FareSelectorServiceTest : public Test
{
public:
  void SetUp() override
  {
    _memHandle.create<TestConfigInitializer>();
    _fsService = _memHandle.create<FareSelectorService>("Name", *_tseServer);
    _paxType = _memHandle.create<PaxType>();
    _trx = _memHandle.create<StructuredRuleTrx>();
    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);
    _trx->setOptions(_memHandle.create<PricingOptions>());
  }
  void TearDown() override { _memHandle.clear(); }

  void preparePaxTypeMapAndFareMarkets(unsigned int numberOfPassengers,
                                       unsigned int sharedFareMarketNumber,
                                       MultiPaxFCMapping& fcMap)
  {
    for (unsigned int j = 0; j < sharedFareMarketNumber; j++)
      _trx->fareMarket().push_back(_memHandle.create<FareMarket>());

    for (unsigned int i = 0; i < numberOfPassengers; i++)
    {
      auto paxType = _memHandle.create<PaxType>();
      _trx->paxType().push_back(paxType);
      auto& fareComponetVector = fcMap[paxType];

      for (unsigned int j = 0; j < sharedFareMarketNumber; j++)
      {
        FareCompInfo* fc = _memHandle.create<FareCompInfo>();
        fc->fareBasisCode() = "FareBasis" + std::to_string(i * 10 + j);
        fc->fareCalcFareAmt() = i * 10 + j;
        fc->fareMarket() = _trx->fareMarket()[j];
        fareComponetVector.push_back(fc);
      }
    }
    // fare markets point to first random PaxType's fare components

    for (unsigned int j = 0; j < sharedFareMarketNumber; j++)
    {
      auto fareMaket = _trx->fareMarket()[j];
      fareMaket->fareCompInfo() = (*fcMap.begin()).second[j];
      initializeAllPaxTypeFareOnFareMarket(*fareMaket, j * 10);

      unsigned int shift = 0;
      for (auto paxType : _trx->paxType())
      {
        createBucketWithPaxTypeOnFm(paxType, *fareMaket);
        initializeFaresInsideBucket(paxType, *fareMaket, j * 10 + shift);
        shift += 2;
      }
    }
  }

  template <typename FareContainer>
  void initializeFares(FareContainer& container,
                       FareMarket& fm,
                       unsigned int startIndex,
                       unsigned int quantity)
  {
    for (unsigned int i = startIndex; i < startIndex + quantity; i++)
    {
      std::string fareClassCode = "FareBasis" + std::to_string(i);
      container.push_back((_memHandle.insert(new PaxTypeFareBuilder(&fm, *_trx))
                               ->withFareClassCode(fareClassCode)
                               .withAmount(i)
                               .build()));
    }
  }

  void initializeAllPaxTypeFareOnFareMarket(FareMarket& fm,
                                            unsigned int startIndex,
                                            unsigned int quantity = 6)
  {
    initializeFares(fm.allPaxTypeFare(), fm, startIndex, quantity);
  }

  void initializeFaresInsideBucket(PaxType* paxType,
                                   FareMarket& fm,
                                   unsigned int startIndex,
                                   unsigned int quantity = 4)
  {
    auto& faresInBucket = fm.paxTypeCortege(paxType)->paxTypeFare();
    initializeFares(faresInBucket, fm, startIndex, quantity);
  }

  void createBucketWithPaxTypeOnFm(PaxType* paxType, FareMarket& fm)
  {
    PaxTypeBucket bucket(paxType);
    bucket.actualPaxType().push_back(paxType);
    fm.paxTypeCortege().push_back(bucket);
  }
  void prepareTestFilterMultPassanger()
  {
    _trx->createMultiPassangerFCMapping();
    _trx->setMultiPassengerSFRRequestType();
    auto fcMap = _trx->getMultiPassengerFCMapping();
    preparePaxTypeMapAndFareMarkets(2, 2, *fcMap);

    FareMarket* fM3 = _memHandle.create<FareMarket>();
    _trx->fareMarket().push_back(fM3);
    for (auto paxType : _trx->paxType())
    {
      createBucketWithPaxTypeOnFm(paxType, *fM3);
      initializeFaresInsideBucket(paxType, *fM3, 0, 6);
    }
    fM3->fareCompInfo() = _trx->fareMarket()[0]->fareCompInfo();
  }

  TestMemHandle _memHandle;
  FareMarket _fm;
  StructuredRuleTrx* _trx;
  Itin* _itin;
  TseServer* _tseServer = nullptr;
  FareSelectorService* _fsService;
  PaxType* _paxType;
};

TEST_F(FareSelectorServiceTest, SelectCorrectPaxTypeFaresInBucketsSameMarkets)
{
  _trx->paxType().push_back(_paxType);
  createBucketWithPaxTypeOnFm(_paxType, _fm);

  PaxTypeFare ptf;
  _fm.paxTypeCortege(_paxType)->paxTypeFare().push_back(&ptf);
  _fm.allPaxTypeFare().push_back(&ptf);

  std::vector<PaxTypeFare*> paxTypeFaresInBucketBefore =
      _fm.paxTypeCortege(_paxType)->paxTypeFare();
  _fsService->selectCorrectPaxTypeFaresInBuckets(&_fm, *_trx);
  std::vector<PaxTypeFare*> paxTypeFaresInBucketAfter = _fm.paxTypeCortege(_paxType)->paxTypeFare();

  ASSERT_TRUE(paxTypeFaresInBucketBefore.size() == 1);
  ASSERT_TRUE(std::equal(paxTypeFaresInBucketBefore.begin(),
                         paxTypeFaresInBucketBefore.end(),
                         paxTypeFaresInBucketAfter.begin()));
}

TEST_F(FareSelectorServiceTest, SelectCorrectPaxTypeFaresInBucketsDifferentFareMarkets)
{
  _trx->paxType().push_back(_paxType);
  createBucketWithPaxTypeOnFm(_paxType, _fm);

  PaxTypeFare ptf;
  PaxTypeFare ptf2;
  _fm.paxTypeCortege(_paxType)->paxTypeFare().push_back(&ptf);
  _fm.allPaxTypeFare().push_back(&ptf);
  _fm.allPaxTypeFare().push_back(&ptf2);

  std::vector<PaxTypeFare*> paxTypeFaresInBucketBefore =
      _fm.paxTypeCortege(_paxType)->paxTypeFare();
  _fsService->selectCorrectPaxTypeFaresInBuckets(&_fm, *_trx);
  std::vector<PaxTypeFare*> paxTypeFaresInBucketAfter = _fm.paxTypeCortege(_paxType)->paxTypeFare();

  ASSERT_TRUE(paxTypeFaresInBucketAfter.size() == 1);
  ASSERT_TRUE(std::equal(paxTypeFaresInBucketBefore.begin(),
                         paxTypeFaresInBucketBefore.end(),
                         paxTypeFaresInBucketAfter.begin()));
}

TEST_F(FareSelectorServiceTest, SelectCorrectPaxTypeFaresInBucketsAdditionalInBuckets)
{
  _trx->paxType().push_back(_paxType);
  createBucketWithPaxTypeOnFm(_paxType, _fm);

  auto* bucket = _fm.paxTypeCortege(_paxType);
  PaxTypeFare ptf;
  PaxTypeFare ptf2;
  bucket->paxTypeFare().push_back(&ptf);
  bucket->paxTypeFare().push_back(&ptf2);

  _fm.allPaxTypeFare().push_back(&ptf2);

  std::vector<PaxTypeFare*> paxTypeFaresInBucketBefore =
      _fm.paxTypeCortege(_paxType)->paxTypeFare();
  _fsService->selectCorrectPaxTypeFaresInBuckets(&_fm, *_trx);
  std::vector<PaxTypeFare*> paxTypeFaresInBucketAfter = _fm.paxTypeCortege(_paxType)->paxTypeFare();

  ASSERT_TRUE(paxTypeFaresInBucketBefore.size() != paxTypeFaresInBucketAfter.size());
  ASSERT_TRUE(paxTypeFaresInBucketAfter.front() == &ptf2);
}

TEST_F(FareSelectorServiceTest, ProcessForSinglePassenger_faresAreFilteredProperly)
{
  FareCompInfo fc;
  fc.fareBasisCode() = "FareBasis4";
  fc.fareCalcFareAmt() = 4;
  FareCompInfo fc2;
  fc2.fareBasisCode() = "FareBasis24";
  fc2.fareCalcFareAmt() = 102;
  FareCompInfo fc3;
  fc3.fareBasisCode() = "FareBasis35";
  fc3.fareCalcFareAmt() = 35;
  FareCompInfo fc4;
  fc4.fareBasisCode() = "FareBasis65";
  fc4.fareCalcFareAmt() = 65;

  FareMarket fm1;
  fm1.fareCompInfo() = &fc;
  initializeAllPaxTypeFareOnFareMarket(fm1, 0);

  FareMarket fm2;
  fm2.fareCompInfo() = &fc2;
  initializeAllPaxTypeFareOnFareMarket(fm2, 20);

  FareMarket fm3;
  fm3.fareCompInfo() = &fc3;
  initializeAllPaxTypeFareOnFareMarket(fm3, 30);

  FareMarket fm4;
  fm4.fareCompInfo() = &fc4;

  // 2 x 8  = 16 fares
  initializeAllPaxTypeFareOnFareMarket(fm4, 60);
  initializeAllPaxTypeFareOnFareMarket(fm4, 60);

  _trx->fareMarket().push_back(&fm1);
  _trx->fareMarket().push_back(&fm2);
  _trx->fareMarket().push_back(&fm3);
  _trx->fareMarket().push_back(&fm4);

  _fsService->processForSinglePassenger(*_trx);

  // There should be only one fare after filtering ( fare market 1, 2, 3 )

  ASSERT_TRUE(_trx->fareMarket()[0]->allPaxTypeFare().size() == 1);
  ASSERT_TRUE(_trx->fareMarket()[1]->allPaxTypeFare().size() == 0);
  ASSERT_TRUE(_trx->fareMarket()[2]->allPaxTypeFare().size() == 1);
  ASSERT_TRUE(_trx->fareMarket()[3]->allPaxTypeFare().size() == 2);

  ASSERT_TRUE(_trx->fareMarket()[0]->allPaxTypeFare()[0]->fareAmount() == 4);
  ASSERT_TRUE(_trx->fareMarket()[0]->allPaxTypeFare()[0]->fare()->fareClass() == "FareBasis4");
  ASSERT_TRUE(_trx->fareMarket()[2]->allPaxTypeFare()[0]->fareAmount() == 35);
  ASSERT_TRUE(_trx->fareMarket()[2]->allPaxTypeFare()[0]->fare()->fareClass() == "FareBasis35");

  // There should be two fares after filtering on fare market 4

  ASSERT_TRUE(_trx->fareMarket()[3]->allPaxTypeFare()[0]->fareAmount() == 65);
  ASSERT_TRUE(_trx->fareMarket()[3]->allPaxTypeFare()[0]->fare()->fareClass() == "FareBasis65");
  ASSERT_TRUE(_trx->fareMarket()[3]->allPaxTypeFare()[1]->fareAmount() == 65);
  ASSERT_TRUE(_trx->fareMarket()[3]->allPaxTypeFare()[1]->fare()->fareClass() == "FareBasis65");
}

TEST_F(FareSelectorServiceTest, ProcessForMultiPassenger_filterAllPassengerBucketsProperly)
{
  prepareTestFilterMultPassanger();
  /* First Fare Market
    *
    * allPaxType :  FareBasis0, FareBasis1, FareBasis2, FareBasis3, FareBasis4, FareBasis5
    * Bucket1 : FareBasis0, FareBasis1, FareBasis2, FareBasis3
    * Bucket2 : FareBasis2, FareBasis3, FareBasis4, FareBasis5
    *
    */

  /* Second Fare Market
   *
   * allPaxType :  FareBasis10, FareBasis11, FareBasis12, FareBasis13, FareBasis14, FareBasis15
   * Bucket1 : FareBasis10, FareBasis11, FareBasis12, FareBasis13
   * Bucket2 : FareBasis12, FareBasis13, FareBasis14, FareBasis15
   *
   */
  _fsService->process(*_trx);

  FareMarket* fM1 = _trx->fareMarket()[0];
  FareMarket* fM2 = _trx->fareMarket()[1];
  auto& fM1_bucket1_fares = fM1->paxTypeCortege(_trx->paxType()[0])->paxTypeFare();
  auto& fM1_bucket2_fares = fM1->paxTypeCortege(_trx->paxType()[1])->paxTypeFare();

  ASSERT_TRUE(fM1->allPaxTypeFare().size() == 1);
  ASSERT_TRUE(fM1->allPaxTypeFare()[0]->fareAmount() == 0);
  ASSERT_TRUE(fM1->allPaxTypeFare()[0]->fare()->fareClass() == "FareBasis0");

  ASSERT_TRUE(fM1_bucket1_fares.size() == 1);
  ASSERT_TRUE(fM1_bucket1_fares[0]->fareAmount() == 0);
  ASSERT_TRUE(fM1_bucket1_fares[0]->fare()->fareClass() == "FareBasis0");

  ASSERT_TRUE(fM1_bucket2_fares.empty());

  auto& fM2_bucket1_fares = fM2->paxTypeCortege(_trx->paxType()[0])->paxTypeFare();
  auto& fM2_bucket2_fares = fM2->paxTypeCortege(_trx->paxType()[1])->paxTypeFare();
  ASSERT_TRUE(fM2->allPaxTypeFare().empty());
  ASSERT_TRUE(fM2_bucket1_fares.empty());
  ASSERT_TRUE(fM2_bucket2_fares.empty());

  //                                      FM 1         FM2
  // First  PaxType Fare Components :  FareBasis0,   FareBasis1
  // Second PaxType Fare Components :  FareBasis10,  FareBasis11

  //=================================== After filtering=========================================//

  /* First Fare Market
   *
   * allPaxType :  FareBasis0
   * Bucket1 : FareBasis0
   * Bucket2 : empty
   *
   */

  /* Second Fare Market
   *
   * allPaxType :  empty
   * Bucket1 : empty
   * Bucket2 : empty
   *
   */
}

TEST_F(FareSelectorServiceTest,
       ProcessForMultiPassenger_testFilterAllBucketsIfFareMarketDoesNotPointToPaxFareCompInfo)
{
  prepareTestFilterMultPassanger();

  /* First Fare Market
    *
    * allPaxType :  FareBasis0, FareBasis1, FareBasis2, FareBasis3, FareBasis4, FareBasis5
    * Bucket1 : FareBasis0, FareBasis1, FareBasis2, FareBasis3
    * Bucket2 : FareBasis2, FareBasis3, FareBasis4, FareBasis5
    *
    */

  /* Second Fare Market
   *
   * allPaxType :  FareBasis10, FareBasis11, FareBasis12, FareBasis13, FareBasis14, FareBasis15
   * Bucket1 : FareBasis10, FareBasis11, FareBasis12, FareBasis13
   * Bucket2 : FareBasis12, FareBasis13, FareBasis14, FareBasis15
   *
   */
  /* Third Fare Market -- not pointed by any FareComponent
   *
   * allPaxType :  FareBasis0, FareBasis1, FareBasis2, FareBasis3, FareBasis4, FareBasis5
   * Bucket1 : FareBasis0, FareBasis1, FareBasis2, FareBasis3
   * Bucket1 : FareBasis0, FareBasis1, FareBasis2, FareBasis3
   *
   */

  //                                      FM 1         FM2
  // First  PaxType Fare Components :  FareBasis0,   FareBasis1
  // Second PaxType Fare Components :  FareBasis10,  FareBasis11

  _fsService->process(*_trx);
  FareMarket* fM3 = _trx->fareMarket()[2];
  auto _bucket31 = fM3->paxTypeCortege(_trx->paxType()[0]);
  auto _bucket32 = fM3->paxTypeCortege(_trx->paxType()[1]);
  ASSERT_TRUE(_bucket31->isBucketFilteredSFR() && _bucket32->isBucketFilteredSFR());
  ASSERT_TRUE(_bucket31->paxTypeFare().size() == 1);
  ASSERT_TRUE(_bucket31->paxTypeFare()[0]->fare()->fareClass() == "FareBasis0");
  ASSERT_TRUE(_bucket32->paxTypeFare().size() == 0);

  //=================================== After filtering=========================================//

  /* First Fare Market
   *
   * allPaxType :  FareBasis0
   * Bucket1 : FareBasis0
   * Bucket2 : empty
   *
   */

  /* Second Fare Market
   *
   * allPaxType :  empty
   * Bucket1 : empty
   * Bucket2 : empty
   *
   */
  /* Third Fare Market
   *
   * allPaxType :  FareBasis0
   * Bucket1 : FareBasis0
   * Bucket2 : empty
   *
   */
}
}
