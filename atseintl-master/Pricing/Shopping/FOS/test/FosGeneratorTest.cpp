// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include <stdint.h>
#include <vector>

#include "DBAccess/Mileage.h"
#include "Pricing/Shopping/FOS/FosFilter.h"
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/test/FosGeneratorMock.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/RefDataHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    if (origin == "FRA" && dest == "WAW")
    {
      ret->mileage() = 895;
      return ret;
    }
    if (origin == "WAW" && dest == "KRK")
    {
      ret->mileage() = 246;
      return ret;
    }
    if (origin == "KRK" && dest == "MUC")
    {
      ret->mileage() = 618;
      return ret;
    }
    if (origin == "MUC" && dest == "SYD")
    {
      ret->mileage() = 16300;
      return ret;
    }
    if (globalDir == GlobalDirection::XX)
      return 0;

    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
};
} // namespace

namespace fos
{
class FosFilterDivisible : public FosFilter
{
public:
  FosFilterDivisible(uint32_t divisor) : _divisor(divisor) {}

  FilterType getType() const { return FILTER_COMPOSITE; }

  bool isApplicableSolution(const SopCombination& sopCombination) const
  {
    uint32_t sum = 0;
    for (uint32_t sopId = 0; sopId < sopCombination.size(); ++sopId)
      sum += sopCombination[sopId];
    return !(sum % _divisor);
  }

private:
  SopsWithDetailsSet _sopsWithDetailsFilteredSet;
  const uint32_t _divisor;
};

class FosFilterCxnCity : public FosFilter
{
public:
  FosFilterCxnCity(ShoppingTrx& trx, std::string cxnCity = "") : _trx(trx), _cxnCity(cxnCity) {}

  FilterType getType() const { return FILTER_COMPOSITE; }
  bool isApplicableSolution(const SopCombination& sopCombination) const { return true; }

