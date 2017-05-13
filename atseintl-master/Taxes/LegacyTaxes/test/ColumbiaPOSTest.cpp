#include "test/include/CppUnitHelperMacros.h"
#include "Taxes/LegacyTaxes/ColumbiaPOS.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/DiskCache.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "Common/Config/ConfigMan.h"
#include <boost/assign/list_inserter.hpp>
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

using namespace boost::assign;

namespace tse
{
class CarrierListMock : public ColumbiaPOS::CarrierList
{
  friend class ColumbiaPOSTest;

  void parseCarriers(const std::string& carriersStr, std::set<std::string>& carriers) const
  {
    insert(carriers)("T0")("CO")("G3");
  }
};

class ColumbiaPOSTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ColumbiaPOSTest);
  CPPUNIT_TEST(testchargeUSTaxesReturnFalseIfNoValidatingCarrier);
  CPPUNIT_TEST(testchargeUSTaxesReturnFalseIfValidatingCarrierNotChargeUSTaxes);
  CPPUNIT_TEST(
      testchargeUSTaxesReturnFalseIfValidatingCarrierChargedUSTaxesMarketingCarrierNotCharged);
  CPPUNIT_TEST(testchargeMXTaxesReturnFalseIfNoValidatingCarrier);
  CPPUNIT_TEST(testchargeMXTaxesReturnFalseIfValidatingCarrierNotChargedUSTaxes);
  CPPUNIT_TEST(testCarrierListParseCarriersEmptyCarrierStr);
  CPPUNIT_TEST(testCarrierListParseCarriers);
  CPPUNIT_TEST(testCarrierListGetCarriersNullConfigMan);
  CPPUNIT_TEST(testCarrierListGetCarriers);
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
  void setUp() { _memHandle.create<SpecificTestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

protected:
  void testchargeUSTaxesReturnFalseIfNoValidatingCarrier()
  {
    PricingTrx trx;
    TaxResponse response;
    ColumbiaPOS columbiaPOS;
    FarePath fp;
    response.farePath() = &fp;
    Itin itin;
    fp.itin() = &itin;

    CPPUNIT_ASSERT(!columbiaPOS.chargeUSTaxes(trx, response));
  }

  void testchargeUSTaxesReturnFalseIfValidatingCarrierNotChargeUSTaxes()
  {
    PricingTrx trx;
    TaxResponse response;
    ColumbiaPOS columbiaPOS;
    FarePath fp;
    response.farePath() = &fp;
    Itin itin;
    fp.itin() = &itin;
    itin.validatingCarrier() = "BA";

    CPPUNIT_ASSERT(!columbiaPOS.chargeUSTaxes(trx, response));
  }

  // One way trip wholly in US
  // Validation carrier charged US tax
  // Marketing carrier not charged US tax
  void testchargeUSTaxesReturnFalseIfValidatingCarrierChargedUSTaxesMarketingCarrierNotCharged()
  {
    PricingTrx trx;
    TaxResponse response;
    ColumbiaPOS columbiaPOS;
    FarePath fp;
    response.farePath() = &fp;
    Itin itin;
    fp.itin() = &itin;
    itin.validatingCarrier() = "AA";

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_ORD.xml");
    airSeg->setMarketingCarrierCode("BA");
    itin.travelSeg().push_back(airSeg);

    CPPUNIT_ASSERT(!columbiaPOS.chargeUSTaxes(trx, response));
  }

  void testchargeMXTaxesReturnFalseIfNoValidatingCarrier()
  {
    PricingTrx trx;
    TaxResponse response;
    ColumbiaPOS columbiaPOS;
    FarePath fp;
    response.farePath() = &fp;
    Itin itin;
    fp.itin() = &itin;

    CPPUNIT_ASSERT_EQUAL(columbiaPOS.chargeMXTaxes(trx, response), false);
  }

  void testchargeMXTaxesReturnFalseIfValidatingCarrierNotChargedUSTaxes()
  {
    PricingTrx trx;
    TaxResponse response;
    ColumbiaPOS columbiaPOS;
    FarePath fp;
    response.farePath() = &fp;
    Itin itin;
    fp.itin() = &itin;
    itin.validatingCarrier() = "BA";

    CPPUNIT_ASSERT_EQUAL(columbiaPOS.chargeMXTaxes(trx, response), false);
  }

  void testCarrierListParseCarriersEmptyCarrierStr()
  {
    std::string carriersStr;
    size_t carrierLen = 0;
    std::set<std::string> carriers;

    ColumbiaPOS::ChargeUSTaxes.parseCarriers(carriersStr, carriers);

    CPPUNIT_ASSERT_EQUAL(carriers.size(), carrierLen);
  }

  void testCarrierListParseCarriers()
  {
    std::string carriersStr("TA|LR|T0|CO|G3");

    std::set<std::string> carriers;

    std::set<std::string> expectedCarriers;
    insert(expectedCarriers)("TA")("LR")("T0")("CO")("G3");

    ColumbiaPOS::ChargeUSTaxes.parseCarriers(carriersStr, carriers);

    CPPUNIT_ASSERT(carriers == expectedCarriers);
  }

  void testCarrierListGetCarriersNullConfigMan()
  {
    std::string paramName = "COLUMBIA_POS_CARRIERS_CHARGE_MX";

    tse::ConfigMan* config = 0;

    CarrierListMock carrierListMock;
    std::set<std::string> carriers;
    size_t carrierLen = 0;

    carrierListMock.getCarriers(paramName, carriers, config);

    CPPUNIT_ASSERT_EQUAL(carriers.size(), carrierLen);
  }

  void testCarrierListGetCarriers()
  {
    std::string paramName = "COLUMBIA_POS_CARRIERS_CHARGE_MX";
    std::string carriersStr("T0|CO|G3");

    std::set<std::string> expectedCarriers;
    insert(expectedCarriers)("T0")("CO")("G3");

    tse::ConfigMan config;
    config.setValue(paramName, carriersStr, "TAX_SVC");

    CarrierListMock carrierListMock;
    std::set<std::string> carriers;

    carrierListMock.getCarriers(paramName, carriers, &config);

    CPPUNIT_ASSERT(carriers == expectedCarriers);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ColumbiaPOSTest);

}; // namespace
