#include <vector>

#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/FareMarketUtil.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/BrandedFaresParityValidator.h"
#include "test/include/TestConfigInitializer.h"
#include "Common/ErrorResponseException.h"
#include "DataModel/AirSeg.h"
#include "Common/DateTime.h"
#include "Common/ShoppingUtil.h"

#include "test/include/TestMemHandle.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/S8BrandedFaresSelector.h"
#include "Common/ErrorResponseException.h"

namespace tse
{
using boost::assign::operator+=;
using namespace std;

class BrandedFaresParityValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandedFaresParityValidatorTest);

  CPPUNIT_TEST(checkSetsIntersection);
  CPPUNIT_TEST(removeInvalidBrandsFromBrandVector);
  CPPUNIT_TEST(markMarketsWithoutBrandedFaresAsInvalid);
  CPPUNIT_TEST(fullTest);
  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  DataHandle _dataHandle;

public:
  void setPricingRequest(ShoppingTrx* trx)
  {
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    trx->setRequest(pRequest);
    trx->getRequest()->setCatchAllBucketRequest(false);
  }

  void markMarketsWithoutBrandedFaresAsInvalid()
  {
    ShoppingTrx* trx = _memHandle.create<ShoppingTrx>();
    setPricingRequest(trx);

    BrandedFaresParityValidator validator(*trx);
    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->failCode() = ErrorResponseException::NO_ERROR;
    validator.markMarketsWithoutBrandedFaresAsInvalid(fareMarket);
    CPPUNIT_ASSERT(fareMarket->failCode() == ErrorResponseException::NO_VALID_BRAND_FOUND);
  }

  void removeInvalidBrandsFromBrandVector()
  {
    std::vector<int> removedBrands;
    std::vector<int> brandsVec;
    BrandCodeSet commonBrands;

    BrandCode commonBrandsArray[] = { "B1", "B2" };
    commonBrands.insert(commonBrandsArray, commonBrandsArray + 2);
    int brandsForVector[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    brandsVec.insert(brandsVec.begin(), brandsForVector, brandsForVector + 10);

    ShoppingTrx* trx = _memHandle.create<ShoppingTrx>();
    setPricingRequest(trx);
    prepareTestData(trx);
    BrandedFaresParityValidator validator(*trx);

    FareMarket fm;
    fm.brandProgramIndexVec().insert(
        fm.brandProgramIndexVec().begin(), brandsVec.begin(), brandsVec.end());

    validator.removeInvalidBrands(&fm, commonBrands, removedBrands);

    CPPUNIT_ASSERT(removedBrands.size() == 6);
    CPPUNIT_ASSERT(fm.brandProgramIndexVec()[0] == 0 && fm.brandProgramIndexVec()[1] == 1 &&
                   fm.brandProgramIndexVec()[2] == 5 && fm.brandProgramIndexVec()[3] == 6);
  }

  void checkSetsIntersection()
  {
    ShoppingTrx* trx = _memHandle.create<ShoppingTrx>();
    setPricingRequest(trx);
    BrandedFaresParityValidator validator(*trx);

    BrandCode myInts1[] = { "B1", "B2", "B3", "B4", "B5" };
    BrandCode myInts2[] = { "B4", "B5", "B6", "B7" };
    BrandCode myInts3[] = { "B7", "B1", "B4", "B5" };

    BrandCode resultInts[] = { "B4", "B5" };

    BrandCodeSet set1(myInts1, myInts1 + 5);
    BrandCodeSet set2(myInts2, myInts2 + 4);
    BrandCodeSet set3(myInts3, myInts3 + 4);

    BrandCodeSetVec sets;
    sets.push_back(set1);
    sets.push_back(set2);
    sets.push_back(set3);

    BrandCodeSet expected_result(resultInts, resultInts + 2);

    BrandCodeSet result = validator.getIntersectionOfSets(sets);

    CPPUNIT_ASSERT(expected_result == result);
  }

  void prepareTestData(ShoppingTrx* shoppingTrx)
  {
    shoppingTrx->getRequest()->setCatchAllBucketRequest(false);

    BrandProgram* bProgram1 = _memHandle.create<BrandProgram>();
    BrandProgram* bProgram2 = _memHandle.create<BrandProgram>();
    BrandInfo* brand1 = _memHandle.create<BrandInfo>();
    brand1->brandCode() = "B1";
    BrandInfo* brand2 = _memHandle.create<BrandInfo>();
    brand2->brandCode() = "B2";
    BrandInfo* brand3 = _memHandle.create<BrandInfo>();
    brand3->brandCode() = "B3";
    BrandInfo* brand4 = _memHandle.create<BrandInfo>();
    brand4->brandCode() = "B4";
    BrandInfo* brand5 = _memHandle.create<BrandInfo>();
    brand5->brandCode() = "B5";
    BrandInfo* brand6 = _memHandle.create<BrandInfo>();
    brand6->brandCode() = "B1";
    BrandInfo* brand7 = _memHandle.create<BrandInfo>();
    brand7->brandCode() = "B2";
    BrandInfo* brand8 = _memHandle.create<BrandInfo>();
    brand8->brandCode() = "B8";
    BrandInfo* brand9 = _memHandle.create<BrandInfo>();
    brand9->brandCode() = "B9";
    BrandInfo* brand0 = _memHandle.create<BrandInfo>();
    brand0->brandCode() = "B0";

    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram1, brand1));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram1, brand2));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram1, brand3));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram1, brand4));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram1, brand5));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram2, brand6));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram2, brand7));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram2, brand8));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram2, brand9));
    shoppingTrx->brandProgramVec().push_back(std::make_pair(bProgram2, brand0));

    AirSeg* airSeg1 = buildSegment("AAA", "BBB", "AA");
    AirSeg* airSeg2 = buildSegment("BBB", "CCC", "AA");

    Itin* itineraryOb1;
    _dataHandle.get(itineraryOb1);

    FareMarket* fareMarket1 = _memHandle.create<FareMarket>();
    fareMarket1->governingCarrier() = "AA";

    int brands1Array[] = { 1, 2, 3 };
    fareMarket1->brandProgramIndexVec().insert(
        fareMarket1->brandProgramIndexVec().begin(), brands1Array, brands1Array + 3);
    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = "AAA";
    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = "CCC";
    fareMarket1->origin() = origin;
    fareMarket1->destination() = destination;
    shoppingTrx->fareMarket().push_back(fareMarket1);

    PaxTypeFare* fare1 = _memHandle.create<PaxTypeFare>();
    CPPUNIT_ASSERT(fare1->isValidForBranding() == true); // by default is has to be true
    fare1->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));
    fare1->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));
    fare1->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));

    PaxTypeFare* fare2 = _memHandle.create<PaxTypeFare>();
    CPPUNIT_ASSERT(fare2->isValidForBranding() == true); // by default is has to be true
    fare2->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));
    fare2->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    fare2->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

    PaxTypeFare* fare3 = _memHandle.create<PaxTypeFare>();
    fare3->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    fare3->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    fare3->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    fareMarket1->allPaxTypeFare().push_back(fare1);
    fareMarket1->allPaxTypeFare().push_back(fare2);
    fareMarket1->allPaxTypeFare().push_back(fare3);

    FareMarket* fareMarket2 = _memHandle.create<FareMarket>();
    fareMarket2->governingCarrier() = "BB";
    int brands2Array[] = { 4, 5, 6 };
    fareMarket2->brandProgramIndexVec().insert(
        fareMarket2->brandProgramIndexVec().begin(), brands2Array, brands2Array + 3);
    Loc* origin2 = _memHandle.create<Loc>();
    origin2->loc() = "AAA";
    Loc* destination2 = _memHandle.create<Loc>();
    destination2->loc() = "BBB";
    fareMarket2->origin() = origin2;
    fareMarket2->destination() = destination2;
    shoppingTrx->fareMarket().push_back(fareMarket2);

    FareMarket* fareMarket3 = _memHandle.create<FareMarket>();
    fareMarket3->governingCarrier() = "BB";
    int brands3Array[] = { 0, 1, 3, 5, 6 };
    fareMarket3->brandProgramIndexVec().insert(
        fareMarket3->brandProgramIndexVec().begin(), brands3Array, brands3Array + 5);
    Loc* origin3 = _memHandle.create<Loc>();
    origin3->loc() = "BBB";
    Loc* destination3 = _memHandle.create<Loc>();
    destination3->loc() = "CCC";
    fareMarket3->origin() = origin3;
    fareMarket3->destination() = destination3;
    shoppingTrx->fareMarket().push_back(fareMarket3);
    itineraryOb1->fareMarket().push_back(fareMarket1);
    itineraryOb1->fareMarket().push_back(fareMarket2);
    itineraryOb1->fareMarket().push_back(fareMarket3);

    FareMarket* fareMarket5 = _memHandle.create<FareMarket>();
    fareMarket5->governingCarrier() = "BB";
    int brands5Array[] = { 7, 8 };
    fareMarket5->brandProgramIndexVec().insert(
        fareMarket5->brandProgramIndexVec().begin(), brands5Array, brands5Array + 2);
    Loc* origin5 = _memHandle.create<Loc>();
    origin5->loc() = "AAA";
    Loc* destination5 = _memHandle.create<Loc>();
    destination5->loc() = "DDD";
    fareMarket5->origin() = origin5;
    fareMarket5->destination() = destination5;
    shoppingTrx->fareMarket().push_back(fareMarket5);

    FareMarket* fareMarket6 = _memHandle.create<FareMarket>();
    fareMarket6->governingCarrier() = "BB";
    int brands6Array[] = { 8, 9 };
    fareMarket6->brandProgramIndexVec().insert(
        fareMarket6->brandProgramIndexVec().begin(), brands6Array, brands6Array + 2);
    Loc* origin6 = _memHandle.create<Loc>();
    origin6->loc() = "DDD";
    Loc* destination6 = _memHandle.create<Loc>();
    destination6->loc() = "CCC";
    fareMarket6->origin() = origin6;
    fareMarket6->destination() = destination6;
    shoppingTrx->fareMarket().push_back(fareMarket6);

    FareMarket* fareMarket4 = _memHandle.create<FareMarket>();
    fareMarket4->brandProgramIndexVec().push_back(4);
    Loc* origin4 = _memHandle.create<Loc>();
    origin4->loc() = "AAA";
    Loc* destination4 = _memHandle.create<Loc>();
    destination4->loc() = "BBB";
    fareMarket4->origin() = origin4;
    fareMarket4->destination() = destination4;
    shoppingTrx->fareMarket().push_back(fareMarket4);

    itineraryOb1->fareMarket().push_back(fareMarket5);
    itineraryOb1->fareMarket().push_back(fareMarket6);
    itineraryOb1->fareMarket().push_back(fareMarket4);

    itineraryOb1->travelSeg().push_back(airSeg1);
    itineraryOb1->travelSeg().push_back(airSeg2);

    shoppingTrx->legs().push_back(ShoppingTrx::Leg());
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb1, 1, true));
    shoppingTrx->legs().front().sop().front().governingCarrier() = "AA";

    ItinIndex itinIndex;
    ItinIndex::ItinCellInfo itinCellInfo;
    ItinIndex::Key carrierKey;
    ShoppingUtil::createCxrKey("AA", carrierKey);
    ItinIndex::Key scheduleKey;
    ShoppingUtil::createScheduleKey(itineraryOb1, scheduleKey);
    shoppingTrx->legs().front().carrierIndex().addItinCell(
        itineraryOb1, itinCellInfo, carrierKey, scheduleKey);
    itinIndex.addItinCell(itineraryOb1, itinCellInfo, carrierKey, scheduleKey);
  }

  AirSeg* buildSegment(string origin,
                       string destination,
                       string carrier,
                       DateTime departure = DateTime::localTime(),
                       DateTime arrival = DateTime::localTime(),
                       bool stopOver = false)
  {
    AirSeg* airSeg;
    _dataHandle.get(airSeg);
    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;
    Loc* locorig, *locdest;
    _dataHandle.get(locorig);
    _dataHandle.get(locdest);
    locorig->loc() = origin;
    locdest->loc() = destination;

    airSeg->origAirport() = origin;
    airSeg->origin() = locorig;
    airSeg->destAirport() = destination;
    airSeg->destination() = locdest;

    airSeg->carrier() = carrier;
    airSeg->setOperatingCarrierCode(carrier);
    airSeg->boardMultiCity() = locorig->loc();
    airSeg->offMultiCity() = locdest->loc();
    airSeg->stopOver() = stopOver;

    return airSeg;
  }

  void fullTest()
  {
    ShoppingTrx* shoppingTrx;
    _dataHandle.get(shoppingTrx);
    setPricingRequest(shoppingTrx);
    prepareTestData(shoppingTrx);

    BrandedFaresParityValidator parityValidator(*shoppingTrx);
    BrandCodeSetVec brandsPerLeg(1);
    parityValidator.getBrandsOnEachLeg(brandsPerLeg);

    BrandCodeSet brandsCommonForAllLegs = parityValidator.getIntersectionOfSets(brandsPerLeg);

    BrandCodeSet::iterator it;

    BrandCode resultBrands[] = { "B1", "B2", "B3", "B4", "B9" };
    BrandCodeSet correctResult;
    correctResult.insert(resultBrands, resultBrands + 5);

    BrandCodeSet::iterator it2;

    CPPUNIT_ASSERT(brandsCommonForAllLegs.size() == 5);
    CPPUNIT_ASSERT(brandsCommonForAllLegs == correctResult);

    parityValidator.removeInvalidBrandsInFaresAndFareMarkets(parityValidator._allFareMarkets,
                                                             brandsCommonForAllLegs);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->brandProgramIndexVec().size() == 3);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[1]->brandProgramIndexVec().size() == 2);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[2]->brandProgramIndexVec().size() == 5);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[3]->brandProgramIndexVec().size() == 1);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[4]->brandProgramIndexVec().size() == 1);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->brandProgramIndexVec()[0] == 1);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->brandProgramIndexVec()[1] == 2);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->brandProgramIndexVec()[2] == 3);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[1]->brandProgramIndexVec()[0] == 5);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[1]->brandProgramIndexVec()[1] == 6);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[2]->brandProgramIndexVec()[0] == 0);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[2]->brandProgramIndexVec()[1] == 1);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[2]->brandProgramIndexVec()[2] == 3);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[2]->brandProgramIndexVec()[3] == 5);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[2]->brandProgramIndexVec()[4] == 6);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[3]->brandProgramIndexVec()[0] == 8);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[4]->brandProgramIndexVec()[0] == 8);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->allPaxTypeFare()[0]->getBrandStatusVec().size() == 3);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->allPaxTypeFare()[0]->isValidForBranding() == true);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->allPaxTypeFare()[1]->getBrandStatusVec().size() == 3);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->allPaxTypeFare()[1]->isValidForBranding() == true);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->allPaxTypeFare()[2]->getBrandStatusVec().size() == 3);
    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[0]->allPaxTypeFare()[2]->isValidForBranding() == false);

    CPPUNIT_ASSERT(shoppingTrx->fareMarket()[5]->failCode() ==
                   ErrorResponseException::NO_VALID_BRAND_FOUND);
  }

  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BrandedFaresParityValidatorTest);
}
