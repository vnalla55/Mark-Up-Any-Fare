#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "test/include/TestMemHandle.h"
namespace tse
{

using namespace boost::assign;

class FareValidatorOrchestrator_CombineAltDateStatusTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_CombineAltDateStatusTest);
  CPPUNIT_TEST(testFareValidatorOrchestrator_CombineAltDateStatusTest_emptyAltDateStatus);
  CPPUNIT_TEST(testFareValidatorOrchestrator_CombineAltDateStatusTest_onlyOnePaxTypeFare);
  CPPUNIT_TEST(testFareValidatorOrchestrator_CombineAltDateStatusTest_combineCorrectness);
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

  void testFareValidatorOrchestrator_CombineAltDateStatusTest_emptyAltDateStatus()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);
    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedAltDateStatus;

    goThroughAllPaxTypeFaresWithMethod_combineAltDateStatus(_fvo, combinedAltDateStatus, fM);

    CPPUNIT_ASSERT(combinedAltDateStatus.empty());
  }

  void testFareValidatorOrchestrator_CombineAltDateStatusTest_onlyOnePaxTypeFare()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);
    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedAltDateStatus;

    std::vector<DatePair> vectDT;
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 20), DateTime(2007, 12, 22)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 22), DateTime(2007, 12, 23)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 24), DateTime(2007, 12, 25)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 26), DateTime(2007, 12, 28)));

    std::map<DatePair, uint8_t> mapDTStatus;
    mapDTStatus[vectDT[0]] = 0;
    mapDTStatus[vectDT[1]] = 0;
    mapDTStatus[vectDT[2]] = 1;
    mapDTStatus[vectDT[3]] = 1;

    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithAltDataStatus(mapDTStatus));

    goThroughAllPaxTypeFaresWithMethod_combineAltDateStatus(_fvo, combinedAltDateStatus, fM);

    // check size of combinedAltDateStatus
    CPPUNIT_ASSERT_EQUAL(combinedAltDateStatus.size(), size_t(4));

    std::vector<uint8_t> expectedcombinedAltDateStatus;
    expectedcombinedAltDateStatus += 0, 0, 1, 1;
    assertCombinedAltDateStatusBits(expectedcombinedAltDateStatus, combinedAltDateStatus, vectDT);
  }

  void testFareValidatorOrchestrator_CombineAltDateStatusTest_combineCorrectness()
  {
    FareMarket* fM = 0;
    _memHandle.get(fM);
    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedAltDateStatus;

    std::vector<DatePair> vectDT;
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 20), DateTime(2007, 12, 22)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 22), DateTime(2007, 12, 23)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 24), DateTime(2007, 12, 25)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 26), DateTime(2007, 12, 28)));

    std::map<DatePair, uint8_t> mapDTStatus1;
    mapDTStatus1[vectDT[0]] = 0;
    mapDTStatus1[vectDT[1]] = 0;
    mapDTStatus1[vectDT[2]] = 1;
    mapDTStatus1[vectDT[3]] = 1;
    std::map<DatePair, uint8_t> mapDTStatus2;
    mapDTStatus2[vectDT[0]] = 0;
    mapDTStatus2[vectDT[1]] = 1;
    mapDTStatus2[vectDT[2]] = 0;
    mapDTStatus2[vectDT[3]] = 1;

    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithAltDataStatus(mapDTStatus1));
    fM->allPaxTypeFare().push_back(buildPaxTypeFareWithAltDataStatus(mapDTStatus2));

    goThroughAllPaxTypeFaresWithMethod_combineAltDateStatus(_fvo, combinedAltDateStatus, fM);

    // check size of combinedAltDateStatus
    CPPUNIT_ASSERT_EQUAL(combinedAltDateStatus.size(), size_t(4));

    std::vector<uint8_t> expectedcombinedAltDateStatus;
    expectedcombinedAltDateStatus += 0, 0, 0, 1;
    assertCombinedAltDateStatusBits(expectedcombinedAltDateStatus, combinedAltDateStatus, vectDT);
  }

protected:
  void assertCombinedAltDateStatusBits(
      std::vector<uint8_t>& expectedcombinedAltDateStatus,
      std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDateStatus,
      std::vector<DatePair>& vectDT)
  {
    for (size_t i = 0; i < expectedcombinedAltDateStatus.size(); i++)
      CPPUNIT_ASSERT_EQUAL(expectedcombinedAltDateStatus[i],
                           combinedAltDateStatus[vectDT[i]].flightBitStatus);
  }

  void goThroughAllPaxTypeFaresWithMethod_combineAltDateStatus(
      FareValidatorOrchestratorDerived* _fvo,
      std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDateStatus,
      FareMarket* fM)
  {
    std::vector<PaxTypeFare*>::iterator pTFIter = fM->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator pTFEndIter = fM->allPaxTypeFare().end();

    for (; pTFIter != pTFEndIter; ++pTFIter)
    {
      PaxTypeFare*& curFare = static_cast<PaxTypeFare*&>(*pTFIter);

      if (curFare)
      {
        _fvo->combineAltDateStatus(combinedAltDateStatus, curFare, 0);
      }
    }
  }

  PaxTypeFare* buildPaxTypeFareWithAltDataStatus(std::map<DatePair, uint8_t>& mapDTStatus)
  {
    PaxTypeFare* paxTypeFare = 0;
    _memHandle.get(paxTypeFare);

    std::map<DatePair, uint8_t>::const_iterator it = mapDTStatus.begin();
    for (; it != mapDTStatus.end(); ++it)
    {
      paxTypeFare->altDateStatus()[it->first] = it->second;
    }

    return paxTypeFare;
  }

protected:
  FareValidatorOrchestratorDerived* _fvo;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareValidatorOrchestrator_CombineAltDateStatusTest);

} // tse
