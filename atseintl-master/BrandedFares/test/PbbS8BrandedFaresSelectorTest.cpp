//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelectorTest.cpp
//  Created:     2014
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "BrandedFares/PbbS8BrandedFaresSelector.h"
#include "BrandedFares/test/S8BrandedFaresSelectorInitializer.h"

#include "test/include/TestFallbackUtil.h"

using namespace std;
namespace tse
{
FALLBACKVALUE_DECL(fallbackBrandDirectionality);

class PbbBrandedFaresSelectorTest : public CppUnit::TestFixture, S8BrandedFaresSelectorInitializer<PbbBrandedFaresSelector>
{
CPPUNIT_TEST_SUITE(PbbBrandedFaresSelectorTest);
CPPUNIT_TEST(testValidateBrandCodeMatched);
CPPUNIT_TEST(testValidateBrandCodeMatchedDirectionality);
CPPUNIT_TEST(testValidateBrandCodeNotMatched_allFaresFailed);
CPPUNIT_TEST(testIsValidForPbbTrue);
CPPUNIT_TEST(testIsValidForPbbFalse);
CPPUNIT_TEST(testFailedFares_noBrandedMarket);
CPPUNIT_TEST(testInvalidBrandProgram);
CPPUNIT_TEST_SUITE_END();

public:
  PbbBrandedFaresSelectorTest(){}
  ~PbbBrandedFaresSelectorTest(){}

public:
  void setUp()
  {
    init();
    _s8BrandedFaresSelector = _memHandle.create<PbbBrandedFaresSelector>(
                                *_trx, *(_memHandle.create<BrandedFareValidatorFactory>(*_trx)));
    _fdS8BrandedFaresSelector = _memHandle.create<PbbBrandedFaresSelector>(
                                  *_fdTrx, *(_memHandle.create<BrandedFareValidatorFactory>(*_fdTrx)));
  }

  void tearDown() { _memHandle.clear(); }

  void testValidateBrandCodeMatched()
  {
    fallback::value::fallbackBrandDirectionality.set(true);
    _fm1->travelSeg()[0]->setBrandCode("BC");
    std::vector<FareMarket*>::iterator iBeg = _trx->fareMarket().begin();
    std::vector<FareMarket*>::iterator iEnd = _trx->fareMarket().end();
    for (; iBeg != iEnd; ++iBeg)
    {
      CPPUNIT_ASSERT_EQUAL(6, (int)(*iBeg)->brandProgramIndexVec().size());
      _s8BrandedFaresSelector->validate(**iBeg);
      CPPUNIT_ASSERT_EQUAL(1, (int)(*iBeg)->brandProgramIndexVec().size());
    }
  }

  void testValidateBrandCodeMatchedDirectionality()
  {
    _fm1->travelSeg()[0]->setBrandCode("BC");
    std::vector<FareMarket*>::iterator iBeg = _trx->fareMarket().begin();
    std::vector<FareMarket*>::iterator iEnd = _trx->fareMarket().end();
    for (; iBeg != iEnd; ++iBeg)
    {
      CPPUNIT_ASSERT_EQUAL(6, (int)(*iBeg)->brandProgramIndexVec().size());
      _s8BrandedFaresSelector->validate(**iBeg);
      //brand code 3 is present 2 times in two different programs
      CPPUNIT_ASSERT_EQUAL(2, (int)(*iBeg)->brandProgramIndexVec().size());
    }
  }

  void testValidateBrandCodeNotMatched_allFaresFailed()
  {
    _fm1->travelSeg()[0]->setBrandCode("TP");;
    std::vector<FareMarket*>::iterator iBeg = _trx->fareMarket().begin();
    std::vector<FareMarket*>::iterator iEnd = _trx->fareMarket().end();
    for (; iBeg != iEnd; ++iBeg)
    {
      CPPUNIT_ASSERT_EQUAL(6, (int)(*iBeg)->brandProgramIndexVec().size());
      _s8BrandedFaresSelector->validate(**iBeg);
      CPPUNIT_ASSERT((*iBeg)->brandProgramIndexVec().empty());

      for (PaxTypeFare* fare : (*iBeg)->allPaxTypeFare())
        CPPUNIT_ASSERT(!fare->isValidForBranding());

    }
  }

  void testIsValidForPbbTrue()
  {
    _ptf1->setIsShoppingFare();
    std::vector<FareMarket*>::iterator iBeg = _trx->fareMarket().begin();
    std::vector<FareMarket*>::iterator iEnd = _trx->fareMarket().end();
    for (; iBeg != iEnd; ++iBeg)
    {
      _s8BrandedFaresSelector->validate(**iBeg);
      for (PaxTypeFare* fare : (*iBeg)->allPaxTypeFare())
      {
        CPPUNIT_ASSERT(fare->isValidForBranding());
      }
    }
  }

  void testIsValidForPbbFalse()
  {
    _svcFeesFareIdInfo->fareApplInd() = 'N';
    _ptf1->setIsShoppingFare();
    std::vector<FareMarket*>::iterator iBeg = _trx->fareMarket().begin();
    std::vector<FareMarket*>::iterator iEnd = _trx->fareMarket().end();
    for (; iBeg != iEnd; ++iBeg)
    {
      _s8BrandedFaresSelector->validate(**iBeg);
      for (PaxTypeFare* fare : (*iBeg)->allPaxTypeFare())
      {
        CPPUNIT_ASSERT(!fare->isValidForBranding());
      }
    }
  }

  void testFailedFares_noBrandedMarket()
  {
    _trx->brandedMarketMap().clear();

    std::vector<FareMarket*>::iterator iBeg = _trx->fareMarket().begin();
    std::vector<FareMarket*>::iterator iEnd = _trx->fareMarket().end();
    for (; iBeg != iEnd; ++iBeg)
    {
      for (PaxTypeFare* fare : (*iBeg)->allPaxTypeFare())
        CPPUNIT_ASSERT(fare->isValidForBranding());

      _s8BrandedFaresSelector->validate(**iBeg);

      for (PaxTypeFare* fare : (*iBeg)->allPaxTypeFare())
        CPPUNIT_ASSERT(!fare->isValidForBranding());
    }
  }

  void testInvalidBrandProgram()
  {
    _fm1->marketIDVec().clear();
    _s8BrandedFaresSelector->validate(*_fm1);
    CPPUNIT_ASSERT(!_fm1->allPaxTypeFare().empty());
    for (PaxTypeFare* fare : _fm1->allPaxTypeFare())
      CPPUNIT_ASSERT(!fare->isValidForBranding());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PbbBrandedFaresSelectorTest);
}

