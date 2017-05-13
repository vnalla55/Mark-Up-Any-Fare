#include "Common/Throttling.h"
#include "Common/ThrottlingPred.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxTrx.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class ThrottlingPredTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ThrottlingPredTest);

  CPPUNIT_TEST(testQualifyingSVCFail);
  CPPUNIT_TEST(testQualifyingMKTFail);
  CPPUNIT_TEST(testQualifyingSEGFail);
  CPPUNIT_TEST(testMatchPCCFail);
  CPPUNIT_TEST(testMatchLNIATAFail);
  CPPUNIT_TEST(testMatchPNRFail);
  CPPUNIT_TEST(testMatchPARTFail);
  CPPUNIT_TEST(testPricingTrxAllPasses);
  CPPUNIT_TEST(testAncillaryPricingTrxAllPasses);
  CPPUNIT_TEST(testTaxTrxAllPasses);
  CPPUNIT_TEST(testShoppingTrxFailOnSeg);
  CPPUNIT_TEST(testShoppingTrxAllPasses);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  Billing* _billing;
  PricingOptions* _options;
  PricingRequest* _request;
  Itin* _itin1;
  Itin* _itin2;
  AirSeg* _seg1;
  AirSeg* _seg2;
  AirSeg* _seg3;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _billing = _memHandle.create<Billing>();
    _options = _memHandle.create<PricingOptions>();
    _request = _memHandle.create<PricingRequest>();

    _itin1 = _memHandle.create<Itin>();
    _itin2 = _memHandle.create<Itin>();

    _seg1 = _memHandle.create<AirSeg>();
    _seg2 = _memHandle.create<AirSeg>();
    _seg3 = _memHandle.create<AirSeg>();

    _itin1->travelSeg().push_back(_seg1);

    _itin2->travelSeg().push_back(_seg2);
    _itin2->travelSeg().push_back(_seg3);

    _seg1->boardMultiCity() = "LON";
    _seg1->offMultiCity() = "LAX";

    _seg2->boardMultiCity() = "LON";
    _seg2->offMultiCity() = "NYC";

    _seg3->boardMultiCity() = "NYC";
    _seg3->offMultiCity() = "LAX";

    Agent* _agent = _memHandle.create<Agent>();
    _request->ticketingAgent() = _agent;

    _agent->tvlAgencyPCC() = "WT4H";
  }

  void tearDown() { _memHandle.clear(); }

  void setupTrx(PricingTrx& trx)
  {
    trx.billing() = _billing;

    _billing->parentServiceName() = "INTDWPI1";
    _billing->aaaCity() = "80K2";

    _billing->userSetAddress() = "193988";

    trx.setOptions(_options);
    _options->pnr() = "123456";

    _billing->partitionID() = "AA";

    trx.itin().push_back(_itin1);
    trx.itin().push_back(_itin2);
  }

  void setupTrx(AncillaryPricingTrx& trx)
  {
    setupTrx(static_cast<PricingTrx&>(trx));

    trx.setRequest(_request);
  }

  void setupTrx(TaxTrx& trx)
  {
    setupTrx(static_cast<PricingTrx&>(trx));

    _billing->userPseudoCityCode() = "BT88";
  }

  void setupTrx(ShoppingTrx& trx)
  {
    setupTrx(static_cast<PricingTrx&>(trx));

    ShoppingTrx::Leg leg1;
    ShoppingTrx::Leg leg2;
    trx.legs().push_back(leg1);
    trx.legs().push_back(leg2);
  }

  void testQualifyingSVCFail()
  {
    ParseCustomer parser("PCC-80K2&LNIATA-193988&PNR-123456&PART-AA=CON-2:SVC-INTDWPI2&MKT-LONLAX&SEG<3");
    PricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
  }

  void testQualifyingMKTFail()
  {
    ParseCustomer parser("PCC-80K2&LNIATA-193988&PNR-123456&PART-AA=CON-2:SVC-INTDWPI2&MKT-LONNYC&SEG<3");
    PricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
  }

  void testQualifyingSEGFail()
  {
    ParseCustomer parser("PCC-80K2&LNIATA-193988&PNR-123456&PART-AA=CON-2:SVC-INTDWPI1&MKT-LONLAX&SEG<2");
    PricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
  }

  void testMatchPCCFail()
  {
    ParseCustomer parser("PCC-K280&LNIATA-193988&PNR-123456&PART-AA=CON-2:SVC-INTDWPI1&MKT-LONLAX&SEG<3");
    PricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
  }

  void testMatchLNIATAFail()
  {
    ParseCustomer parser("PCC-80K2&LNIATA-193000&PNR-123456&PART-AA=CON-2:SVC-INTDWPI1&MKT-LONLAX&SEG<3");
    PricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
  }

  void testMatchPNRFail()
  {
    ParseCustomer parser("PCC-80K2&LNIATA-193988&PNR-654321&PART-AA=CON-2:SVC-INTDWPI1&MKT-LONLAX&SEG<3");
    PricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
  }

  void testMatchPARTFail()
  {
    ParseCustomer parser("PCC-80K2&LNIATA-193988&PNR-123456&PART-UA=CON-2:SVC-INTDWPI1&MKT-LONLAX&SEG<3");
    PricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
  }

  void testPricingTrxAllPasses()
  {
    ParseCustomer parser("PCC-80K2&LNIATA-193988&PNR-123456&PART-AA=CON-2:SVC-INTDWPI1&MKT-LONLAX&SEG<3");
    PricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(!parser.test(trx));
  }

  void testAncillaryPricingTrxAllPasses()
  {
    ParseCustomer parser("PCC-WT4H&LNIATA-193988&PNR-123456&PART-AA=CON-2:SVC-INTDWPI1&MKT-LONLAX&SEG<3");
    AncillaryPricingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(!parser.test(trx));
  }

  void testTaxTrxAllPasses()
  {
    ParseCustomer parser("PCC-BT88&LNIATA-193988&PNR-123456&PART-AA=CON-2:SVC-INTDWPI1&MKT-LONLAX&SEG<3");
    TaxTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(!parser.test(trx));
  }

  void testShoppingTrxFailOnSeg()
  {
    ParseCustomer parser("PCC-BT88&LNIATA-193988&PNR-123456&PART-AA=CON-2:SVC-INTDWPI1&SEG<2");
    ShoppingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
  }

  void testShoppingTrxAllPasses()
  {
    ParseCustomer parser("PCC-80K2&LNIATA-193988&PART-AA=CON-2:SVC-INTDWPI1&SEG<3");
    ShoppingTrx trx;
    Throttling throttling(trx);
    setupTrx(trx);

    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(parser.test(trx));
    CPPUNIT_ASSERT(!parser.test(trx));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ThrottlingPredTest);

} // end of tse namespace

