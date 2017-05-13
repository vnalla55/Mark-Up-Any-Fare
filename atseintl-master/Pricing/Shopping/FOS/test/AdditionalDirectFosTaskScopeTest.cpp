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

#include "test/include/CppUnitHelperMacros.h"
#include <map>

#include "DataModel/Diversity.h"
#include "Pricing/Shopping/FOS/AdditionalDirectFosTaskScope.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace fos
{

// ==================================
// TEST CLASS
// ==================================

class AdditionalDirectFosTaskScopeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdditionalDirectFosTaskScopeTest);
  CPPUNIT_TEST(testParameters);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testParameters()
  {
    uint32_t numFos = 7;
    std::map<CarrierCode, uint32_t> perCarrier;
    perCarrier["AA"] = 4;
    perCarrier["LF"] = 2;
    perCarrier[Diversity::INTERLINE_CARRIER] = 1;

    AdditionalDirectFosTaskScope taskScope(numFos, perCarrier);

    CPPUNIT_ASSERT_EQUAL(uint32_t(0), taskScope.getNumOnlineFos());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), taskScope.getNumSnowmanFos());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), taskScope.getNumDiamondFos());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), taskScope.getNumTriangleFos());
    CPPUNIT_ASSERT_EQUAL(numFos, taskScope.getNumDirectFos());
    CPPUNIT_ASSERT_EQUAL(size_t(0), taskScope.getNumFosPerCarrier().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), taskScope.getNumDirectFosPerCarrier().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(4), taskScope.getNumDirectFosPerCarrier().at("AA"));
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), taskScope.getNumDirectFosPerCarrier().at("LF"));
    CPPUNIT_ASSERT_EQUAL(uint32_t(1),
                         taskScope.getNumDirectFosPerCarrier().at(Diversity::INTERLINE_CARRIER));
    CPPUNIT_ASSERT_EQUAL(int32_t(-1), taskScope.getNumCustomFos());
    CPPUNIT_ASSERT_EQUAL(int32_t(-1), taskScope.getNumLongConxFos());

    CPPUNIT_ASSERT(!taskScope.checkConnectingFlights());
    CPPUNIT_ASSERT(!taskScope.checkConnectingCities());
    CPPUNIT_ASSERT(!taskScope.pqConditionOverride());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AdditionalDirectFosTaskScopeTest);

} // fos
} // tse
