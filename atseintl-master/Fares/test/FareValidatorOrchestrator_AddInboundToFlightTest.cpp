#include "test/include/CppUnitHelperMacros.h"
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "DataModel/FlightFinderTrx.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class FareValidatorOrchestrator_AddInboundToFlightTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_AddInboundToFlightTest);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddInboundToFlightTest_addForNonExistingOutbound);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddInboundToFlightTest_addForExistingOutbound);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_AddInboundToFlightTest_addNextOutboundForExistingOutbound);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_AddInboundToFlightTest_addAlreadyExistingSOPForInbound);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.insert(_fvo = new FareValidatorOrchestratorDerived("FVO", *_memHandle.create<MockTseServer>()));
    _fFTrx = _memHandle.create<FlightFinderTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void testFareValidatorOrchestrator_AddInboundToFlightTest_addForNonExistingOutbound()
  {
    DateTime outboundDepartureDT(2007, 9, 19);
    DateTime inboundDepartureDT(2007, 9, 20);
    _fvo->addInboundToFlightList(
        *_fFTrx, outboundDepartureDT, inboundDepartureDT, 40, paxTypeFareVect, bkgCodes);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().size(), size_t(0));
  }

  void testFareValidatorOrchestrator_AddInboundToFlightTest_addForExistingOutbound()
  {
    DateTime outboundDepartureDT(2007, 9, 19);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDT, 1, paxTypeFareVect, bkgCodes);
    DateTime inboundDepartureDT(2007, 9, 20);
    // add first inbound
    _fvo->addInboundToFlightList(
        *_fFTrx, outboundDepartureDT, inboundDepartureDT, 40, paxTypeFareVect, bkgCodes);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().size(), size_t(1));
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().count(outboundDepartureDT), size_t(1));
    CPPUNIT_ASSERT_EQUAL(getSizeOfInboundFlightList(outboundDepartureDT, inboundDepartureDT),
                         uint32_t(1));
  }

  void testFareValidatorOrchestrator_AddInboundToFlightTest_addNextOutboundForExistingOutbound()
  {
    DateTime outboundDepartureDT(2007, 9, 19);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDT, 1, paxTypeFareVect, bkgCodes);
    DateTime inboundDepartureDTFirst(2007, 9, 20);
    DateTime inboundDepartureDTSecond(2007, 9, 21);
    _fvo->addInboundToFlightList(
        *_fFTrx, outboundDepartureDT, inboundDepartureDTFirst, 40, paxTypeFareVect, bkgCodes);
    // add second inbound
    _fvo->addInboundToFlightList(
        *_fFTrx, outboundDepartureDT, inboundDepartureDTSecond, 30, paxTypeFareVect, bkgCodes);

    CPPUNIT_ASSERT_EQUAL(getSizeOfInboundFlightList(outboundDepartureDT, inboundDepartureDTFirst),
                         uint32_t(1));
    CPPUNIT_ASSERT_EQUAL(getSizeOfInboundFlightList(outboundDepartureDT, inboundDepartureDTSecond),
                         uint32_t(1));
  }

  void testFareValidatorOrchestrator_AddInboundToFlightTest_addAlreadyExistingSOPForInbound()
  {
    DateTime outboundDepartureDT(2007, 9, 19);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDT, 1, paxTypeFareVect, bkgCodes);
    DateTime inboundDepartureDT(2007, 9, 20);
    _fvo->addInboundToFlightList(
        *_fFTrx, outboundDepartureDT, inboundDepartureDT, 40, paxTypeFareVect, bkgCodes);
    // add second SOP
    _fvo->addInboundToFlightList(
        *_fFTrx, outboundDepartureDT, inboundDepartureDT, 30, paxTypeFareVect, bkgCodes);

    CPPUNIT_ASSERT_EQUAL(getSizeOfInboundFlightList(outboundDepartureDT, inboundDepartureDT),
                         uint32_t(2));
  }

protected:
  uint32_t getSizeOfInboundFlightList(DateTime& outboundDepartureDT, DateTime& inboundDepartureDT)
  {
    uint32_t inboundFlightListSize = 0;

    if (_fFTrx->outboundDateflightMap().count(outboundDepartureDT) > 0)
    {
      if ((_fFTrx->outboundDateflightMap())[outboundDepartureDT]->iBDateFlightMap.count(
              inboundDepartureDT) > 0)
      {
        inboundFlightListSize = ((_fFTrx->outboundDateflightMap())[outboundDepartureDT]
                                     ->iBDateFlightMap)[inboundDepartureDT]->flightList.size();
      }
    }

    return inboundFlightListSize;
  }

protected:
  FlightFinderTrx* _fFTrx;
  FareValidatorOrchestratorDerived* _fvo;
  std::vector<PaxTypeFare*> paxTypeFareVect;
  std::vector<std::vector<FlightFinderTrx::BookingCodeData> > bkgCodes;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareValidatorOrchestrator_AddInboundToFlightTest);

} // tse
