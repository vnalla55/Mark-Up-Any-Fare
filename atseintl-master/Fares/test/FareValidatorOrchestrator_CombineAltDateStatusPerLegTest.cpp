#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace boost::assign;

class FareValidatorOrchestrator_CombineAltDateStatusPerLegTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_CombineAltDateStatusPerLegTest);
  CPPUNIT_TEST(testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_Empty);
  CPPUNIT_TEST(testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_OneWay);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_RoundTrip_FirstLegIsValid);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_RoundTrip_SecondLegIsValid);
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

  void testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_Empty()
  {
    std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo> > combinedAltDateStatusPerLeg;
    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedCombinedAltDateStatus;

    _fvo->combineAltDateStatusPerLeg(combinedCombinedAltDateStatus,
                                     combinedAltDateStatusPerLeg,
                                     emptyPaxTypeFarePassedForRequestedDateVect);

    CPPUNIT_ASSERT_EQUAL(combinedAltDateStatusPerLeg.size(), size_t(0));
  }

  void testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_OneWay()
  {
    std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo> > combinedAltDateStatusPerLeg;

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

    addCombinedAltDateStatusPerLeg(combinedAltDateStatusPerLeg, mapDTStatus);

    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedCombinedAltDateStatus;
    _fvo->combineAltDateStatusPerLeg(combinedCombinedAltDateStatus,
                                     combinedAltDateStatusPerLeg,
                                     emptyPaxTypeFarePassedForRequestedDateVect);

    CPPUNIT_ASSERT_EQUAL(combinedAltDateStatusPerLeg.size(), size_t(1));

    std::vector<uint8_t> expectedcombinedAltDateStatus;
    expectedcombinedAltDateStatus += 0, 0, 1, 1;
    assertCombinedAltDateStatusBits(
        expectedcombinedAltDateStatus, combinedCombinedAltDateStatus, vectDT);
  }

  void testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_RoundTrip_FirstLegIsValid()
  {
    std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo> > combinedAltDateStatusPerLeg;

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

    std::map<DatePair, FlightFinderTrx::FlightBitInfo> emptyCombinedAltDateStatusForFirstLeg;
    combinedAltDateStatusPerLeg.push_back(emptyCombinedAltDateStatusForFirstLeg);
    addCombinedAltDateStatusPerLeg(combinedAltDateStatusPerLeg, mapDTStatus);

    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedCombinedAltDateStatus;
    _fvo->combineAltDateStatusPerLeg(combinedCombinedAltDateStatus,
                                     combinedAltDateStatusPerLeg,
                                     emptyPaxTypeFarePassedForRequestedDateVect);

    CPPUNIT_ASSERT_EQUAL(combinedAltDateStatusPerLeg.size(), size_t(2));

    std::vector<uint8_t> expectedcombinedAltDateStatus;
    expectedcombinedAltDateStatus += 0, 0, 1, 1;
    assertCombinedAltDateStatusBits(
        expectedcombinedAltDateStatus, combinedCombinedAltDateStatus, vectDT);
  }

  void testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_RoundTrip_SecondLegIsValid()
  {
    std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo> > combinedAltDateStatusPerLeg;

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

    addCombinedAltDateStatusPerLeg(combinedAltDateStatusPerLeg, mapDTStatus);
    std::map<DatePair, FlightFinderTrx::FlightBitInfo> emptyCombinedAltDateStatusForSecondLeg;
    combinedAltDateStatusPerLeg.push_back(emptyCombinedAltDateStatusForSecondLeg);

    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedCombinedAltDateStatus;
    _fvo->combineAltDateStatusPerLeg(combinedCombinedAltDateStatus,
                                     combinedAltDateStatusPerLeg,
                                     emptyPaxTypeFarePassedForRequestedDateVect);

    CPPUNIT_ASSERT_EQUAL(combinedAltDateStatusPerLeg.size(), size_t(2));

    std::vector<uint8_t> expectedcombinedAltDateStatus;
    expectedcombinedAltDateStatus += 0, 0, 1, 1;
    assertCombinedAltDateStatusBits(
        expectedcombinedAltDateStatus, combinedCombinedAltDateStatus, vectDT);
  }

  void testFareValidatorOrchestrator_CombineAltDateStatusPerLegTest_RoundTrip()
  {
    std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo> > combinedAltDateStatusPerLeg;

    std::vector<DatePair> vectDT;
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 20), DateTime(2007, 12, 22)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 22), DateTime(2007, 12, 23)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 24), DateTime(2007, 12, 25)));
    vectDT.push_back(std::make_pair(DateTime(2007, 12, 26), DateTime(2007, 12, 28)));

    std::map<DatePair, uint8_t> mapDTStatus_FirstLeg;
    mapDTStatus_FirstLeg[vectDT[0]] = 0;
    mapDTStatus_FirstLeg[vectDT[1]] = 0;
    mapDTStatus_FirstLeg[vectDT[2]] = 1;
    mapDTStatus_FirstLeg[vectDT[3]] = 1;

    addCombinedAltDateStatusPerLeg(combinedAltDateStatusPerLeg, mapDTStatus_FirstLeg);

    std::map<DatePair, uint8_t> mapDTStatus_SecondLeg;
    mapDTStatus_SecondLeg[vectDT[0]] = 1;
    mapDTStatus_SecondLeg[vectDT[1]] = 0;
    mapDTStatus_SecondLeg[vectDT[2]] = 1;
    mapDTStatus_SecondLeg[vectDT[3]] = 0;

    addCombinedAltDateStatusPerLeg(combinedAltDateStatusPerLeg, mapDTStatus_SecondLeg);

    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedCombinedAltDateStatus;
    _fvo->combineAltDateStatusPerLeg(combinedCombinedAltDateStatus,
                                     combinedAltDateStatusPerLeg,
                                     emptyPaxTypeFarePassedForRequestedDateVect);

    CPPUNIT_ASSERT_EQUAL(combinedAltDateStatusPerLeg.size(), size_t(2));

    std::vector<uint8_t> expectedcombinedAltDateStatus;
    expectedcombinedAltDateStatus += 1, 0, 1, 1;
    assertCombinedAltDateStatusBits(
        expectedcombinedAltDateStatus, combinedCombinedAltDateStatus, vectDT);
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

  void addCombinedAltDateStatusPerLeg(
      std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo> >& combinedAltDateStatusPerLeg,
      std::map<DatePair, uint8_t>& mapDTStatus)
  {
    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedFlightBitmap;

    std::map<DatePair, uint8_t>::const_iterator it = mapDTStatus.begin();
    for (; it != mapDTStatus.end(); ++it)
    {
      FlightFinderTrx::FlightBitInfo fBitInfo;
      fBitInfo.flightBitStatus = it->second;
      combinedFlightBitmap[it->first] = fBitInfo;
    }

    combinedAltDateStatusPerLeg.push_back(combinedFlightBitmap);
  }

protected:
  FareValidatorOrchestratorDerived* _fvo;
  std::vector<PaxTypeFare*> emptyPaxTypeFarePassedForRequestedDateVect;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareValidatorOrchestrator_CombineAltDateStatusPerLegTest);

} // tse
