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

#include "Common/TseCodeTypes.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiskCache.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierFlight.h"
#include "Taxes/LegacyFacades/CarrierFlightServiceV2.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class CarrierFlightServiceV2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarrierFlightServiceV2Test);
  CPPUNIT_TEST(testGetCarrierFlightReturnNull);
  CPPUNIT_TEST(testGetCarrierFlightReturnNotNull);
  CPPUNIT_TEST_SUITE_END();

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

private:
  DataHandleMock* _dataHandleMock;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _dataHandleMock = _memHandle.create<DataHandleMock>();
    _memHandle.create<SpecificTestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testGetCarrierFlightReturnNull()
  {
    TaxCarrierFlightInfo * rawData = 0;
    _dataHandleMock->set_getTaxCarrierFlight(&rawData);

    CarrierFlightServiceV2 carrierFlightServiceV2(DateTime::localTime());

    std::shared_ptr<const tax::CarrierFlight> taxCarrierFlight =
        carrierFlightServiceV2.getCarrierFlight(tax::type::Vendor("ATP"), tax::type::Index(1234));

    CPPUNIT_ASSERT(!taxCarrierFlight);
  }

  void testGetCarrierFlightReturnNotNull()
  {
    TaxCarrierFlightInfo rawData;
    TaxCarrierFlightInfo* rawDataPtr = &rawData;
    TaxCarrierFlightInfo::dummyData(rawData);
    _dataHandleMock->set_getTaxCarrierFlight(&rawDataPtr);

    CarrierFlightServiceV2 carrierFlightServiceV2(DateTime::localTime());

    std::shared_ptr<const tax::CarrierFlight> taxCarrierFlight =
        carrierFlightServiceV2.getCarrierFlight(tax::type::Vendor("ATP"), tax::type::Index(1235));

    CPPUNIT_ASSERT(taxCarrierFlight);
    CPPUNIT_ASSERT_EQUAL(toTaxVendorCode(rawData.vendor()), taxCarrierFlight->vendor);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(rawData.itemNo()), taxCarrierFlight->itemNo);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CarrierFlightServiceV2Test);

} // namespace tse
