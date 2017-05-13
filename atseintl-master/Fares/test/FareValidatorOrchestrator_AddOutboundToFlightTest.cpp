#include "test/include/CppUnitHelperMacros.h"
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "DataModel/FlightFinderTrx.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class FareValidatorOrchestrator_AddOutboundToFlightTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_AddOutboundToFlightTest);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddOutboundToFlightTest_addToEmtyList);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddOutboundToFlightTest_addNextOutbound);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddOutboundToFlightTest_addSOPforNonExistingOutbound);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddOutboundToFlightTest_addSOPforExistingOutbound);
  CPPUNIT_TEST(
      testFareValidatorOrchestrator_AddOutboundToFlightTest_addAlreadyExistingSOPforOutbound);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.insert(_fvo = new FareValidatorOrchestratorDerived("FVO", *_memHandle.create<MockTseServer>()));
    _fFTrx = _memHandle.create<FlightFinderTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void testFareValidatorOrchestrator_AddOutboundToFlightTest_addToEmtyList()
  {
    // check if data was properly added
    DateTime outboundDepartureDTFirst(2007, 12, 20);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDTFirst, 1, paxTypeFareVect, bkgCodes);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().size(), size_t(1));
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().count(outboundDepartureDTFirst), size_t(1));
  }

  void testFareValidatorOrchestrator_AddOutboundToFlightTest_addNextOutbound()
  {
    DateTime outboundDepartureDTFirst(2007, 10, 20);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDTFirst, 1, paxTypeFareVect, bkgCodes);

    // check if data was properly added
    DateTime outboundDepartureDTSecond(2007, 11, 20);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDTSecond, 1, paxTypeFareVect, bkgCodes);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().size(), size_t(2));
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().count(outboundDepartureDTSecond),
                         size_t(1));
  }

  void testFareValidatorOrchestrator_AddOutboundToFlightTest_addSOPforNonExistingOutbound()
  {
    // add sop for non existing outbound date
    DateTime outboundDepartureDT(2007, 9, 20);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDT, 1, paxTypeFareVect, bkgCodes);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().size(), size_t(1));
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().count(outboundDepartureDT), size_t(1));
    CPPUNIT_ASSERT_EQUAL(getSizeOfOutboundFlightList(outboundDepartureDT), uint32_t(1));
  }

  void testFareValidatorOrchestrator_AddOutboundToFlightTest_addSOPforExistingOutbound()
  {
    // add sop for non existing outbound date
    DateTime outboundDepartureDT(2007, 9, 20);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDT, 1, paxTypeFareVect, bkgCodes);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDT, 2, paxTypeFareVect, bkgCodes);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().size(), size_t(1));
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().count(outboundDepartureDT), size_t(1));
    CPPUNIT_ASSERT_EQUAL(getSizeOfOutboundFlightList(outboundDepartureDT), uint32_t(2));
  }

  void testFareValidatorOrchestrator_AddOutboundToFlightTest_addAlreadyExistingSOPforOutbound()
  {
    // add sop for non existing outbound date
    DateTime outboundDepartureDT(2007, 9, 20);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDT, 1, paxTypeFareVect, bkgCodes);
    _fvo->addOutboundToFlightList(*_fFTrx, outboundDepartureDT, 1, paxTypeFareVect, bkgCodes);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().size(), size_t(1));
    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().count(outboundDepartureDT), size_t(1));
    CPPUNIT_ASSERT_EQUAL(getSizeOfOutboundFlightList(outboundDepartureDT), uint32_t(2));
  }

protected:
  uint32_t getSizeOfOutboundFlightList(DateTime& outboundDepartureDT)
  {
    uint32_t outboundFlightListSize = 0;

    if (_fFTrx->outboundDateflightMap().count(outboundDepartureDT) > 0)
    {
      outboundFlightListSize =
          _fFTrx->outboundDateflightMap()[outboundDepartureDT]->flightInfo.flightList.size();
    }

    return outboundFlightListSize;
  }

protected:
  FlightFinderTrx* _fFTrx;
  FareValidatorOrchestratorDerived* _fvo;
  std::vector<PaxTypeFare*> paxTypeFareVect;
  std::vector<std::vector<FlightFinderTrx::BookingCodeData> > bkgCodes;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareValidatorOrchestrator_AddOutboundToFlightTest);

} // tse
