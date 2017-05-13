#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include "Pricing/test/FactoriesConfigStub.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"
#include "DataModel/Diversity.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "Pricing/Shopping/PQ/SoloFarePathFactory.h"

namespace tse
{
namespace shpq
{

class SoloFarePathFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloFarePathFactoryTest);
  CPPUNIT_TEST(checkBrandParity);
  //  CPPUNIT_TEST(checkBrandParity_catchAllBucket);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _farePath = _memHandle.create<FarePath>();
    _pu1 = _memHandle.create<PricingUnit>();
    _fu11 = _memHandle.create<FareUsage>();
    _ptf11 = _memHandle.create<PaxTypeFare>();
    _fu12 = _memHandle.create<FareUsage>();
    _ptf12 = _memHandle.create<PaxTypeFare>();
    _pu2 = _memHandle.create<PricingUnit>();
    _fu21 = _memHandle.create<FareUsage>();
    _ptf21 = _memHandle.create<PaxTypeFare>();
    _fu22 = _memHandle.create<FareUsage>();
    _ptf22 = _memHandle.create<PaxTypeFare>();
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

    _bProgram1 = _memHandle.create<BrandProgram>();
    _bProgram2 = _memHandle.create<BrandProgram>();

    _brand1 = _memHandle.create<BrandInfo>();
    _brand1->brandCode() = "B1";
    _brand2 = _memHandle.create<BrandInfo>();
    _brand2->brandCode() = "B2";
    _brand3 = _memHandle.create<BrandInfo>();
    _brand3->brandCode() = "B3";
    _brand4 = _memHandle.create<BrandInfo>();
    _brand4->brandCode() = "B4";
    _brand5 = _memHandle.create<BrandInfo>();
    _brand5->brandCode() = "B5";
    _brand6 = _memHandle.create<BrandInfo>();
    _brand6->brandCode() = "B1";
    _brand7 = _memHandle.create<BrandInfo>();
    _brand7->brandCode() = "B2";
    _brand8 = _memHandle.create<BrandInfo>();
    _brand8->brandCode() = "B8";
    _brand9 = _memHandle.create<BrandInfo>();
    _brand9->brandCode() = "B9";

    _dataHandle.get(_shoppingTrx);
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    _shoppingTrx->setRequest(pRequest);

    _shoppingTrx->legs().push_back(ShoppingTrx::Leg());
    _shoppingTrx->legs().push_back(ShoppingTrx::Leg());

    _shoppingTrx->brandProgramVec().push_back(std::make_pair(_bProgram1, _brand1));
    _shoppingTrx->brandProgramVec().push_back(std::make_pair(_bProgram1, _brand2));
    _shoppingTrx->brandProgramVec().push_back(std::make_pair(_bProgram1, _brand3));
    _shoppingTrx->brandProgramVec().push_back(std::make_pair(_bProgram1, _brand4));
    _shoppingTrx->brandProgramVec().push_back(std::make_pair(_bProgram2, _brand5));
    _shoppingTrx->brandProgramVec().push_back(std::make_pair(_bProgram2, _brand6));
    _shoppingTrx->brandProgramVec().push_back(std::make_pair(_bProgram2, _brand7));
    _shoppingTrx->brandProgramVec().push_back(std::make_pair(_bProgram2, _brand8));

    _fm1 = _memHandle.create<FareMarket>();
    _fm2 = _memHandle.create<FareMarket>();
    _fm3 = _memHandle.create<FareMarket>();
    _fm4 = _memHandle.create<FareMarket>();

    _fm1->legIndex() = 0;
    _fm2->legIndex() = 0;
    _fm3->legIndex() = 1;
    _fm4->legIndex() = 1;

    _org = _memHandle.create<Loc>();
    _org->loc() = "SYD";
    _dest = _memHandle.create<Loc>();
    _dest->loc() = "AKL";

    _fareInfo = _memHandle.create<FareInfo>();
    _fareInfo->directionality() = BOTH;
    _fareInfo->market1() = "SYD";
    _fareInfo->market1() = "AKL";

    _fare = _memHandle.create<Fare>();
    _fare->initialize(Fare::FareState::FS_Domestic, _fareInfo, *_fm1, nullptr, nullptr);
    _fm1->origin() = _org;
    _fm1->destination() = _dest;
    _fm2->origin() = _org;
    _fm2->destination() = _dest;
    _fm3->origin() = _org;
    _fm3->destination() = _dest;
    _fm4->origin() = _org;
    _fm4->destination() = _dest;

    _ptf11->setFare(_fare);
    _ptf12->setFare(_fare);
    _ptf21->setFare(_fare);
    _ptf22->setFare(_fare);

