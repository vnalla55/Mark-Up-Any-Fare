// -------------------------------------------------------------------
//
//! \author       Natalia Walus
//! \date         24-04-2013
//! \file         SoloFlightOnlySolutionsAltDatesTest.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2013
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
// -------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PricingTrx.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsAltDates.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
namespace shpq
{
class SoloFlightOnlySolutionsAltDatesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloFlightOnlySolutionsAltDatesTest);
  CPPUNIT_TEST(testFosAltDatesOWCheck1);
  CPPUNIT_TEST(testFosAltDatesOWCheck2);
  CPPUNIT_TEST(testFosAltDatesOWCheckEnoughSolutionsForOneDate);
  CPPUNIT_TEST(testFosAltDatesRTOnline);
  CPPUNIT_TEST(testFosAltDatesRTInlineNotSnowman);
  CPPUNIT_TEST(testFosAltDatesRTInlineSnowman);
  CPPUNIT_TEST_SUITE_END();

public:
  void testFosAltDatesOWCheck1()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
    _trx->setAltDates(true);
    _trx->altDatePairs().clear();

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "AA", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "BA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "BA", DateTime(2013, 4, 2));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 2, "LO", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "LO", DateTime(2013, 4, 3));

    _trx->getOptions()->setRequestedNumberOfSolutions(2);
    DatePair datePair(_trx->legs()[0].sop()[0].itin()->travelSeg()[0]->departureDT(),
                      DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    _trx->altDatePairs().insert(mapItem);

    DatePair datePair2(_trx->legs()[0].sop()[1].itin()->travelSeg()[0]->departureDT(),
                       DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo2 = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo2->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem2(datePair2, altDateInfo2);
    _trx->altDatePairs().insert(mapItem2);

    DatePair datePair3(_trx->legs()[0].sop()[2].itin()->travelSeg()[0]->departureDT(),
                       DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo3 = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo3->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem3(datePair3, altDateInfo3);
    _trx->altDatePairs().insert(mapItem3);

    fos::SoloFlightOnlySolutionsAltDates fosAltDates(*_trx);
    fosAltDates.process();

    CPPUNIT_ASSERT(static_cast<unsigned>(_trx->flightMatrix().size()) == 3);
  }

  void testFosAltDatesOWCheck2()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
    _trx->setAltDates(true);
    _trx->altDatePairs().clear();

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "AA", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "BA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "BA", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 2, "LO", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "LO", DateTime(2013, 4, 2));

    _trx->getOptions()->setRequestedNumberOfSolutions(2);
    DatePair datePair(_trx->legs()[0].sop()[0].itin()->travelSeg()[0]->departureDT(),
                      DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    _trx->altDatePairs().insert(mapItem);

    DatePair datePair2(_trx->legs()[0].sop()[1].itin()->travelSeg()[0]->departureDT(),
                       DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo2 = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo2->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem2(datePair2, altDateInfo2);
    _trx->altDatePairs().insert(mapItem2);

    DatePair datePair3(_trx->legs()[0].sop()[2].itin()->travelSeg()[0]->departureDT(),
                       DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo3 = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo3->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem3(datePair3, altDateInfo3);
    _trx->altDatePairs().insert(mapItem3);

    fos::SoloFlightOnlySolutionsAltDates fosAltDates(*_trx);
    fosAltDates.process();

    CPPUNIT_ASSERT(static_cast<unsigned>(_trx->flightMatrix().size()) == 3);
  }

  void testFosAltDatesOWCheckEnoughSolutionsForOneDate()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
    _trx->setAltDates(true);
    _trx->altDatePairs().clear();

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "AA", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "BA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "BA", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 2, "LO", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "LO", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 3, "LH", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "LH", DateTime(2013, 4, 2));

    _trx->getOptions()->setRequestedNumberOfSolutions(2);
    DatePair datePair(_trx->legs()[0].sop()[0].itin()->travelSeg()[0]->departureDT(),
                      DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    _trx->altDatePairs().insert(mapItem);

    DatePair datePair2(_trx->legs()[0].sop()[1].itin()->travelSeg()[0]->departureDT(),
                       DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo2 = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo2->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem2(datePair2, altDateInfo2);
    _trx->altDatePairs().insert(mapItem2);

    DatePair datePair3(_trx->legs()[0].sop()[2].itin()->travelSeg()[0]->departureDT(),
                       DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo3 = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo3->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem3(datePair3, altDateInfo3);
    _trx->altDatePairs().insert(mapItem3);

    DatePair datePair4(_trx->legs()[0].sop()[3].itin()->travelSeg()[0]->departureDT(),
                       DateTime::emptyDate());
    PricingTrx::AltDateInfo* altDateInfo4 = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo4->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem4(datePair4, altDateInfo4);
    _trx->altDatePairs().insert(mapItem4);

    fos::SoloFlightOnlySolutionsAltDates fosAltDates(*_trx);
    fosAltDates.process();

    CPPUNIT_ASSERT(static_cast<unsigned>(_trx->flightMatrix().size()) == 3);

    std::vector<int> solution;
    solution.push_back(3);
    CPPUNIT_ASSERT(_trx->flightMatrix().find(solution) != _trx->flightMatrix().end());
  }

  void testFosAltDatesRTOnline()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
    _trx->setAltDates(true);
    _trx->altDatePairs().clear();

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "AA", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "KRK", "SYD", "AA", DateTime(2013, 4, 7));

    _trx->getOptions()->setRequestedNumberOfSolutions(1);
    DatePair datePair(_trx->legs()[0].sop()[0].itin()->travelSeg()[0]->departureDT(),
                      _trx->legs()[1].sop()[0].itin()->travelSeg()[0]->departureDT());
    PricingTrx::AltDateInfo* altDateInfo = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 1;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    _trx->altDatePairs().insert(mapItem);

    fos::SoloFlightOnlySolutionsAltDates fosAltDates(*_trx);
    fosAltDates.process();

    CPPUNIT_ASSERT(static_cast<unsigned>(_trx->flightMatrix().size()) == 1);
  }

  void testFosAltDatesRTInlineNotSnowman()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
    _trx->setAltDates(true);
    _trx->altDatePairs().clear();

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "AA", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 0, "LH", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "KRK", "SYD", "LH", DateTime(2013, 4, 7));

    _trx->getOptions()->setRequestedNumberOfSolutions(1);
    DatePair datePair(_trx->legs()[0].sop()[0].itin()->travelSeg()[0]->departureDT(),
                      _trx->legs()[1].sop()[0].itin()->travelSeg()[0]->departureDT());
    PricingTrx::AltDateInfo* altDateInfo = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 1;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    _trx->altDatePairs().insert(mapItem);

    fos::SoloFlightOnlySolutionsAltDates fosAltDates(*_trx);
    fosAltDates.process();

    CPPUNIT_ASSERT(static_cast<unsigned>(_trx->flightMatrix().size()) == 0);
  }

  void testFosAltDatesRTInlineSnowman()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
    _trx->setAltDates(true);
    _trx->altDatePairs().clear();

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "AA", DateTime(2013, 4, 1));
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "FRA", "KRK", "AA", DateTime(2013, 4, 1));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 0, "LH", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "KRK", "FRA", "LH", DateTime(2013, 4, 7));
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "FRA", "SYD", "LH", DateTime(2013, 4, 7));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 1, "LH", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "KRK", "SYD", "LH", DateTime(2013, 4, 9));

    _trx->getOptions()->setRequestedNumberOfSolutions(2);
    DatePair datePair(_trx->legs()[0].sop()[0].itin()->travelSeg()[0]->departureDT(),
                      DateTime(2013, 4, 7));
    PricingTrx::AltDateInfo* altDateInfo = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    _trx->altDatePairs().insert(mapItem);

    DatePair datePair2(_trx->legs()[0].sop()[0].itin()->travelSeg()[0]->departureDT(),
                       DateTime(2013, 4, 9));
    PricingTrx::AltDateInfo* altDateInfo2 = _memHandle.create<PricingTrx::AltDateInfo>();
    altDateInfo2->numOfSolutionNeeded = 2;
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem2(datePair2, altDateInfo2);
    _trx->altDatePairs().insert(mapItem2);

    fos::SoloFlightOnlySolutionsAltDates fosAltDates(*_trx);
    fosAltDates.process();

    std::vector<int> snowmanSolution;
    snowmanSolution.push_back(0);
    snowmanSolution.push_back(0);

    CPPUNIT_ASSERT(static_cast<unsigned>(_trx->flightMatrix().size()) == 1);
    CPPUNIT_ASSERT(_trx->flightMatrix().find(snowmanSolution) != _trx->flightMatrix().end());
  }

  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SoloFlightOnlySolutionsAltDatesTest);
}
}