  const SopDetailsPtrVec& getFilteredSopDetails(const DetailedSop& orginal)
  {
    typedef SopDetailsPtrVec::const_iterator SopDetailsCI;
    SopsWithDetailsSet::iterator foundSop = _sopsWithDetailsFilteredSet.find(orginal);
    if (foundSop != _sopsWithDetailsFilteredSet.end())
      return foundSop->getSopDetailsVec();

    uint32_t legId = orginal.getLegId();
    uint32_t sopId = orginal.getSopId();
    DetailedSop* newSop = &_trx.dataHandle().safe_create<DetailedSop>(legId, sopId);

    SopDetailsCI it = orginal.getSopDetailsVec().begin();
    SopDetailsCI itEnd = orginal.getSopDetailsVec().end();

    for (; it != itEnd; ++it)
    {
      SopDetails* sd = *it;
      if (sd->destAirport == _cxnCity)
      {
        newSop->addDetail(sd);
      }
    }
    _sopsWithDetailsFilteredSet.insert(*newSop);

    return newSop->getSopDetailsVec();
  }

private:
  SopsWithDetailsSet _sopsWithDetailsFilteredSet;
  ShoppingTrx& _trx;
  const std::string _cxnCity;
};

class FosFilterCombReuse : public FosFilter
{
public:
  bool isApplicableSolution(const SopCombination& sopCombination) const { return true; }
  FilterType getType() const { return FILTER_COMPOSITE; }
  bool isCombinationForReuse(const SopCombination& sopCombination) const { return true; }
};

class FosGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FosGeneratorTest);
  CPPUNIT_TEST(getNextCombinationTest);
  CPPUNIT_TEST(getNextCombinationOneFilterTest);
  CPPUNIT_TEST(getNextCombinationTwoFilterTest);
  CPPUNIT_TEST(generateSopDetailsTest);
  CPPUNIT_TEST(getSopDetailsTest);
  CPPUNIT_TEST(addCombinationForReuseTest);
  CPPUNIT_TEST_SUITE_END();

  void getNextCombinationTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    int sopIds[] = { 1, 2, 3, 4, 5, 6 };
    std::vector<int> sopVec(sopIds, sopIds + sizeof(sopIds) / sizeof(int));
    std::vector<std::vector<int> > sopByLeg;
    sopByLeg.insert(sopByLeg.begin(), sopVec);

    FosStatistic fosStatistic(*trx);
    FosFilterComposite fosFilterComposite(fosStatistic);
    FosFilterDivisible fosFilterDivisible(1);
    fosFilterComposite.addFilter(fosFilterDivisible);
    FosGeneratorMock fosGeneratorMock(*trx, fosFilterComposite);
    fosGeneratorMock.collectSops(sopByLeg);

    uint32_t sopIdx = 0;
    SopCombination sopComb;
    while (sopIdx < sopVec.size() &&
           fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb))
    {
      CPPUNIT_ASSERT_EQUAL(sopVec[sopIdx], sopComb[0]);
      ++sopIdx;
    }
    CPPUNIT_ASSERT_EQUAL(6u, sopIdx);
  }

  void getNextCombinationOneFilterTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    int sopIds[] = { 1, 2, 34, 11, 25, 8, 9 };
    std::vector<int> sopVec(sopIds, sopIds + sizeof(sopIds) / sizeof(int));
    std::vector<std::vector<int> > sopByLeg;
    sopByLeg.insert(sopByLeg.begin(), sopVec);

    FosStatistic fosStatistic(*trx);
    FosFilterComposite fosFilterComposite(fosStatistic);
    FosFilterDivisible fosFilterDivisibleByFive(5);
    fosFilterComposite.addFilter(fosFilterDivisibleByFive);

    FosGeneratorMock fosGeneratorMock(*trx, fosFilterComposite);
    fosGeneratorMock.collectSops(sopByLeg);

    SopCombination sopComb;
    CPPUNIT_ASSERT(fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb));
    CPPUNIT_ASSERT_EQUAL(25, sopComb[0]);
    CPPUNIT_ASSERT(!fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb));
  }

  void getNextCombinationTwoFilterTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    int sopIds[] = { 1, 3, 4, 5, 8, 9, 20, 17, 11, 12 };
    std::vector<int> sopVec(sopIds, sopIds + sizeof(sopIds) / sizeof(int));
    std::vector<std::vector<int> > sopByLeg;
    sopByLeg.insert(sopByLeg.begin(), sopVec);

    FosStatistic fosStatistic(*trx);
    FosFilterComposite fosFilterComposite(fosStatistic);
    FosFilterDivisible fosFilterDivisibleByFive(5);
    fosFilterComposite.addFilter(fosFilterDivisibleByFive);

    FosFilterDivisible fosFilterDivisibleByThree(3);
    fosFilterComposite.addFilter(fosFilterDivisibleByThree);

    FosGeneratorMock fosGeneratorMock(*trx, fosFilterComposite);
    fosGeneratorMock.collectSops(sopByLeg);

    SopCombination sopComb;
    for (int i = 0; i < 2; ++i)
    {
      CPPUNIT_ASSERT(fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb));
      CPPUNIT_ASSERT(!(sopComb[0] % 5));
    }
    for (int i = 0; i < 3; ++i)
    {
      CPPUNIT_ASSERT(fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb));
      CPPUNIT_ASSERT(!(sopComb[0] % 3));
    }
    for (int i = 0; i < 5; ++i)
    {
      if (!fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb))
        CPPUNIT_ASSERT((sopComb[0] % 3) && (sopComb[0] % 5));
    }
    CPPUNIT_ASSERT(!fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb));
  }

  void generateSopDetailsTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    PricingOrchestratorTestShoppingCommon::createSOP(*dataHandle, trx, trx->legs()[0], 0, "BA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, trx->legs()[0], "SYD", "FRA", "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, trx->legs()[0], "FRA", "WAW", "LH");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, trx->legs()[0], "WAW", "KRK", "LO", DateTime::localTime().addDays(1));

    PricingOrchestratorTestShoppingCommon::createSOP(*dataHandle, trx, trx->legs()[0], 1, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, trx->legs()[0], "SYD", "FRA", "AA", DateTime::localTime().addDays(7));
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, trx->legs()[0], "FRA", "KRK", "BA", DateTime::localTime().addDays(8));

    FosStatistic fosStatistic(*trx);
    FosFilterComposite fosFilterComposite(fosStatistic);
    FosGeneratorMock fosGeneratorMock(*trx, fosFilterComposite);

    DetailedSop* detailedSop = fosGeneratorMock.generateSopDetails(0, 0);
    const SopDetailsPtrVec& sopDetails = detailedSop->getSopDetailsVec();
    CPPUNIT_ASSERT(sopDetails.size() == 2);
    std::string det1[] = { "SYDFRA", "FRAKRK", "BA", "LH", "FRA" };
    std::vector<std::string> det1Vec(det1, det1 + sizeof(det1) / sizeof(std::string));
    std::string det2[] = { "SYDWAW", "WAWKRK", "BA", "LO", "WAW" };
    std::vector<std::string> det2Vec(det2, det2 + sizeof(det2) / sizeof(std::string));
    std::vector<std::vector<std::string> > details;

    details.push_back(det1Vec);
    details.push_back(det2Vec);
    int idx = 0;
    for (SopDetailsPtrVec::const_iterator iter = sopDetails.begin(); iter < sopDetails.end();
         ++iter, ++idx)
    {
      CPPUNIT_ASSERT((*iter)->fareMarketOD[0] == details[idx][0]);
      CPPUNIT_ASSERT((*iter)->fareMarketOD[1] == details[idx][1]);
      CPPUNIT_ASSERT((*iter)->cxrCode[0] == details[idx][2]);
      CPPUNIT_ASSERT((*iter)->cxrCode[1] == details[idx][3]);
      CPPUNIT_ASSERT((*iter)->destAirport == details[idx][4]);
    }
  }

  void getSopDetailsTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    PricingOrchestratorTestShoppingCommon::createSOP(*dataHandle, trx, trx->legs()[0], 0, "BA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, trx->legs()[0], "SYD", "FRA", "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, trx->legs()[0], "FRA", "WAW", "LH");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, trx->legs()[0], "WAW", "KRK", "LO", DateTime::localTime().addDays(1));

    FosStatistic fosStatistic(*trx);
    FosFilterComposite fosFilterComposite(fosStatistic);
    FosFilterCxnCity fosFilterCxnCityFRA(*trx, "FRA");
    fosFilterComposite.addFilter(fosFilterCxnCityFRA);
    FosGeneratorMock fosGeneratorMock(*trx, fosFilterComposite);

    SopDetailsPtrVec sopDetails = fosGeneratorMock.getSopDetails(0, 0);
    CPPUNIT_ASSERT(sopDetails.size() == 1);
    CPPUNIT_ASSERT(sopDetails[0]->destAirport == "FRA");
    fosFilterComposite.pop();
    sopDetails = fosGeneratorMock.getSopDetails(0, 0);
    CPPUNIT_ASSERT(sopDetails.size() == 2);
  }

  void addCombinationForReuseTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    int sopIds[] = { 1, 2, 3, 4, 5, 6 };
    std::vector<int> sopVec(sopIds, sopIds + sizeof(sopIds) / sizeof(int));
    std::vector<std::vector<int> > sopByLeg;
    sopByLeg.insert(sopByLeg.begin(), sopVec);

    FosStatistic fosStatistic(*trx);
    FosFilterComposite fosFilterComposite(fosStatistic);
    FosFilterCombReuse fosFilterCombReuse;
    fosFilterComposite.addFilter(fosFilterCombReuse);
    fosFilterComposite.addFilter(fosFilterCombReuse);
    FosGeneratorMock fosGeneratorMock(*trx, fosFilterComposite);
    fosGeneratorMock.collectSops(sopByLeg);

    SopCombination sopComb;
    fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb);
    fosGeneratorMock.addCombinationForReuse(sopComb);
    bool foundReusedComb = false;
    SopCombination sopComb2;

    while (fosGeneratorMock.getNextCombination(~ValidatorBitMask(0), sopComb2))
    {
      if (sopComb[0] == sopComb2[0])
        foundReusedComb = true;
    }
    CPPUNIT_ASSERT(foundReusedComb);
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

private:
  ShoppingTrx* trx;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FosGeneratorTest);
} // fos
} // tse
