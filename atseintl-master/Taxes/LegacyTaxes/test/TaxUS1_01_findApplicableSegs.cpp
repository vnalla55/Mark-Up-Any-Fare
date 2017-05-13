#include <string>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "Taxes/LegacyTaxes/TaxUS1_01.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestXMLHelper.h"

namespace tse
{

class TaxUS1_01_findApplicableSegs : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUS1_01_findApplicableSegs);
  //  CPPUNIT_TEST(findApplicableSegs_AllUSAndBufferZone_soldInUS_0seg);
  //  CPPUNIT_TEST(findApplicableSegs_AllUSAndBufferZone_soldInUS_1seg);
  CPPUNIT_TEST(findApplicableSegs_AllUSAndBufferZone_notSoldInUS_0seg);
  CPPUNIT_TEST(findApplicableSegs_AllUSAndBufferZone_notSoldInUS_1seg);
  // CPPUNIT_TEST(findApplicableSegs_FromInternationalToUS_soldInUS_stopInUS);
  CPPUNIT_TEST(findApplicableSegs_FromInternationalToUS_notSoldInUS_stopInUS);
  //  CPPUNIT_TEST(findApplicableSegs_FromInternationalToUS_soldInUS_notStopInUS);
  //  CPPUNIT_TEST(findApplicableSegs_FromInternationalToUS_hidden);
  //  CPPUNIT_TEST(findApplicableSegs_FromInternationalToUS_notMatchingSeg);
  //  CPPUNIT_TEST(findApplicableSegs_FromInternationalToUS_surface);
  //  CPPUNIT_TEST(checkEquipment_noReduction);
  //  CPPUNIT_TEST(checkEquipment_bus_ewr_abe);
  //  CPPUNIT_TEST(checkEquipment_train_ewr_zfv);
  //  CPPUNIT_SKIP_TEST(isAllUS_US);
  //  CPPUNIT_SKIP_TEST(isAllUS_notUS);
  CPPUNIT_TEST_SUITE_END();

  std::string xmlPath;
  Itin itin;
  PricingTrx* trx;
  TaxResponse* taxResponse;
  FarePath* farePath;
  PricingRequest* request;

