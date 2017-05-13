
#include <string>
#include "test/include/CppUnitHelperMacros.h"


#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "Taxes/Common/TaxUtility.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestXMLHelper.h"

namespace tse
{

class TaxUtility_findFareBreaks : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUtility_findFareBreaks);
  CPPUNIT_TEST(findFareBreaks_onePricingUnitOneFare);
  CPPUNIT_TEST(findFareBreaks_onePricingUnitTwoFares);
  CPPUNIT_TEST(findFareBreaks_twoPricingUnitsTwoFares);
  CPPUNIT_TEST_SUITE_END();

  std::map<uint16_t, FareUsage*> fareBreaks;
  FarePath farePath;
  std::string xmlPath;
  Itin itin;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    fareBreaks.clear();
    farePath.pricingUnit().clear();
    itin.travelSeg().clear();
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    farePath.itin() = &itin;
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void findFareBreaks_onePricingUnitOneFare()
  {
    PricingUnit pricingUnit;
    farePath.pricingUnit().push_back(&pricingUnit);
    FareUsage fareUsage;
    pricingUnit.fareUsage().push_back(&fareUsage);
    fareUsage.travelSeg().push_back(itin.travelSeg()[0]);
    fareUsage.travelSeg().push_back(itin.travelSeg()[1]);
    taxUtil::findFareBreaks(fareBreaks, farePath);
    CPPUNIT_ASSERT_EQUAL(size_t(1), fareBreaks.size());
    CPPUNIT_ASSERT(fareBreaks.find(1) != fareBreaks.end());
  }

  void findFareBreaks_onePricingUnitTwoFares()
  {
    PricingUnit pricingUnit;
    farePath.pricingUnit().push_back(&pricingUnit);
    FareUsage fareUsage;
    pricingUnit.fareUsage().push_back(&fareUsage);
    fareUsage.travelSeg().push_back(itin.travelSeg()[0]);
    FareUsage fareUsage2;
    pricingUnit.fareUsage().push_back(&fareUsage2);
    fareUsage2.travelSeg().push_back(itin.travelSeg()[1]);
    taxUtil::findFareBreaks(fareBreaks, farePath);
    CPPUNIT_ASSERT_EQUAL(size_t(2), fareBreaks.size());
    CPPUNIT_ASSERT(fareBreaks.find(0) != fareBreaks.end());
    CPPUNIT_ASSERT(fareBreaks.find(1) != fareBreaks.end());
  }

  void findFareBreaks_twoPricingUnitsTwoFares()
  {
    PricingUnit pricingUnit;
    farePath.pricingUnit().push_back(&pricingUnit);
    PricingUnit pricingUnit2;
    farePath.pricingUnit().push_back(&pricingUnit2);
    FareUsage fareUsage;
    pricingUnit.fareUsage().push_back(&fareUsage);
    FareUsage fareUsage2;
    pricingUnit2.fareUsage().push_back(&fareUsage2);
    fareUsage.travelSeg().push_back(itin.travelSeg()[0]);
    fareUsage2.travelSeg().push_back(itin.travelSeg()[1]);
    taxUtil::findFareBreaks(fareBreaks, farePath);
    CPPUNIT_ASSERT_EQUAL(size_t(2), fareBreaks.size());
    CPPUNIT_ASSERT(fareBreaks.find(0) != fareBreaks.end());
    CPPUNIT_ASSERT(fareBreaks.find(1) != fareBreaks.end());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUtility_findFareBreaks);
};
