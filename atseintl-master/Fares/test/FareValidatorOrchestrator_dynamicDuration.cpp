#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/set.hpp>
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace boost::assign;

class FareValidatorOrchestrator_dynamicDuration : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_dynamicDuration);
  CPPUNIT_TEST(testFareValidatorOrchestrator_dynamicDuration_getNextDuration_allEmpty);
  CPPUNIT_TEST(testFareValidatorOrchestrator_dynamicDuration_getNextDuration_durationsEmpty);
  CPPUNIT_TEST(testFareValidatorOrchestrator_dynamicDuration_getNextDuration_checkedDurationsEmpty);
  CPPUNIT_TEST(testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findMax);
  CPPUNIT_TEST(testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findMaxAtTheEnd);
  CPPUNIT_TEST(testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findFirst);
  CPPUNIT_TEST(testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findLast);
  CPPUNIT_TEST(testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findIfAllFound);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.insert(_fvo = new FareValidatorOrchestratorDerived("FVO", *_memHandle.create<MockTseServer>()));
    _trx = _memHandle.create<ShoppingTrx>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testFareValidatorOrchestrator_dynamicDuration_getNextDuration_allEmpty()
  {
    uint64_t expectedNextDuration = 0;
    uint64_t nextDuration = 0;
    std::set<uint64_t> checkedDurations;

    nextDuration = _fvo->getNextDuration(*_trx, checkedDurations);

    CPPUNIT_ASSERT_EQUAL(expectedNextDuration, nextDuration);
  }

  void testFareValidatorOrchestrator_dynamicDuration_getNextDuration_durationsEmpty()
  {
    uint64_t expectedNextDuration = 0;
    uint64_t nextDuration = 0;
    std::set<uint64_t> checkedDurations;
    checkedDurations += 2, 1;

    nextDuration = _fvo->getNextDuration(*_trx, checkedDurations);

    CPPUNIT_ASSERT_EQUAL(expectedNextDuration, nextDuration);
  }

  void testFareValidatorOrchestrator_dynamicDuration_getNextDuration_checkedDurationsEmpty()
  {
    uint64_t expectedNextDuration = 1;
    uint64_t nextDuration = 0;
    std::set<uint64_t> checkedDurations;

    _trx->durationAltDatePairs().clear();
    std::vector<DatePair> datePairsForFirstDuraton;
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 10), DateTime(2008, 10, 10));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 11), DateTime(2008, 10, 11));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 12), DateTime(2008, 10, 12));
    createDurationAltDatePairs(*_trx, 1, createAltDatePairs(datePairsForFirstDuraton));
    std::vector<DatePair> datePairsForSecondDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 20), DateTime(2008, 10, 20));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 21), DateTime(2008, 10, 21));
    createDurationAltDatePairs(*_trx, 2, createAltDatePairs(datePairsForSecondDuraton));

    nextDuration = _fvo->getNextDuration(*_trx, checkedDurations);

    CPPUNIT_ASSERT_EQUAL(expectedNextDuration, nextDuration);
  }

  void testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findMax()
  {
    uint64_t expectedNextDuration = 1;
    uint64_t nextDuration = 0;
    std::set<uint64_t> checkedDurations;
    checkedDurations += 2;

    _trx->durationAltDatePairs().clear();
    std::vector<DatePair> datePairsForFirstDuraton;
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 10), DateTime(2008, 10, 10));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 11), DateTime(2008, 10, 11));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 12), DateTime(2008, 10, 12));
    createDurationAltDatePairs(*_trx, 1, createAltDatePairs(datePairsForFirstDuraton));
    std::vector<DatePair> datePairsForSecondDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 20), DateTime(2008, 10, 20));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 21), DateTime(2008, 10, 21));
    createDurationAltDatePairs(*_trx, 2, createAltDatePairs(datePairsForSecondDuraton));

    nextDuration = _fvo->getNextDuration(*_trx, checkedDurations);

    CPPUNIT_ASSERT_EQUAL(expectedNextDuration, nextDuration);
  }

  void testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findMaxAtTheEnd()
  {
    uint64_t expectedNextDuration = 2;
    uint64_t nextDuration = 0;
    std::set<uint64_t> checkedDurations;
    checkedDurations += 1;

    _trx->durationAltDatePairs().clear();
    std::vector<DatePair> datePairsForFirstDuraton;
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 10), DateTime(2008, 10, 10));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 11), DateTime(2008, 10, 11));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 12), DateTime(2008, 10, 12));
    createDurationAltDatePairs(*_trx, 1, createAltDatePairs(datePairsForFirstDuraton));
    std::vector<DatePair> datePairsForSecondDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 20), DateTime(2008, 10, 20));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 21), DateTime(2008, 10, 21));
    createDurationAltDatePairs(*_trx, 2, createAltDatePairs(datePairsForSecondDuraton));

    nextDuration = _fvo->getNextDuration(*_trx, checkedDurations);

    CPPUNIT_ASSERT_EQUAL(expectedNextDuration, nextDuration);
  }

  void testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findFirst()
  {
    uint64_t expectedNextDuration = 1;
    uint64_t nextDuration = 0;
    std::set<uint64_t> checkedDurations;
    checkedDurations += 4;

    _trx->durationAltDatePairs().clear();
    std::vector<DatePair> datePairsForFirstDuraton;
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 10), DateTime(2008, 10, 10));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 11), DateTime(2008, 10, 11));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 12), DateTime(2008, 10, 12));
    createDurationAltDatePairs(*_trx, 1, createAltDatePairs(datePairsForFirstDuraton));
    std::vector<DatePair> datePairsForSecondDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 20), DateTime(2008, 10, 20));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 21), DateTime(2008, 10, 21));
    createDurationAltDatePairs(*_trx, 2, createAltDatePairs(datePairsForSecondDuraton));
    std::vector<DatePair> datePairsForThirdDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 24), DateTime(2008, 11, 25));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 25), DateTime(2008, 11, 26));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 26), DateTime(2008, 11, 27));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 27), DateTime(2008, 11, 28));
    createDurationAltDatePairs(*_trx, 4, createAltDatePairs(datePairsForThirdDuraton));

    nextDuration = _fvo->getNextDuration(*_trx, checkedDurations);

    CPPUNIT_ASSERT_EQUAL(expectedNextDuration, nextDuration);
  }

  void testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findLast()
  {
    uint64_t expectedNextDuration = 2;
    uint64_t nextDuration = 0;
    std::set<uint64_t> checkedDurations;
    checkedDurations += 4, 1;

    _trx->durationAltDatePairs().clear();
    std::vector<DatePair> datePairsForFirstDuraton;
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 10), DateTime(2008, 10, 10));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 11), DateTime(2008, 10, 11));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 12), DateTime(2008, 10, 12));
    createDurationAltDatePairs(*_trx, 1, createAltDatePairs(datePairsForFirstDuraton));
    std::vector<DatePair> datePairsForSecondDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 20), DateTime(2008, 10, 20));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 21), DateTime(2008, 10, 21));
    createDurationAltDatePairs(*_trx, 2, createAltDatePairs(datePairsForSecondDuraton));
    std::vector<DatePair> datePairsForThirdDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 24), DateTime(2008, 11, 25));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 25), DateTime(2008, 11, 26));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 26), DateTime(2008, 11, 27));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 27), DateTime(2008, 11, 28));
    createDurationAltDatePairs(*_trx, 4, createAltDatePairs(datePairsForThirdDuraton));

    nextDuration = _fvo->getNextDuration(*_trx, checkedDurations);

    CPPUNIT_ASSERT_EQUAL(expectedNextDuration, nextDuration);
  }

  void testFareValidatorOrchestrator_dynamicDuration_getNextDuration_findIfAllFound()
  {
    uint64_t expectedNextDuration = 0;
    uint64_t nextDuration = 0;
    std::set<uint64_t> checkedDurations;
    checkedDurations += 4, 1, 2;

    _trx->durationAltDatePairs().clear();
    std::vector<DatePair> datePairsForFirstDuraton;
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 10), DateTime(2008, 10, 10));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 11), DateTime(2008, 10, 11));
    datePairsForFirstDuraton += std::make_pair(DateTime(2008, 9, 12), DateTime(2008, 10, 12));
    createDurationAltDatePairs(*_trx, 1, createAltDatePairs(datePairsForFirstDuraton));
    std::vector<DatePair> datePairsForSecondDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 20), DateTime(2008, 10, 20));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 21), DateTime(2008, 10, 21));
    createDurationAltDatePairs(*_trx, 2, createAltDatePairs(datePairsForSecondDuraton));
    std::vector<DatePair> datePairsForThirdDuraton;
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 24), DateTime(2008, 11, 25));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 25), DateTime(2008, 11, 26));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 26), DateTime(2008, 11, 27));
    datePairsForSecondDuraton += std::make_pair(DateTime(2008, 9, 27), DateTime(2008, 11, 28));
    createDurationAltDatePairs(*_trx, 4, createAltDatePairs(datePairsForThirdDuraton));

    nextDuration = _fvo->getNextDuration(*_trx, checkedDurations);

    CPPUNIT_ASSERT_EQUAL(expectedNextDuration, nextDuration);
  }

protected:
  PricingTrx::AltDatePairs* createAltDatePairs(std::vector<DatePair>& datePairs)
  {
    PricingTrx::AltDatePairs* altDatePairs = 0;
    _memHandle.get(altDatePairs);
    PricingTrx::AltDateInfo* altDateInfo = 0;
    std::vector<DatePair>::const_iterator datePairsIter = datePairs.begin();
    for (; datePairsIter != datePairs.end(); ++datePairsIter)
    {
      (*altDatePairs)[*datePairsIter] = altDateInfo;
    }

    return altDatePairs;
  }

  void createDurationAltDatePairs(ShoppingTrx& trx,
                                  uint64_t duration,
                                  PricingTrx::AltDatePairs* altDatePairs)
  {
    trx.durationAltDatePairs()[duration] = *altDatePairs;
  }

protected:
  ShoppingTrx* _trx;
  FareValidatorOrchestratorDerived* _fvo;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareValidatorOrchestrator_dynamicDuration);

} // tse
