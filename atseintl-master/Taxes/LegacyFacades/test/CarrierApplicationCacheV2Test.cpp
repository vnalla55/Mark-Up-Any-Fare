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

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/TaxCarrierAppl.h"
#include "DBAccess/TaxCarrierApplSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiskCache.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierApplication.h"
#include "Taxes/LegacyFacades/CarrierApplicationServiceV2.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class CarrierApplicationServiceV2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarrierApplicationServiceV2Test);
  CPPUNIT_TEST(testGetCarrierApplication);
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

  void testGetCarrierApplication()
  {
    TaxCarrierAppl rawData;
    rawData.segs().push_back(new TaxCarrierApplSeg);
    rawData.segs().back()->carrier() = "BC";
    rawData.segs().back()->applInd() = ' ';
    rawData.segs().push_back(new TaxCarrierApplSeg);
    rawData.segs().back()->carrier() = "AA";
    rawData.segs().back()->applInd() = 'X';
    rawData.segs().push_back(new TaxCarrierApplSeg);
    rawData.segs().back()->carrier() = "BC";
    rawData.segs().back()->applInd() = ' ';

    TaxCarrierAppl* rawDataPtr = &rawData;
    _dataHandleMock->set_getTaxCarrierAppl(&rawDataPtr);

    CarrierApplicationServiceV2 carrierApplicationServiceV2(DateTime::localTime());

    std::shared_ptr<const tax::CarrierApplication> carrierApplication =
        carrierApplicationServiceV2.getCarrierApplication(tax::type::Vendor("ATP"),
                                                          tax::type::Index(1234));

    CPPUNIT_ASSERT(0 != carrierApplication);
    CPPUNIT_ASSERT_EQUAL(size_t(3), carrierApplication->entries.size());
    CPPUNIT_ASSERT(carrierApplication->entries[0].carrier == "BC");
    CPPUNIT_ASSERT(carrierApplication->entries[0].applind ==
                   tax::type::CarrierApplicationIndicator::Positive);
    CPPUNIT_ASSERT(carrierApplication->entries[1].carrier == "AA");
    CPPUNIT_ASSERT(carrierApplication->entries[1].applind ==
                   tax::type::CarrierApplicationIndicator::Negative);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CarrierApplicationServiceV2Test);

} // namespace tse
