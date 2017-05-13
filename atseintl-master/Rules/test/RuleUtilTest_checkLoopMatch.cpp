#include "test/include/CppUnitHelperMacros.h"

#include "Rules/RuleUtil.h"

namespace tse
{

class RuleUtilTest_checkLoopMatch : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleUtilTest_checkLoopMatch);
  CPPUNIT_TEST(testCheckLoopMatch);
  CPPUNIT_TEST(testCheckLoopMatch_default);
  CPPUNIT_TEST(testCheckLoopMatch_once);
  CPPUNIT_TEST(testCheckLoopMatch_once_LocSpec);
  CPPUNIT_TEST(testCheckLoopMatch_once_LocSpec52);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _defaultTSIInfo = new TSIInfo;
    _defaultTSIInfo->tsi() = -1;

    _defaultTSIData =
        new RuleUtil::TSIData(*_defaultTSIInfo, RuleConst::TSI_SCOPE_JOURNEY, "ATP", 0, 0, 0, 0);
    _prevMatch = RuleConst::TSI_NOT_MATCH;
    _currMatch = RuleConst::TSI_NOT_MATCH;
    _inputLocSpecified = false;
  }

  void tearDown()
  {
    delete _defaultTSIInfo;
    delete _defaultTSIData;
  }

  bool isPossibleLoopMatch(RuleConst::TSILoopMatch loopMatch,
                           RuleConst::TSIMatch prevMatch,
                           RuleConst::TSIMatch currMatch)
  {
    if (loopMatch != RuleConst::TSI_MATCH_SECOND_FIRST)
      return _currMatch == RuleConst::TSI_MATCH;

    if (_prevMatch == RuleConst::TSI_MATCH)
      return true;

    if (_prevMatch == RuleConst::TSI_SOFT_MATCH && _currMatch == RuleConst::TSI_MATCH)
      return true;

    return false;
  }

  void assertPrevBit()
  {
    bool expected = false;
    if (isPossibleLoopMatch(
            RuleConst::TSILoopMatch(_defaultTSIInfo->loopMatch()), _prevMatch, _currMatch))
    {
      switch (_defaultTSIInfo->loopToSet())
      {
      case RuleConst::TSI_LOOP_SET_PREVIOUS:
      case RuleConst::TSI_LOOP_SET_CUR_PREV:
        expected = true;
        break;
      default:
        break;
      }
    }
    CPPUNIT_ASSERT_MESSAGE("bad bit: addPrevTravSeg ", _resultPrevBit == expected);
  }

  void assertCurrBit()
  {
    bool expected = false;
    if (isPossibleLoopMatch(
            RuleConst::TSILoopMatch(_defaultTSIInfo->loopMatch()), _prevMatch, _currMatch))
    {
      switch (_defaultTSIInfo->loopToSet())
      {
      case RuleConst::TSI_LOOP_SET_CURRENT:
      case RuleConst::TSI_LOOP_SET_CUR_NEXT:
      case RuleConst::TSI_LOOP_SET_CUR_PREV:
        expected = true;
        break;
      default:
        break;
      }
    }
    CPPUNIT_ASSERT_MESSAGE("bad bit: addCurrTravSeg ", _resultCurrBit == expected);
  }
  void assertNextBit()
  {
    bool expected = false;
    if (isPossibleLoopMatch(
            RuleConst::TSILoopMatch(_defaultTSIInfo->loopMatch()), _prevMatch, _currMatch))
    {
      switch (_defaultTSIInfo->loopToSet())
      {
      case RuleConst::TSI_LOOP_SET_NEXT:
      case RuleConst::TSI_LOOP_SET_CUR_NEXT:
        expected = true;
        break;
      default:
        break;
      }
    }
    CPPUNIT_ASSERT_MESSAGE("bad bit: addNextTravSeg ", _resultNextBit == expected);
  }

  void assertContBit()
  {
    // usally don't continue when curr matches
    bool expected = _currMatch != RuleConst::TSI_MATCH;

    // check for unusual cases
    switch (RuleConst::TSILoopMatch(_defaultTSIInfo->loopMatch()))
    {
    case RuleConst::TSI_MATCH_ALL:
      expected = true;
      break;

    case RuleConst::TSI_MATCH_FIRST: // no-op
      break;

    case RuleConst::TSI_MATCH_ONCE:
      // extra conditions for stopping
      expected = (_currMatch != RuleConst::TSI_MATCH && _defaultTSIInfo->tsi() == 52 &&
                  _inputLocSpecified);
      break;

    case RuleConst::TSI_MATCH_FIRST_ALL:
      expected = _defaultTSIData->scope() != RuleConst::TSI_SCOPE_FARE_COMPONENT;
      break;

    case RuleConst::TSI_MATCH_SOFT_MATCH:
      // can also stop on soft match
      if (RuleConst::TSI_SOFT_MATCH == _currMatch)
        expected = true;
      break;

    default:
      // bad loopMatch value
      return;
    }
    CPPUNIT_ASSERT_MESSAGE("bad bit: continueLooping ", _resultContBit == expected);
  }

  void callAndAssert()
  {
    RuleUtil::checkLoopMatch(*_defaultTSIData,
                             _prevMatch,
                             _currMatch,
                             _resultPrevBit,
                             _resultCurrBit,
                             _resultNextBit,
                             _resultContBit,
                             _inputLocSpecified);
    assertPrevBit();
    assertCurrBit();
    assertNextBit();
    assertContBit();
  }

  void testCheckLoopMatch()
  {
    _defaultTSIInfo->loopMatch() = RuleConst::TSI_MATCH_ALL;
    _defaultTSIInfo->loopToSet() = RuleConst::TSI_LOOP_SET_PREVIOUS;

    _currMatch = RuleConst::TSI_MATCH;

    callAndAssert();
  }

  void testCheckLoopMatch_default() { callAndAssert(); }

  void testCheckLoopMatch_once()
  {
    _defaultTSIInfo->loopMatch() = RuleConst::TSI_MATCH_ONCE;
    callAndAssert();
  }

  void testCheckLoopMatch_once_LocSpec()
  {
    _defaultTSIInfo->loopMatch() = RuleConst::TSI_MATCH_ONCE;
    _inputLocSpecified = true;
    callAndAssert();
  }

  void testCheckLoopMatch_once_LocSpec52()
  {
    _defaultTSIInfo->loopMatch() = RuleConst::TSI_MATCH_ONCE;
    _defaultTSIInfo->tsi() = 52;
    _inputLocSpecified = true;
    callAndAssert();
  }

private:
  TSIInfo* _defaultTSIInfo;
  RuleUtil::TSIData* _defaultTSIData;
  bool _inputLocSpecified;

  bool _resultPrevBit;
  bool _resultCurrBit;
  bool _resultNextBit;
  bool _resultContBit;
  RuleConst::TSIMatch _prevMatch;
  RuleConst::TSIMatch _currMatch;
};

CPPUNIT_TEST_SUITE_REGISTRATION(tse::RuleUtilTest_checkLoopMatch);

} // tse