public:
  void setUp()
  {
    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    itin.travelSeg().clear();
    trx = new PricingTrx();
    taxResponse = new TaxResponse();
    farePath = new FarePath();
    taxResponse->farePath() = farePath;
    farePath->itin() = &itin;
    request = new PricingRequest();
    trx->setRequest(request);
    trx->getRequest()->ticketingDT() = DateTime(2012, 5, 14, 11, 0, 0);
  }

  void tearDown()
  {
    delete farePath;
    delete taxResponse;
    delete trx;
    delete request;
  }

  void findApplicableSegs_AllUSAndBufferZone_soldInUS_0seg()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    uint16_t endIndex;
    bool res;
    tax._soldInUS = true;
    tax._allUS = true;
    res = tax.findApplicableSegs(*trx, *taxResponse, 0, endIndex, false);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT_EQUAL(endIndex, uint16_t(1));
  }

  void findApplicableSegs_AllUSAndBufferZone_soldInUS_1seg()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    uint16_t endIndex;
    bool res;
    tax._soldInUS = true;
    tax._allUS = true;
    res = tax.findApplicableSegs(*trx, *taxResponse, 1, endIndex, false);
    CPPUNIT_ASSERT(res);
  }

  void findApplicableSegs_AllUSAndBufferZone_notSoldInUS_0seg()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    uint16_t endIndex;
    bool res;
    tax._soldInUS = false;
    res = tax.findApplicableSegs(*trx, *taxResponse, 0, endIndex, false);
    CPPUNIT_ASSERT(!res);
  }

  void findApplicableSegs_AllUSAndBufferZone_notSoldInUS_1seg()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    uint16_t endIndex;
    bool res;
    tax._soldInUS = false;
    res = tax.findApplicableSegs(*trx, *taxResponse, 1, endIndex, false);
    CPPUNIT_ASSERT(!res);
  }

  void findApplicableSegs_FromInternationalToUS_soldInUS_stopInUS()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(
        TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1_forcedStop.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    uint16_t endIndex;
    bool res;
    tax._soldInUS = true;
    res = tax.findApplicableSegs(*trx, *taxResponse, 2, endIndex, false);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT_EQUAL(endIndex, uint16_t(2));
  }

  void findApplicableSegs_FromInternationalToUS_notSoldInUS_stopInUS()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(
        TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1_forcedStop.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    uint16_t endIndex;
    bool res;
    tax._soldInUS = false;
    res = tax.findApplicableSegs(*trx, *taxResponse, 2, endIndex, false);
    CPPUNIT_ASSERT(!res);
  }

  void findApplicableSegs_FromInternationalToUS_soldInUS_notStopInUS()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    uint16_t endIndex;
    bool res;
    tax._soldInUS = true;
    res = tax.findApplicableSegs(*trx, *taxResponse, 2, endIndex, false);
    CPPUNIT_ASSERT(!res);
  }

  void findApplicableSegs_FromInternationalToUS_hidden()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(
        TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1_forcedStop.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    Loc hl = *(itin.travelSeg()[2]->destination());
    itin.travelSeg()[1]->hiddenStops().push_back(&hl);
    uint16_t endIndex;
    bool res;
    tax._soldInUS = true;
    res = tax.findApplicableSegs(*trx, *taxResponse, 1, endIndex, false);
    CPPUNIT_ASSERT(res);
  }

  void findApplicableSegs_FromInternationalToUS_notMatchingSeg()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(
        TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1_forcedStop.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    uint16_t endIndex;
    bool res;
    tax._soldInUS = true;
    res = tax.findApplicableSegs(*trx, *taxResponse, 1, endIndex, false);
    CPPUNIT_ASSERT(!res);
  }

  void findApplicableSegs_FromInternationalToUS_surface()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg()[1]->segmentType() = Arunk;
    uint16_t endIndex;
    bool res;
    tax._soldInUS = true;
    res = tax.findApplicableSegs(*trx, *taxResponse, 0, endIndex, false);
    CPPUNIT_ASSERT(!res);
  }

  void checkEquipment_noReduction()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    MoneyAmount money = 100.00;
    tax.checkEquipment(money, 0, 1, *trx, itin);
    CPPUNIT_ASSERT_EQUAL(100.0, money);
  }

  void checkEquipment_bus_ewr_abe()
  {

    TaxUS1_01 tax;
    AirSeg* aSeg = TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml");
    aSeg->equipmentType() = BUS;
    LocCode& loc1 = const_cast<LocCode&>(aSeg->origin()->loc());
    LocCode& loc2 = const_cast<LocCode&>(aSeg->destination()->loc());
    loc1 = "ABE";
    loc2 = "EWR";
    aSeg->carrier() = "CO";
    itin.travelSeg().push_back(aSeg);
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    MoneyAmount money = 100.00;
    tax.checkEquipment(money, 0, 1, *trx, itin);
    CPPUNIT_ASSERT_EQUAL(100.0 - 29, money);
  }

  void checkEquipment_train_ewr_zfv()
  {
    TaxUS1_01 tax;
    AirSeg* aSeg = TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml");
    aSeg->equipmentType() = TRAIN;
    LocCode& loc1 = const_cast<LocCode&>(aSeg->origin()->loc());
    LocCode& loc2 = const_cast<LocCode&>(aSeg->destination()->loc());
    loc1 = "EWR";
    loc2 = "ZFV";
    aSeg->carrier() = "CO";
    itin.travelSeg().push_back(aSeg);
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    MoneyAmount money = 100.00;
    tax.checkEquipment(money, 0, 1, *trx, itin);
    CPPUNIT_ASSERT_EQUAL(100.0 - 46, money);
  }

  void isAllUS_notUS()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(
        TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1_forcedStop.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    bool res;
    res = tax.isAllUS(*itin.farePath().front());
    CPPUNIT_ASSERT(!res);
  }

  void isAllUS_US()
  {
    TaxUS1_01 tax;
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    bool res;
    res = tax.isAllUS(*itin.farePath().front());
    CPPUNIT_ASSERT(res);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUS1_01_findApplicableSegs);

} // tse
