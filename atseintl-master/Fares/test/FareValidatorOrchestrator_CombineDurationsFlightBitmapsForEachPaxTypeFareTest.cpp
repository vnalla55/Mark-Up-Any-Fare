#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace boost::assign;

class FareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest
    : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest_emptyDurationBitMaps);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest_oneDurationTwoPaxTypeFares);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest_combineCorrectness);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.insert(new FareValidatorOrchestratorDerived("FVO", *_memHandle.create<MockTseServer>()));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void
  testFareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest_emptyDurationBitMaps()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);

    PaxTypeFare* paxTypeFare1 = 0;
    _memHandle.get(paxTypeFare1);
    PaxTypeFare* paxTypeFare2 = 0;
    _memHandle.get(paxTypeFare2);

    fM->allPaxTypeFare().push_back(paxTypeFare1);
    fM->allPaxTypeFare().push_back(paxTypeFare2);

    std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo> > combinedDurationFlightBitmap;

    goThroughAllPaxTypeFaresWithMethod_combineDurationsFlightBitmapsForEachPaxTypeFare(
        _fvo, combinedDurationFlightBitmap, fM);

    // check size of combinedDurationFlightBitmap
    CPPUNIT_ASSERT(combinedDurationFlightBitmap.empty());
  }

  void
  testFareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest_oneDurationTwoPaxTypeFares()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);

    size_t sizeOfBitMap = 4;
    const uint32_t durationFirst = 1;

    std::vector<uint8_t> bits1;
    bits1 += 1, 0, 1, 0; // 1010
    std::vector<uint8_t> bits2;
    bits2 += 0, 0, 1, 1; // 0011

    PaxTypeFare* paxTypeFare1 = 0;
    _memHandle.get(paxTypeFare1);
    PaxTypeFare* paxTypeFare2 = 0;
    _memHandle.get(paxTypeFare2);

    addDurationToFaxTypeFare(paxTypeFare1, durationFirst, bits1);
    addDurationToFaxTypeFare(paxTypeFare2, durationFirst, bits2);

    fM->allPaxTypeFare().push_back(paxTypeFare1);
    fM->allPaxTypeFare().push_back(paxTypeFare2);

    std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo> > combinedDurationFlightBitmap;

    goThroughAllPaxTypeFaresWithMethod_combineDurationsFlightBitmapsForEachPaxTypeFare(
        _fvo, combinedDurationFlightBitmap, fM);

    // check size of combinedDurationFlightBitmap
    CPPUNIT_ASSERT_EQUAL(combinedDurationFlightBitmap[durationFirst].size(), sizeOfBitMap);

    std::vector<uint8_t> expectedCombinedDurationFlightBitmap;
    expectedCombinedDurationFlightBitmap += 0, 0, 1, 0;
    assertCombinedFlightBitmapBits(expectedCombinedDurationFlightBitmap,
                                   combinedDurationFlightBitmap[durationFirst]);
  }

  void
  testFareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest_combineCorrectness()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);

    size_t sizeOfBitMap = 4;
    const uint32_t durationFirst = 1;
    const uint32_t durationSecond = 2;

    std::vector<uint8_t> bits1;
    bits1 += 1, 0, 1, 0; // 1010
    std::vector<uint8_t> bits2;
    bits2 += 0, 0, 1, 1; // 0011

    std::vector<uint8_t> bits3;
    bits3 += 1, 0, 0, 1; // 1001
    std::vector<uint8_t> bits4;
    bits4 += 1, 0, 1, 0; // 1010

    PaxTypeFare* paxTypeFare1 = 0;
    _memHandle.get(paxTypeFare1);
    PaxTypeFare* paxTypeFare2 = 0;
    _memHandle.get(paxTypeFare2);

    addDurationToFaxTypeFare(paxTypeFare1, durationFirst, bits1);
    addDurationToFaxTypeFare(paxTypeFare1, durationSecond, bits2);
    addDurationToFaxTypeFare(paxTypeFare2, durationFirst, bits3);
    addDurationToFaxTypeFare(paxTypeFare2, durationSecond, bits4);

    fM->allPaxTypeFare().push_back(paxTypeFare1);
    fM->allPaxTypeFare().push_back(paxTypeFare2);

    std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo> > combinedDurationFlightBitmap;

    goThroughAllPaxTypeFaresWithMethod_combineDurationsFlightBitmapsForEachPaxTypeFare(
        _fvo, combinedDurationFlightBitmap, fM);

    // check size of combinedDurationFlightBitmap
    CPPUNIT_ASSERT_EQUAL(combinedDurationFlightBitmap[durationFirst].size(), sizeOfBitMap);
    CPPUNIT_ASSERT_EQUAL(combinedDurationFlightBitmap[durationSecond].size(), sizeOfBitMap);

    std::vector<uint8_t> expectedCombinedDurationFlightBitmapFirst;
    expectedCombinedDurationFlightBitmapFirst += 1, 0, 0, 0;
    assertCombinedFlightBitmapBits(expectedCombinedDurationFlightBitmapFirst,
                                   combinedDurationFlightBitmap[durationFirst]);

    std::vector<uint8_t> expectedCombinedDurationFlightBitmapSecond;
    expectedCombinedDurationFlightBitmapSecond += 0, 0, 1, 0;
    assertCombinedFlightBitmapBits(expectedCombinedDurationFlightBitmapSecond,
                                   combinedDurationFlightBitmap[durationSecond]);
  }

protected:
  void
  assertCombinedFlightBitmapBits(std::vector<uint8_t>& expectedCombinedDurationFlightBitmap,
                                 std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap)
  {
    for (size_t i = 0; i < expectedCombinedDurationFlightBitmap.size(); i++)
      CPPUNIT_ASSERT_EQUAL(expectedCombinedDurationFlightBitmap[i],
                           combinedFlightBitmap[i].flightBitStatus);
  }

  void goThroughAllPaxTypeFaresWithMethod_combineDurationsFlightBitmapsForEachPaxTypeFare(
      FareValidatorOrchestratorDerived* _fvo,
      std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo> >&
          combinedDurationFlightBitmap,
      FareMarket* fM)
  {
    FlightFinderTrx ffTrx;
    std::vector<PaxTypeFare*>::iterator pTFIter = fM->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator pTFEndIter = fM->allPaxTypeFare().end();

    for (; pTFIter != pTFEndIter; ++pTFIter)
    {
      PaxTypeFare*& curFare = static_cast<PaxTypeFare*&>(*pTFIter);

      if (curFare)
      {
        _fvo->combineDurationsFlightBitmapsForEachPaxTypeFare(
            combinedDurationFlightBitmap, curFare, ffTrx);
      }
    }
  }

  void addDurationToFaxTypeFare(PaxTypeFare* paxTypeFare,
                                const uint32_t duration,
                                std::vector<uint8_t>& bits)
  {
    PaxTypeFare::FlightBitmap vectFligthBitmap;
    PaxTypeFare::FlightBit bit;

    for (uint32_t bitNo = 0; bitNo < bits.size(); ++bitNo)
    {
      bit._flightBit = bits[bitNo];
      vectFligthBitmap.push_back(bit);
    }

    paxTypeFare->durationFlightBitmap()[duration] = vectFligthBitmap;
  }

protected:
  FareValidatorOrchestratorDerived* _fvo;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(
    FareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest);

} // tse