    _soloFarePathFactory = _memHandle.create<SoloFarePathFactory>(_factoriesConfig);
    _soloFarePathFactory->_trx = _shoppingTrx;
  }

  void checkBrandParity()
  {
    const int BRAND_ARRAY_SIZE = 8;

    _shoppingTrx->getRequest()->setBrandedFaresRequest(true);

    int brandIndexArray[BRAND_ARRAY_SIZE] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    _fm1->brandProgramIndexVec().assign(brandIndexArray, brandIndexArray + BRAND_ARRAY_SIZE);
    _fm2->brandProgramIndexVec().assign(brandIndexArray, brandIndexArray + BRAND_ARRAY_SIZE);
    _fm3->brandProgramIndexVec().assign(brandIndexArray, brandIndexArray + BRAND_ARRAY_SIZE);
    _fm4->brandProgramIndexVec().assign(brandIndexArray, brandIndexArray + BRAND_ARRAY_SIZE);

    _ptf11->fareMarket() = _fm1;
    _ptf11->getMutableBrandStatusVec().assign(BRAND_ARRAY_SIZE,
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

    _ptf12->fareMarket() = _fm2;
    _ptf12->getMutableBrandStatusVec().assign(BRAND_ARRAY_SIZE,
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

    _ptf21->fareMarket() = _fm3;
    _ptf21->getMutableBrandStatusVec().assign(BRAND_ARRAY_SIZE,
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

    _ptf22->fareMarket() = _fm4;
    _ptf22->getMutableBrandStatusVec().assign(BRAND_ARRAY_SIZE,
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

    bool result;
    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);

    CPPUNIT_ASSERT(result == false);

    _ptf11->getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_SOFT_PASS;
    _ptf12->getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_SOFT_PASS;
    _ptf21->getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_SOFT_PASS;

    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == false);

    _ptf22->getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_SOFT_PASS;

    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == true);

    // IN NGS there's another validation that should not pass a fare path if there isn't at least
    // one hard passed brand on each leg

    // enabling NGS path:a
    _shoppingTrx->setTrxType(PricingTrx::IS_TRX);
    _shoppingTrx->diversity().setEnabled();
    _shoppingTrx->setSimpleTrip(true);
    PaxType* actualPType = _memHandle.create<PaxType>();
    _shoppingTrx->paxType().push_back(actualPType);

    CPPUNIT_ASSERT(_shoppingTrx->isSumOfLocalsProcessingEnabled());

    //************************************************************

    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == false);

    // adding hard passed brands on each leg, but using brands that don't constitute parity for the
    // whole journey
    _ptf11->getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_HARD_PASS;
    _ptf12->getMutableBrandStatusVec()[1].first = PaxTypeFare::BS_HARD_PASS;
    _ptf21->getMutableBrandStatusVec()[2].first = PaxTypeFare::BS_HARD_PASS;
    _ptf22->getMutableBrandStatusVec()[3].first = PaxTypeFare::BS_HARD_PASS;

    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == false);

    // creatng a hard passed brand having parity
    _ptf11->getMutableBrandStatusVec()[5].first = PaxTypeFare::BS_HARD_PASS;
    _ptf12->getMutableBrandStatusVec()[5].first = PaxTypeFare::BS_HARD_PASS;
    _ptf21->getMutableBrandStatusVec()[5].first = PaxTypeFare::BS_HARD_PASS;
    _ptf22->getMutableBrandStatusVec()[5].first = PaxTypeFare::BS_HARD_PASS;

    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == true);

    // remove hard pass of the one brand on the second leg. It should still pass as only one hard
    // pass is needed

    _ptf22->getMutableBrandStatusVec()[5].first = PaxTypeFare::BS_SOFT_PASS;
    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == true);

    // remove the other hard pass on this leg. Now it should fail as there will be no hard pass on
    // this leg

    _ptf21->getMutableBrandStatusVec()[5].first = PaxTypeFare::BS_SOFT_PASS;
    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == false);
  }

  void checkBrandParity_catchAllBucket()
  {
    // catchAllBucket should be ignored - as opposed to the previous version where with CAB response
    // was always true
    _shoppingTrx->getRequest()->setBrandedFaresRequest(true);
    _shoppingTrx->getRequest()->setCatchAllBucketRequest(true);
    bool result;
    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == false);

    _shoppingTrx->getRequest()->setCatchAllBucketRequest(false);
    result = _soloFarePathFactory->hasBrandParity(*_farePath, _dc);
    CPPUNIT_ASSERT(result == false);
  }

  void tearDown() { _memHandle.clear(); }

private:
  ShoppingTrx* _shoppingTrx;

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

  BrandProgram* _bProgram1;
  BrandProgram* _bProgram2;
  BrandInfo* _brand1;
  BrandInfo* _brand2;
  BrandInfo* _brand3;
  BrandInfo* _brand4;
  BrandInfo* _brand5;
  BrandInfo* _brand6;
  BrandInfo* _brand7;
  BrandInfo* _brand8;
  BrandInfo* _brand9;

  FareMarket* _fm1;
  FareMarket* _fm2;
  FareMarket* _fm3;
  FareMarket* _fm4;

  Fare* _fare;
  FareInfo* _fareInfo;
  Loc* _org;
  Loc* _dest;

  DataHandle _dataHandle;
  TestMemHandle _memHandle;
  test::FactoriesConfigStub _factoriesConfig;
  SoloFarePathFactory* _soloFarePathFactory;
  DiagCollector _dc;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SoloFarePathFactoryTest);
}
}
