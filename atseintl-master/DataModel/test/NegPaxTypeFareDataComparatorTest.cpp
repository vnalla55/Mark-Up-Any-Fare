// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2014
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

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "Common/TseConsts.h"

#include "DataModel/NegPaxTypeFareData.h"
#include "DataModel/NegPaxTypeFareDataComparator.h"
#include "Rules/RuleConst.h"

#include "test/include/TestMemHandle.h"

namespace tse
{

class NegPaxTypeFareDataComparatorTest : public ::testing::Test
{
protected:
  typedef NegPaxTypeFareDataComparator Comparator;
  TestMemHandle _memHandle;
  Comparator* _comparator;
  NegPaxTypeFareData* _left, *_right;

  void setSamePsg(const PaxTypeCode& code) { _left->psgType = _right->psgType = code; }
  void setSamePricingOption(bool value)
  {
    _left->isPricingOption = _right->isPricingOption = value;
  }
  void setSameQualifierPresent(bool value)
  {
    _left->isQualifierPresent = _right->isQualifierPresent = value;
  }
  void setSameDirectionalityApplied(bool value)
  {
    _left->isDirectionalityApplied = _right->isDirectionalityApplied = value;
  }
  void setSameSet(uint16_t value) { _left->r2SubSetNum = _right->r2SubSetNum = value; }
  void setSameDir3(bool value) { _left->isDir3 = _right->isDir3 = value; }
  void setSameDir4(bool value) { _left->isDir4 = _right->isDir4 = value; }
  void setSameInOutInd(const char& value) { _left->inOutInd = _right->inOutInd = value; }
  void setDifferentDirectionality()
  {
    _left->isDir3 = true;
    _left->isDir4 = false;
    _left->inOutInd = RuleConst::ALWAYS_APPLIES;
    _right->isDir3 = false;
    _right->isDir4 = true;
    _right->inOutInd = RuleConst::ALWAYS_APPLIES;
  }

public:
  void SetUp()
  {
    _comparator = _memHandle.create<Comparator>();
    _left = _memHandle.create<NegPaxTypeFareData>();
    _right = _memHandle.create<NegPaxTypeFareData>();
  }

  void TearDown() { _memHandle.clear(); }
};

TEST_F(NegPaxTypeFareDataComparatorTest, testNotEquivalentDifferentPsg)
{
  _left->psgType = ADULT;
  _right->psgType = CHILD;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testNotEquivalentNotPricingOption1)
{
  setSamePsg(ADULT);
  _left->isPricingOption = false;
  _right->isPricingOption = true;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testNotEquivalentNotPricingOption2)
{
  setSamePsg(ADULT);
  _left->isPricingOption = true;
  _right->isPricingOption = false;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testNotEquivalentNotPricingOption3)
{
  setSamePsg(ADULT);
  _left->isPricingOption = false;
  _right->isPricingOption = false;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testNotEquivalentDifferentSetWithQualifier1)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  _left->r2SubSetNum = 1u;
  _left->isQualifierPresent = true;
  _right->r2SubSetNum = 2u;
  _right->isQualifierPresent = false;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testNotEquivalentDifferentSetWithQualifier2)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  _left->r2SubSetNum = 1u;
  _left->isQualifierPresent = false;
  _right->r2SubSetNum = 2u;
  _right->isQualifierPresent = true;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testNotEquivalentDifferentSetWithQualifier3)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  _left->r2SubSetNum = 1u;
  _left->isQualifierPresent = true;
  _right->r2SubSetNum = 2u;
  _right->isQualifierPresent = true;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest,
       testNotEquivalentDifferentSetNoQualifierDirectionalityApplied_1)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  _left->r2SubSetNum = 1u;
  _left->isDirectionalityApplied = true;
  _right->r2SubSetNum = 2u;
  _right->isDirectionalityApplied = false;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest,
       testNotEquivalentDifferentSetNoQualifierDirectionalityApplied_2)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  _left->r2SubSetNum = 1u;
  _right->r2SubSetNum = 2u;
  _left->isDirectionalityApplied = false;
  _right->isDirectionalityApplied = true;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest,
       testNotEquivalentDifferentSetNoQualifierDirectionalityApplied_3)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(true);
  setDifferentDirectionality();

  _left->r2SubSetNum = 1u;
  _right->r2SubSetNum = 2u;

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testEquivalentDifferentSetNoQualifierSameDirectionality)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(true);
  setSameDir3(true);
  _left->r2SubSetNum = 1u;
  _right->r2SubSetNum = 2u;

  ASSERT_TRUE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testEquivalentDifferentSetNoQualifierNoDirectionality)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(false);
  _left->r2SubSetNum = 1u;
  _right->r2SubSetNum = 2u;

  ASSERT_TRUE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testNotEquivalentDifferentDirectionality)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(true);
  setSameSet(1u);
  setDifferentDirectionality();

  ASSERT_FALSE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testEquivalentSameDir3)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(true);
  setSameSet(1u);
  setSameDir3(true);

  ASSERT_TRUE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testEquivalentSameDir4)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(true);
  setSameSet(1u);
  setSameDir3(false);
  setSameDir4(true);

  ASSERT_TRUE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testEquivalentSameInOutInd)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(true);
  setSameSet(1u);
  setSameDir3(false);
  setSameDir4(false);
  setSameInOutInd(RuleConst::FROM_LOC1_TO_LOC2);

  ASSERT_TRUE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testEquivalentSameSetNoQualifierNoDirectionality)
{
  setSamePsg(ADULT);
  setSamePricingOption(true);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(false);
  setSameSet(2u);

  ASSERT_TRUE(_comparator->areEquivalent(*_left, *_right));
}

TEST_F(NegPaxTypeFareDataComparatorTest, testEquivalentSameSetNoQualifierNoDirectionalityFD)
{
  setSamePsg(ADULT);
  setSamePricingOption(false);
  setSameQualifierPresent(false);
  setSameDirectionalityApplied(false);
  setSameSet(2u);
  _left->isFqTrx = _right->isFqTrx = true;

  ASSERT_TRUE(_comparator->areEquivalent(*_left, *_right));
}

} // tse
