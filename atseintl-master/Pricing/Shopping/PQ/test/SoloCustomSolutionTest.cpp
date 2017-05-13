// -------------------------------------------------------------------
//
//! \author       Natalia Walus
//! \date         08-03-2013
//! \file         SoloFlightOnlyCustomsSolutions.cpp
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

#include "Common/ShoppingUtil.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
namespace shpq
{

class SoloCustomSolutionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloCustomSolutionTest);
  CPPUNIT_TEST(testIsCustomSolution);
  CPPUNIT_TEST_SUITE_END();

public:
  void testIsCustomSolution()
  {
    std::vector<int> normalSolution;
    normalSolution.push_back(1);
    normalSolution.push_back(0);

    std::vector<int> customSolution;
    customSolution.push_back(0);
    customSolution.push_back(0);

    CPPUNIT_ASSERT(!ShoppingUtil::isCustomSolution(*_trx, normalSolution));
    CPPUNIT_ASSERT(ShoppingUtil::isCustomSolution(*_trx, customSolution));
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    DataHandle* dataHandle = _memHandle.create<DataHandle>();

    _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "LO", 0, true);
    // PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(*dataHandle, _trx->legs()[0],
    // "LHR", "WAW", "LO");

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "BA", 0);
    // PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(*dataHandle, _trx->legs()[0],
    // "WAW", "LHR", "BA");

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 0, "BA", 0);
    // PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(*dataHandle, _trx->legs()[1],
    // "WAW", "LHR", "BA", DateTime::localTime().addDays(7));
  }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SoloCustomSolutionTest);
}
}
