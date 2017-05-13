// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Pricing/GroupFarePath.h"
#include "Pricing/Shopping/Diversity/ADIsSolutionNeededCheck.h"
#include "Pricing/Shopping/PQ/AltDatesStatistic.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestLogger.h"

namespace tse
{
namespace shpq
{

namespace
{

typedef AltDatesStatistic::Stat Stat;
typedef DiversityUtil::CompoundCarrier ADCarrier;

class DiversityModelCallbackMock : public ADIsSolutionNeededCheck::DiversityModelCallback
{
public:
  typedef boost::function<bool(DatePair)> DatePairFiltCallback;

  DiversityModelCallbackMock(ShoppingTrx& trx, const Stat& stat)
    : _trx(trx), _stat(stat), _datePairFiltCallback(nullDatePairFilt), _isNumOptNeeded(true)
  {
  }

  void setupDatePairFilter(DatePairFiltCallback mock) { _datePairFiltCallback = mock; }
  void setupIsNumOptionNeeded(bool fixture) { _isNumOptNeeded = fixture; }

protected:
  /**** DiversityModelCallback overrides ****/

  virtual Stat getStat(const DiversityModel::SOPCombination& comb, MoneyAmount score) const
  {
    return _stat;
  }

  virtual Stat
  getStat(const ShoppingTrx::FlightMatrix::value_type& solution, const DatePair& datePair) const
  {
    return _stat;
  }

  /**
   * @return true if date pair shall be skipped from processing and diagnostic output
   */
  virtual bool checkDatePairFilt(DatePair datePair) const
  {
    return _datePairFiltCallback(datePair);
  }

  /**
   * @param datePairFirstFareLevelAmount < 0. means has not initialized yet
   */
  virtual bool
  isDatePairFareCutoffReached(MoneyAmount datePairFirstFareLevelAmount, MoneyAmount price) const
  {
    return false;
  }

  virtual bool isFirstCxrFareLevel(const Stat&) const { return true; }

  virtual bool isNumOptionNeeded(const Stat&) const { return _isNumOptNeeded; }

  virtual const ShoppingTrx* getTrx() const { return &_trx; }

  /******************************************/
private:
  ShoppingTrx& _trx;
  Stat _stat; // stub for return value
  DatePairFiltCallback _datePairFiltCallback;
  bool _isNumOptNeeded;

  static bool nullDatePairFilt(DatePair) { return false; }
};

class IsSolutionNeededMock : public ADIsSolutionNeededCheck
{
protected:
  /**
   * @override
   */
  virtual bool
  isSnowmanNeeded(const DiversityModelCallback& ctx, shpq::SopIdxVecArg sops, const Stat& stat)
  {
    CPPUNIT_FAIL("Must not go so far");
    return 0;
  }
};

const DateTime
TODAY("2013-06-19", 0);
const DateTime
TOMORROW("2013-06-20", 0);

} // anon ns

class ADIsSolutionNeededCheckTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ADIsSolutionNeededCheckTest);
  CPPUNIT_TEST(testIsNeeded);
  CPPUNIT_TEST(testIsNeededByLastFareLevel);
  CPPUNIT_TEST(testIsNotNeededForFareLevel);
  CPPUNIT_TEST(testIsNotNeededByDatePairFilt);
  CPPUNIT_TEST(testIsNotNeededByNumOpt);
  CPPUNIT_TEST_SUITE_END();

public:
  ADIsSolutionNeededCheckTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<RootLoggerGetOff>();
    _trx = _memHandle.create<ShoppingTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void testIsNeeded()
  {
    Stat statForNeededComb;

    IsSolutionNeededMock mock;
    DiversityModelCallbackMock modelMock(*_trx, statForNeededComb);
    invokeExamine(mock, modelMock);

    CPPUNIT_ASSERT(mock.isNeeded());
    CPPUNIT_ASSERT_MESSAGE("Should not explain why a comb is needed", !mock.shallExplain());
  }

  void testIsNeededByLastFareLevel()
  {
    Stat statForNeededComb;
    statForNeededComb._fareLevel = _trx->diversity().getAltDates()._fareLevelNumber;

    IsSolutionNeededMock mock;
    DiversityModelCallbackMock modelMock(*_trx, statForNeededComb);
    invokeExamine(mock, modelMock);

    CPPUNIT_ASSERT(mock.isNeeded());
    CPPUNIT_ASSERT_MESSAGE("Should not explain why a comb is needed", !mock.shallExplain());
  }

  void testIsNotNeededForFareLevel()
  {
    uint16_t extraFareLevel = (_trx->diversity().getAltDates()._fareLevelNumber + 1);
    Stat statForNotNeededComb;
    statForNotNeededComb._fareLevel = extraFareLevel;

    IsSolutionNeededMock mock;
    DiversityModelCallbackMock modelMock(*_trx, statForNotNeededComb);
    invokeExamine(mock, modelMock);

    CPPUNIT_ASSERT(!mock.isNeeded());
    CPPUNIT_ASSERT_MESSAGE("Should explain why a comb is not needed", mock.shallExplain());
  }

  void testIsNotNeededByDatePairFilt()
  {
    Stat statForNotNeededComb;
    ExamineParams params;
    params._datePair = DatePair(TODAY, TOMORROW);

    DiversityModelCallbackMock modelMock(*_trx, statForNotNeededComb);
    modelMock.setupDatePairFilter(bind(&datePairFiltFixture, true, params._datePair, _1));
    IsSolutionNeededMock mock;
    invokeExamine(mock, modelMock, params);

    CPPUNIT_ASSERT(!mock.isNeeded());
    CPPUNIT_ASSERT_MESSAGE("Should not explain if a comb is filtered by date pair diag param",
                           !mock.shallExplain());
  }

  void testIsNotNeededByNumOpt()
  {
    Stat statForNotNeededComb;
    DiversityModelCallbackMock modelMock(*_trx, statForNotNeededComb);
    modelMock.setupIsNumOptionNeeded(false);

    IsSolutionNeededMock mock;
    invokeExamine(mock, modelMock);

    CPPUNIT_ASSERT(!mock.isNeeded());
    CPPUNIT_ASSERT_MESSAGE("Should explain why a comb is not needed", mock.shallExplain());
  }

private:
  /**
   * Stat which is customized with default constructor
   */
  class Stat : public AltDatesStatistic::Stat
  {
  public:
    Stat() : AltDatesStatistic::Stat(1, 1, 0.01, CarrierCode("AA")) {}
  };

  struct ExamineParams
  {
    std::vector<int> _comb;
    float _price;
    DatePair _datePair;

    ExamineParams() : _price(0.01) // all to defaults
    {
    }
  };

  TestMemHandle _memHandle;
  ShoppingTrx* _trx;

  static void invokeExamine(ADIsSolutionNeededCheck& invokeTarget,
                            ADIsSolutionNeededCheck::DiversityModelCallback& param1,
                            const ExamineParams& paramPack = ExamineParams())
  {
    GroupFarePath gfp;
    gfp.setTotalNUCAmount(paramPack._price);

    invokeTarget.examine(param1, make_pair(paramPack._comb, &gfp), paramPack._datePair);
  }

  static bool datePairFiltFixture(bool retvalFixture, DatePair expect, DatePair actual)
  {
    CPPUNIT_ASSERT_MESSAGE("Date pair param expectation failed", expect == actual);
    return retvalFixture;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ADIsSolutionNeededCheckTest);

} /* namespace shpq */
} /* namespace tse */
