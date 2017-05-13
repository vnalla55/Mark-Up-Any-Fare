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

#include "Common/GlobalDirectionFinder.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiskCache.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/FlightUsage.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/GeoPath.h"
#include "Taxes/LegacyFacades/MileageGetterV2.h"
#include "Taxes/LegacyFacades/MileageServiceV2.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <memory>

namespace tse
{

class MileageServiceV2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageServiceV2Test);
  CPPUNIT_TEST(testGetMileageGetterReturnsMileageGetter);
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
    _trx.reset(new tse::PricingTrx());
    _request.reset(new PricingRequest());
    _trx->setRequest(_request.get());
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testGetMileageGetterReturnsMileageGetter()
  {
    tax::GeoPath geoPath;
    std::vector<tax::FlightUsage> flightUsages;
    tax::type::Timestamp localTime;
    //DataHandle dataHandle;
    MileageServiceV2 mileageServiceV2(*_trx);

    CPPUNIT_ASSERT(typeid(mileageServiceV2.getMileageGetter(geoPath, flightUsages, localTime)) ==
                   typeid(MileageGetterV2(geoPath, flightUsages, localTime, *_trx, false)));
  }

private:
  std::unique_ptr<PricingTrx> _trx;
  std::unique_ptr<PricingRequest> _request;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MileageServiceV2Test);

} // namespace tse
