#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace boost::assign;

class FareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest
    : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest_emptyFlightBitMaps);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest_differentErrorStatuses);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest_combineCorrectness);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest_onlyOneFlightBitmap);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.insert(_fvo = new FareValidatorOrchestratorDerived("FVO", *_memHandle.create<MockTseServer>()));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testFareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest_emptyFlightBitMaps()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);
    std::vector<FlightFinderTrx::FlightBitInfo> combinedFlightBitmap;
    std::vector<uint8_t> bits;
    size_t sizeOfBitMap = 0;

    std::vector<uint8_t> bitsFirst;
    std::vector<uint8_t> bitsSecond;

    // add PaxTypeFares with empty flight bitmaps
    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithFlightBitmap(bitsFirst));
    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithFlightBitmap(bitsSecond));

    goThroughAllPaxTypeFaresWithMethod_combineFlightBitmapsForEachPaxTypeFare(
        _fvo, combinedFlightBitmap, fM);

    // check size of combinedFlightBitmap
    CPPUNIT_ASSERT_EQUAL(combinedFlightBitmap.size(), sizeOfBitMap);
  }

  void
  testFareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest_differentErrorStatuses()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);
    std::vector<FlightFinderTrx::FlightBitInfo> combinedFlightBitmap;
    std::vector<uint8_t> bits;
    size_t sizeOfBitMap = 4;

    std::vector<uint8_t> bitsFirst;
    bitsFirst += 'G', 'D', 'F', 0;
    std::vector<uint8_t> bitsSecond;
    bitsSecond += 0, 0, 'D', 'D';

    // add PaxTypeFares with flight bitmaps GDF0 and 00DD
    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithFlightBitmap(bitsFirst));
    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithFlightBitmap(bitsSecond));

    goThroughAllPaxTypeFaresWithMethod_combineFlightBitmapsForEachPaxTypeFare(
        _fvo, combinedFlightBitmap, fM);

    // check size of combinedFlightBitmap
    CPPUNIT_ASSERT_EQUAL(combinedFlightBitmap.size(), sizeOfBitMap);

    // check correctness of combinedFlightBitmap, should be 00F0
    std::vector<uint8_t> expectedCombinedFlightBitmap;
    expectedCombinedFlightBitmap += 0, 0, 'F', 0;
    assertCombinedFlightBitmap(expectedCombinedFlightBitmap, combinedFlightBitmap);
  }

  void testFareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest_combineCorrectness()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);
    std::vector<FlightFinderTrx::FlightBitInfo> combinedFlightBitmap;
    std::vector<uint8_t> bits;
    size_t sizeOfBitMap = 4;

    std::vector<uint8_t> bitsFirst;
    bitsFirst += 1, 0, 1, 0;
    std::vector<uint8_t> bitsSecond;
    bitsSecond += 0, 0, 1, 1;

    // add PaxTypeFares with flight bitmaps 1010 and 0011
    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithFlightBitmap(bitsFirst));
    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithFlightBitmap(bitsSecond));

    goThroughAllPaxTypeFaresWithMethod_combineFlightBitmapsForEachPaxTypeFare(
        _fvo, combinedFlightBitmap, fM);

    // check size of combinedFlightBitmap
    CPPUNIT_ASSERT_EQUAL(combinedFlightBitmap.size(), sizeOfBitMap);

    std::vector<uint8_t> expectedCombinedFlightBitmap;
    expectedCombinedFlightBitmap += 0, 0, 1, 0;
    assertCombinedFlightBitmap(expectedCombinedFlightBitmap, combinedFlightBitmap);
  }

  void testFareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest_onlyOneFlightBitmap()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);
    std::vector<FlightFinderTrx::FlightBitInfo> combinedFlightBitmap;
    std::vector<std::vector<uint8_t> > bits;

    size_t sizeOfBitMap = 4;

    std::vector<uint8_t> bitsFirst;
    bitsFirst += 1, 0, 1, 0;

    // add PaxTypeFares with flight bitmaps 1010 and empty one
    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithFlightBitmap(bitsFirst));

    goThroughAllPaxTypeFaresWithMethod_combineFlightBitmapsForEachPaxTypeFare(
        _fvo, combinedFlightBitmap, fM);

    // check size of combinedFlightBitmap
    CPPUNIT_ASSERT_EQUAL(combinedFlightBitmap.size(), sizeOfBitMap);

    std::vector<uint8_t> expectedCombinedFlightBitmap;
    expectedCombinedFlightBitmap += 1, 0, 1, 0;
    assertCombinedFlightBitmap(expectedCombinedFlightBitmap, combinedFlightBitmap);
  }

protected:
  void assertCombinedFlightBitmap(std::vector<uint8_t>& expectedCombinedFlightBitmap,
                                  std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap)
  {
    for (size_t i = 0; i < expectedCombinedFlightBitmap.size(); i++)
      CPPUNIT_ASSERT_EQUAL(expectedCombinedFlightBitmap[i],
                           combinedFlightBitmap[i].flightBitStatus);
  }

  void goThroughAllPaxTypeFaresWithMethod_combineFlightBitmapsForEachPaxTypeFare(
      FareValidatorOrchestratorDerived* _fvo,
      std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap,
      FareMarket* fM)
  {
    std::vector<PaxTypeFare*>::iterator pTFIter = fM->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator pTFEndIter = fM->allPaxTypeFare().end();

    for (; pTFIter != pTFEndIter; ++pTFIter)
    {
      PaxTypeFare*& curFare = static_cast<PaxTypeFare*&>(*pTFIter);

      if (curFare)
      {
        _fvo->combineFlightBitmapsForEachPaxTypeFare(
            combinedFlightBitmap, curFare, curFare->flightBitmap(), 0);
      }
    }
  }

  PaxTypeFare* buildPaxTypeFareWithFlightBitmap(const std::vector<uint8_t>& bits)
  {
    PaxTypeFare* paxTypeFare = 0;
    _memHandle.get(paxTypeFare);

    PaxTypeFare::FlightBit bit;
    uint32_t bitNo = 0;
    for (; bitNo < bits.size(); ++bitNo)
    {
      bit._flightBit = bits[bitNo];
      paxTypeFare->flightBitmap().push_back(bit);
    }

    paxTypeFare->setFlightBitmapSize(bitNo);

    return paxTypeFare;
  }

protected:
  FareValidatorOrchestratorDerived* _fvo;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(
    FareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest);

} // tse
