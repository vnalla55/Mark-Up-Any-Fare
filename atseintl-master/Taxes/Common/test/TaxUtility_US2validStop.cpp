
#include <string>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include "DBAccess/Mileage.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/PricingOptions.h"
#include "Taxes/Common/TaxUtility.h"
#include "TestFactoryManager.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"

namespace tse
{

namespace
{
class MyDataHandle2 : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  ~MyDataHandle2() { _memHandle.clear(); }

  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    Mileage* mil = _memHandle.create<Mileage>();
    if (origin == "FRA")
    {
      if (dest == "DFW")
        mil->mileage() = 5130;
      else if (dest == "YYC")
        mil->mileage() = 4670;
      else if (dest == "YYZ")
        mil->mileage() = 3940;
      else if (dest == "ORD")
        mil->mileage() = 4330;
      else
        mil->mileage() = 0;
    }
    else if (origin == "YYC")
    {
      if (dest == "DFW")
        mil->mileage() = 1523;
      else
        mil->mileage() = 0;
    }
    else
      mil->mileage() = 0;

    return mil;
  }
};
}

class TaxUtility_US2validStop : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUtility_US2validStop);
  CPPUNIT_TEST(US2validStop_mirrorImage);
  CPPUNIT_TEST(US2validStop_toUS);
  CPPUNIT_TEST(US2validStop_openJawFromUS);
  CPPUNIT_TEST(US2validStop_openJawToUS);
  CPPUNIT_TEST(US2validStop_byUS_USmostDistant);
  CPPUNIT_TEST(US2validStop_byUS_USnotMostDistant);
  CPPUNIT_TEST(US2validStop_byUS_USnotMostDistant_surface);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  FarePath farePath;
  std::string xmlPath;
  Itin itin;
  PricingTrx trx;
  TaxResponse taxResponse;
  TaxRequest request;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle2>();
    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    itin.travelSeg().clear();
    farePath.itin() = &itin;
    taxResponse.farePath() = &farePath;
    trx.setRequest(&request);
    trx.setOptions(_memHandle.create<PricingOptions>());
    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();
    TestFactoryManager::instance()->destroyAll();
  }

  void tearDown() { _memHandle.clear(); }

  void US2validStop_mirrorImage()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    bool ret = taxUtil::isMostDistantUS(trx, taxResponse);
    CPPUNIT_ASSERT(ret);
  }

  void US2validStop_toUS()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    bool ret = taxUtil::isMostDistantUS(trx, taxResponse);
    CPPUNIT_ASSERT(ret);
  }

  void US2validStop_openJawFromUS()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_DFW_MEX_MIA_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_DFW_MEX_MIA_1.xml"));
    bool ret = taxUtil::isMostDistantUS(trx, taxResponse);
    CPPUNIT_ASSERT(ret);
  }

  void US2validStop_openJawToUS()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_DFW_MEX_MIA_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_DFW_MEX_MIA_1.xml"));
    bool ret = taxUtil::isMostDistantUS(trx, taxResponse);
    CPPUNIT_ASSERT(ret);
  }

  void US2validStop_byUS_USmostDistant()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "1_FRA_DFW_YYC_YYZ_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "1_FRA_DFW_YYC_YYZ_1.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "1_FRA_DFW_YYC_YYZ_2.xml"));
    bool ret = taxUtil::isMostDistantUS(trx, taxResponse);
    CPPUNIT_ASSERT(!ret);
  }

  void US2validStop_byUS_USnotMostDistant()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "1_FRA_ORD_YYC_YYZ_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "1_FRA_ORD_YYC_YYZ_1.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "1_FRA_ORD_YYC_YYZ_2.xml"));
    bool ret = taxUtil::isMostDistantUS(trx, taxResponse);
    CPPUNIT_ASSERT(!ret);
  }

  void US2validStop_byUS_USnotMostDistant_surface()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "1_FRA_ORD_YYC_YYZ_0.xml"));
    ArunkSeg surfaceSegment;
    surfaceSegment.origin() = itin.travelSeg()[0]->destination();
    itin.travelSeg().push_back(&surfaceSegment);
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "1_FRA_ORD_YYC_YYZ_2.xml"));
    surfaceSegment.destination() = itin.travelSeg()[2]->origin();
    bool ret = taxUtil::isMostDistantUS(trx, taxResponse);
    CPPUNIT_ASSERT(!ret);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUtility_US2validStop);
}
