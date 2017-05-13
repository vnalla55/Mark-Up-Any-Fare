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

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/FlexFares/GroupsData.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
namespace flexFares
{
// ==================================
// TEST
// ==================================

class GroupsDataTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GroupsDataTest);

  CPPUNIT_TEST(testPaxTypeDefault);
  CPPUNIT_TEST(testPaxTypeChild);

  CPPUNIT_TEST(testRequestedCabinDefault);
  CPPUNIT_TEST(testRequestedCabin);

  CPPUNIT_TEST(testNoAdvancePurchase);
  CPPUNIT_TEST(testNoPenalties);
  CPPUNIT_TEST(testNoMinMaxStay);
  CPPUNIT_TEST(testNoRestrictions);

  CPPUNIT_TEST(testMultipleGroups);

  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testPaxTypeDefault()
  {
    GroupsData ffgData;
    ffgData.requireNoRestrictions(0u); // to create flex fare group with index 0

    CPPUNIT_ASSERT_EQUAL(ADULT, ffgData.getPaxTypeCode(0u));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1u), ffgData.getSize());
  }

  void testPaxTypeChild()
  {
    GroupsData ffgData;
    ffgData.setPaxTypeCode(CHILD, 0u);

    CPPUNIT_ASSERT_EQUAL(CHILD, ffgData.getPaxTypeCode(0u));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1u), ffgData.getSize());
  }

  void testRequestedCabinDefault()
  {
    GroupsData ffgData;
    ffgData.requireNoRestrictions(1u);

    CPPUNIT_ASSERT(ffgData.getRequestedCabin(1u).isUnknownClass());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1u), ffgData.getSize());
  }

  void testRequestedCabin()
  {
    GroupsData ffgData;
    ffgData.setRequestedCabin(CabinType::BUSINESS_CLASS, 1u);

    CPPUNIT_ASSERT(ffgData.getRequestedCabin(1u).isBusinessClass());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1u), ffgData.getSize());
  }

  void testNoAdvancePurchase()
  {
    GroupsData ffgData;
    ffgData.requireNoAdvancePurchase(2u);

    CPPUNIT_ASSERT(ffgData.isNoAdvancePurchaseRequired(2u));
    CPPUNIT_ASSERT(!ffgData.isNoPenaltiesRequired(2u));
    CPPUNIT_ASSERT(!ffgData.isNoMinMaxStayRequired(2u));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1u), ffgData.getSize());
  }

  void testNoPenalties()
  {
    GroupsData ffgData;
    ffgData.requireNoPenalties(2u);

    CPPUNIT_ASSERT(!ffgData.isNoAdvancePurchaseRequired(2u));
    CPPUNIT_ASSERT(ffgData.isNoPenaltiesRequired(2u));
    CPPUNIT_ASSERT(!ffgData.isNoMinMaxStayRequired(2u));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1u), ffgData.getSize());
  }

  void testNoMinMaxStay()
  {
    GroupsData ffgData;
    ffgData.requireNoMinMaxStay(2u);

    CPPUNIT_ASSERT(!ffgData.isNoAdvancePurchaseRequired(2u));
    CPPUNIT_ASSERT(!ffgData.isNoPenaltiesRequired(2u));
    CPPUNIT_ASSERT(ffgData.isNoMinMaxStayRequired(2u));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1u), ffgData.getSize());
  }

  void testNoRestrictions()
  {
    GroupsData ffgData;
    ffgData.requireNoRestrictions(4u);

    CPPUNIT_ASSERT(ffgData.isNoAdvancePurchaseRequired(4u));
    CPPUNIT_ASSERT(ffgData.isNoPenaltiesRequired(4u));
    CPPUNIT_ASSERT(ffgData.isNoMinMaxStayRequired(4u));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1u), ffgData.getSize());
  }

  void testMultipleGroups()
  {
    GroupsData ffgData;
    ffgData.requireNoMinMaxStay();
    ffgData.requireNoPenalties(4u);

    CPPUNIT_ASSERT(!ffgData.isNoAdvancePurchaseRequired());
    CPPUNIT_ASSERT(!ffgData.isNoPenaltiesRequired());
    CPPUNIT_ASSERT(ffgData.isNoMinMaxStayRequired());
    CPPUNIT_ASSERT(!ffgData.isNoAdvancePurchaseRequired(4u));
    CPPUNIT_ASSERT(ffgData.isNoPenaltiesRequired(4u));
    CPPUNIT_ASSERT(!ffgData.isNoMinMaxStayRequired(4u));
    CPPUNIT_ASSERT_EQUAL(uint16_t(2u), ffgData.getSize());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GroupsDataTest);
} // flexFares
} // tse
