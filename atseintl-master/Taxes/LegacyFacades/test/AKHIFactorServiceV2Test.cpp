// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "Taxes/LegacyFacades/AKHIFactorServiceV2.h"
#include "Taxes/LegacyFacades/test/AKHIFactorServiceV2Mock.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"

#include "Common/DateTime.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxAkHiFactor.h"

namespace tse
{

class AKHIFactorServiceV2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AKHIFactorServiceV2Test);
  CPPUNIT_TEST(testHawaiiPercent);
  CPPUNIT_TEST(testAlaskaAPercent);
  CPPUNIT_TEST(testAlaskaBPercent);
  CPPUNIT_TEST(testAlaskaCPercent);
  CPPUNIT_TEST(testAlaskaDPercent);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _aKHIFactorServiceV2 = new AKHIFactorServiceV2Mock(_ticketingDT);

    _aKHIFactorServiceV2->taxAkHiFactor().hawaiiPercent() = tse::Percent(10.0);
    _aKHIFactorServiceV2->taxAkHiFactor().zoneAPercent() = tse::Percent(11.0);
    _aKHIFactorServiceV2->taxAkHiFactor().zoneBPercent() = tse::Percent(12.0);
    _aKHIFactorServiceV2->taxAkHiFactor().zoneCPercent() = tse::Percent(13.0);
    _aKHIFactorServiceV2->taxAkHiFactor().zoneDPercent() = tse::Percent(14.0);
  }

  void tearDown() { delete _aKHIFactorServiceV2; }

  void testHawaiiPercent()
  {
    tax::type::AirportCode locCode(tax::UninitializedCode);
    CPPUNIT_ASSERT_EQUAL(tax::doubleToPercent(_aKHIFactorServiceV2->taxAkHiFactor().hawaiiPercent()),
                         _aKHIFactorServiceV2->getHawaiiFactor(locCode));
  }

  void testAlaskaAPercent()
  {
    tax::type::AirportCode locCode(tax::UninitializedCode);
    CPPUNIT_ASSERT_EQUAL(tax::doubleToPercent(_aKHIFactorServiceV2->taxAkHiFactor().zoneAPercent()),
                         _aKHIFactorServiceV2->getAlaskaAFactor(locCode));
  }

  void testAlaskaBPercent()
  {
    tax::type::AirportCode locCode(tax::UninitializedCode);
    CPPUNIT_ASSERT_EQUAL(tax::doubleToPercent(_aKHIFactorServiceV2->taxAkHiFactor().zoneBPercent()),
                         _aKHIFactorServiceV2->getAlaskaBFactor(locCode));
  }

  void testAlaskaCPercent()
  {
    tax::type::AirportCode locCode(tax::UninitializedCode);
    CPPUNIT_ASSERT_EQUAL(tax::doubleToPercent(_aKHIFactorServiceV2->taxAkHiFactor().zoneCPercent()),
                         _aKHIFactorServiceV2->getAlaskaCFactor(locCode));
  }

  void testAlaskaDPercent()
  {
    tax::type::AirportCode locCode(tax::UninitializedCode);
    CPPUNIT_ASSERT_EQUAL(tax::doubleToPercent(_aKHIFactorServiceV2->taxAkHiFactor().zoneDPercent()),
                         _aKHIFactorServiceV2->getAlaskaDFactor(locCode));
  }

private:
  AKHIFactorServiceV2Mock* _aKHIFactorServiceV2;
  DateTime _ticketingDT;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AKHIFactorServiceV2Test);

} // namespace tse
