//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include <vector>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/AirSurfaceRestrictionValidator.h"
#include "Routing/CityCarrierRestrictionValidator.h"
#include "Routing/NonPricingRestrictionValidator.h"
#include "Routing/RestrictionValidatorFactory.h"
#include "Routing/RoutingConsts.h"
#include "Routing/SpecifiedMpmValidator.h"
#include "Routing/StopOverRestrictionValidator.h"
#include "Routing/StopTypeRestrictionValidator.h"
#include "Routing/TravelRestrictionValidator.h"
#include "Routing/TravelRoute.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class RestrictionValidatorFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RestrictionValidatorFactoryTest);

  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_1);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_2);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_3);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_4);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_5);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_6);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_7);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_8);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_9);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_11);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_11);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_13);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_14);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_15);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_17);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_18);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_19);
  CPPUNIT_TEST(testGetRestrictionValidator_RESTNUM_21);
  CPPUNIT_TEST(testGetRestrictionValidator_Other);

  CPPUNIT_TEST_SUITE_END();

  PricingTrx* _trx;
  RestrictionValidatorFactory* _fVF;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _fVF = &tse::Singleton<RestrictionValidatorFactory>::instance();
  }

  void tearDown() { _memHandle.clear(); }

  void testGetRestrictionValidator_RESTNUM_1()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<CityCarrierRestrictionValidator*>(_fVF->getRestrictionValidator("1", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_2()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<CityCarrierRestrictionValidator*>(_fVF->getRestrictionValidator("2", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_3()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<StopTypeRestrictionValidator*>(_fVF->getRestrictionValidator("3", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_4()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<StopTypeRestrictionValidator*>(_fVF->getRestrictionValidator("4", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_5()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<CityCarrierRestrictionValidator*>(_fVF->getRestrictionValidator("5", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_6()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<StopTypeRestrictionValidator*>(_fVF->getRestrictionValidator("6", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_7()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<StopOverRestrictionValidator*>(_fVF->getRestrictionValidator("7", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_8()
  {
    CPPUNIT_ASSERT(dynamic_cast<SpecifiedMpmValidator*>(_fVF->getRestrictionValidator("8", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_9()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<TravelRestrictionValidator*>(_fVF->getRestrictionValidator("9", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_10()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<TravelRestrictionValidator*>(_fVF->getRestrictionValidator("10", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_11()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<AirSurfaceRestrictionValidator*>(_fVF->getRestrictionValidator("11", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_13()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<NonPricingRestrictionValidator*>(_fVF->getRestrictionValidator("13", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_14()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<NonPricingRestrictionValidator*>(_fVF->getRestrictionValidator("14", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_15()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<NonPricingRestrictionValidator*>(_fVF->getRestrictionValidator("15", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_17()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<NonPricingRestrictionValidator*>(_fVF->getRestrictionValidator("17", *_trx)));
  }

  void testGetRestrictionValidator_RESTNUM_18()
  {
    CPPUNIT_ASSERT(!_fVF->getRestrictionValidator("18", *_trx));
  }

  void testGetRestrictionValidator_RESTNUM_19()
  {
    CPPUNIT_ASSERT(!_fVF->getRestrictionValidator("19", *_trx));
  }

  void testGetRestrictionValidator_RESTNUM_21()
  {
    CPPUNIT_ASSERT(
        dynamic_cast<CityCarrierRestrictionValidator*>(_fVF->getRestrictionValidator("21", *_trx)));
  }

  void testGetRestrictionValidator_Other()
  {
    CPPUNIT_ASSERT(!_fVF->getRestrictionValidator("99", *_trx));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RestrictionValidatorFactoryTest);
}
