//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/Loc.h"
#include "Pricing/MergedFareMarket.h"

#include "test/include/CppUnitHelperMacros.h"

#include <string>

namespace tse
{
class AirSeg;
class FareMarket;
class PricingTrx;
}

class FareMarketPathMatrixTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareMarketPathMatrixTest);
  CPPUNIT_TEST(testBuildAllFareMarketPath);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBuildAllFareMarketPath();

protected:
  tse::FareMarket* getFareMarket(tse::Loc* orig, tse::Loc* dest, tse::PricingTrx& trx);
  tse::AirSeg* getTravelSeg(tse::Loc* orig, tse::Loc* dest, tse::PricingTrx& trx);
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareMarketPathMatrixTest);
