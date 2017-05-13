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
#include "Taxes/LegacyFacades/CurrencyServiceV2.h"

#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyFacades/test/CurrencyConversionFacadeMock.h"
#include "Common/Money.h"
#include "DBAccess/DiskCache.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class CurrencyServiceV2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyServiceV2Test);
  CPPUNIT_TEST(testConvert);
  CPPUNIT_TEST(testConvertFailed);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = new PricingTrx();
    _currencyServiceV2 = new CurrencyServiceV2(*_trx, DateTime::localTime());
    _conversionFacadeMock = new CurrencyConversionFacadeMock();
    _currencyServiceV2->_conversionFacade = _conversionFacadeMock;
  }

  void tearDown()
  {
    delete _conversionFacadeMock;
    delete _currencyServiceV2;
    delete _trx;
    _memHandle.clear();
  }

  void testConvert()
  {
    _conversionFacadeMock->setConvertResult(true);
    tax::type::Money source;
    source._amount = 20;
    source._currency = "PLN";
    _currencyServiceV2->convertTo("USD", source);
    CPPUNIT_ASSERT_EQUAL(1u, _conversionFacadeMock->getCallsCounter());
  }

  void testConvertFailed()
  {
    _conversionFacadeMock->setConvertResult(false);
    tax::type::Money source;
    source._amount = 20;
    source._currency = "PLN";
    CPPUNIT_ASSERT_THROW(_currencyServiceV2->convertTo("USD", source), std::runtime_error);
  }

private:
  PricingTrx* _trx;
  CurrencyServiceV2* _currencyServiceV2;
  CurrencyConversionFacadeMock* _conversionFacadeMock;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyServiceV2Test);

} // namespace tse
