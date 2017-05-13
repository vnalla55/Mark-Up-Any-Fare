#include "test/include/CppUnitHelperMacros.h"
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "DataModel/FlightFinderTrx.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class FareValidatorOrchestrator_AddAltDateToFlightListTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_AddAltDateToFlightListTest);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddAltDateToFlightListTest_addToEmptyList);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddAltDateToFlightListTest_addToExistingOutbound);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddAltDateToFlightListTest_addExistingInbound);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddAltDateToFlightListTest_addNextOutbound);
  CPPUNIT_TEST(testFareValidatorOrchestrator_AddAltDateToFlightListTest_addInboundToNextOutbound);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.insert(_fvo = new FareValidatorOrchestratorDerived("FVO", *_memHandle.create<MockTseServer>()));
    _fFTrx = _memHandle.create<FlightFinderTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void testFareValidatorOrchestrator_AddAltDateToFlightListTest_addToEmptyList()
  {
    DateTime outboundDepartureDT(2007, 9, 19);
    DateTime inboundDepartureDT(2007, 9, 20);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDT,
                                 inboundDepartureDT,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);

    CPPUNIT_ASSERT(isOutboundAddInboundInList(outboundDepartureDT, inboundDepartureDT));
  }

  void testFareValidatorOrchestrator_AddAltDateToFlightListTest_addToExistingOutbound()
  {
    DateTime outboundDepartureDT(2007, 9, 19);
    DateTime inboundDepartureDTFirst(2007, 9, 20);
    DateTime inboundDepartureDTSecond(2007, 9, 21);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDT,
                                 inboundDepartureDTFirst,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDT,
                                 inboundDepartureDTSecond,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);

    CPPUNIT_ASSERT(isOutboundAddInboundInList(outboundDepartureDT, inboundDepartureDTSecond));
  }

  void testFareValidatorOrchestrator_AddAltDateToFlightListTest_addExistingInbound()
  {
    DateTime outboundDepartureDTFirst(2007, 9, 18);
    DateTime outboundDepartureDTSecond(2007, 9, 19);
    DateTime inboundDepartureDTFirst(2007, 9, 20);
    DateTime inboundDepartureDTSecond(2007, 9, 21);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDTFirst,
                                 inboundDepartureDTFirst,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDTSecond,
                                 inboundDepartureDTSecond,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);

    CPPUNIT_ASSERT(isOutboundAddInboundInList(outboundDepartureDTSecond, inboundDepartureDTSecond));
  }

  void testFareValidatorOrchestrator_AddAltDateToFlightListTest_addNextOutbound()
  {
    DateTime outboundDepartureDTFirst(2007, 9, 18);
    DateTime outboundDepartureDTSecond(2007, 9, 19);
    DateTime inboundDepartureDTFirst(2007, 9, 20);
    DateTime inboundDepartureDTSecond(2007, 9, 21);
    DateTime inboundDepartureDTThird(2007, 9, 22);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDTFirst,
                                 inboundDepartureDTFirst,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDTSecond,
                                 inboundDepartureDTSecond,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDTSecond,
                                 inboundDepartureDTThird,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);

    CPPUNIT_ASSERT(isOutboundAddInboundInList(outboundDepartureDTSecond, inboundDepartureDTThird));
  }

  void testFareValidatorOrchestrator_AddAltDateToFlightListTest_addInboundToNextOutbound()
  {
    DateTime outboundDepartureDT(2007, 9, 19);
    DateTime inboundDepartureDT(2007, 9, 20);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDT,
                                 inboundDepartureDT,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);
    _fvo->addAltDateToFlightList(*_fFTrx,
                                 outboundDepartureDT,
                                 inboundDepartureDT,
                                 emptyOutboundFareVect,
                                 emptyInboundFareVect);

    CPPUNIT_ASSERT_EQUAL(_fFTrx->outboundDateflightMap().size(), size_t(1));
    CPPUNIT_ASSERT_EQUAL(
        _fFTrx->outboundDateflightMap()[outboundDepartureDT]->iBDateFlightMap.size(), size_t(1));
  }

protected:
  bool isOutboundAddInboundInList(DateTime& outboundDepartureDT, DateTime& inboundDepartureDT)
  {
    if (!_fFTrx->outboundDateflightMap().empty())
    {
      if (_fFTrx->outboundDateflightMap().count(outboundDepartureDT) == 1)
      {
        if (!_fFTrx->outboundDateflightMap()[outboundDepartureDT]->iBDateFlightMap.empty())
        {
          if (_fFTrx->outboundDateflightMap()[outboundDepartureDT]->iBDateFlightMap.count(
                  inboundDepartureDT) == 1)
          {
            return true;
          }
        }
      }
    }

    return false;
  }

protected:
  FlightFinderTrx* _fFTrx;
  FareValidatorOrchestratorDerived* _fvo;
  std::vector<PaxTypeFare*> emptyOutboundFareVect;
  std::vector<PaxTypeFare*> emptyInboundFareVect;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareValidatorOrchestrator_AddAltDateToFlightListTest);

} // tse
