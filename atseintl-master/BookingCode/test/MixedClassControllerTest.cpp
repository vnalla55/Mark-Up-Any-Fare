#include <boost/assign/std/vector.hpp>
#include <time.h>
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MiscFareTag.h"
#include "BookingCode/MixedClassController.h"
#include "BookingCode/DifferentialValidator.h"
#include "Diagnostic/Diag413Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DCFactory.h"
#include "Common/CabinType.h"
#include "Common/ClassOfService.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/Vendor.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "test/include/CppUnitHelperMacros.h"
#include "Common/Config/ConfigMan.h"

using namespace boost::assign;

namespace tse
{

class TestMixedClassController : public MixedClassController
{
public:
  CarrierPreference* preference;
  TestMixedClassController(PricingTrx& trx) : MixedClassController(trx) {}
  virtual ~TestMixedClassController() {}
  const CarrierPreference* getCarrierPreference() { return preference; }
};

class MixedClassControllerTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(MixedClassControllerTest);

  CPPUNIT_TEST(testValidForDiffCalculationFailsWhenCat23Exists);
  CPPUNIT_TEST(testValidForDiffCalculationPassesWhenNotCat23AndAllowDiff);
  CPPUNIT_TEST(testValidForDiffCalculationFailsForHighCabin4);
  CPPUNIT_TEST(testValidForDiffCalculationFailsForHighCabin7);
  CPPUNIT_TEST(testValidForDiffCalculationPassesWithNoCxrPref);
  CPPUNIT_TEST(testValidForDiffCalculationPassesWhenApplyPremBusCabinDiffCalcSet);
  CPPUNIT_TEST(testValidForDiffCalculationPassesWhenApplyPremEconCabinDiffCalcSet);
  CPPUNIT_TEST(testServiceDeterminationNormalFareExpectedDIFFERENTIALNoRBDinCOSvcExists);
  CPPUNIT_TEST(testServiceDeterminationNormalFareExpectedDIFFERENTIALRBDinCOSvcExists);
  CPPUNIT_TEST(testServiceDeterminationNormalFareExpectedDIFFERENTIALnoRBDinCOSvcExistsWPNC);
  CPPUNIT_TEST(testServiceDeterminationNormalFareExpectedDIFFERENTIALisRBDinCOSvcExistsWPNCRebooked);
  CPPUNIT_TEST(testServiceDeterminationSpecialFareExpectedERRORhigherCabin);
  CPPUNIT_TEST(testServiceDeterminationSpecialFareExpectedERRORlowCabin);

  CPPUNIT_TEST_SUITE_END();

public:
  MixedClassController* controller;
  PaxTypeFare* paxTypeFare;
  FareMarket* market;
  Itin* itin;
  FareUsage fu;
  PricingTrx* trx;
  PricingRequest* req;
  TestMemHandle _memHandle;

  void setUp()
  {
    trx = _memHandle.create<PricingTrx>();
    controller = _memHandle.insert<TestMixedClassController>(new TestMixedClassController(*trx));
    paxTypeFare = _memHandle.create<PaxTypeFare>();
    market = _memHandle.create<FareMarket>();
    itin = _memHandle.create<Itin>();
    req = _memHandle.create<PricingRequest>();
    _memHandle.create<TestConfigInitializer>();
    initializeController();
    CPPUNIT_ASSERT(!isCat23(*paxTypeFare));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void initializeController()
  {
    controller->_itin = itin;
    controller->_paxTfare = paxTypeFare;
    controller->_mkt = market;
    controller->_fareUsage = &fu;
    PaxTypeFare::BookingCodeStatus bks;
    controller->bookingCodeStatus() = bks;
    controller->_mkt->governingCarrier() = "SQ";
    trx->setRequest(req);
    DateTime ltime = DateTime(2010, 3, 9, 8, 15, 0);
    req->ticketingDT() = ltime;
  }

  bool isCat23(const PaxTypeFare& paxTypeFare)
  {
    DifferentialValidator diffV;
    diffV.diffTrx() = trx;
    return !diffV.validateMiscFareTag(paxTypeFare);
  }

  void testValidForDiffCalculationFailsWhenCat23Exists()
  {
    MiscFareTag miscFareTag;
    miscFareTag.diffcalcInd() = 'R';
    paxTypeFare->miscFareTag() = &miscFareTag;
    CPPUNIT_ASSERT(isCat23(*paxTypeFare));

    assertValidForDiffCalculationFailsWithSeqNumber(23);
  }

  void testValidForDiffCalculationPassesWhenNotCat23AndAllowDiff()
  {
    assertValidForDiffCalculationPasses();
  }

  void testValidForDiffCalculationPassesWhenApplyPremBusCabinDiffCalcSet()
  {
    CarrierPreference cxrPref;
    ((TestMixedClassController*)controller)->preference = &cxrPref;

    cxrPref.applyPremBusCabinDiffCalc() = 'Y';

    setBusinessClass();
    controller->_isHighCabin4 = true;

    assertValidForDiffCalculationPasses();
  }

  void testValidForDiffCalculationPassesWhenApplyPremEconCabinDiffCalcSet()
  {
    CarrierPreference cxrPref;
    ((TestMixedClassController*)controller)->preference = &cxrPref;

    cxrPref.applyPremEconCabinDiffCalc() = 'Y';

    setEconomyClass();
    controller->_isHighCabin7 = true;

    assertValidForDiffCalculationPasses();
  }

  void testValidForDiffCalculationFailsForHighCabin4()
  {
    CarrierPreference cxrPref;
    ((TestMixedClassController*)controller)->preference = &cxrPref;
    cxrPref.applyPremBusCabinDiffCalc() = 'N';

    setBusinessClass();
    controller->_isHighCabin4 = true;

    if (controller->_isHighCabin4)

      assertValidForDiffCalculationFailsWithSeqNumber(4);
  }

  void testValidForDiffCalculationFailsForHighCabin7()
  {
    CarrierPreference cxrPref;
    ((TestMixedClassController*)controller)->preference = &cxrPref;
    cxrPref.applyPremEconCabinDiffCalc() = 'N';

    setEconomyClass();
    controller->_isHighCabin7 = true;

    assertValidForDiffCalculationFailsWithSeqNumber(7);
  }

  void testValidForDiffCalculationPassesWithNoCxrPref()
  {
    ((TestMixedClassController*)controller)->preference = 0;
    setEconomyClass();
    controller->_isHighCabin7 = true;

    assertValidForDiffCalculationPasses();
  }

  void assertValidForDiffCalculationPasses()
  {
    CPPUNIT_ASSERT(controller->validForDiffCalculation());
    CPPUNIT_ASSERT_EQUAL(0, (int)controller->_fareUsage->differSeqNumber());
  }

  void assertValidForDiffCalculationFailsWithSeqNumber(int seqNumber)
  {
    CPPUNIT_ASSERT(!controller->validForDiffCalculation());
    CPPUNIT_ASSERT_EQUAL(seqNumber, (int)controller->_fareUsage->differSeqNumber());
    CPPUNIT_ASSERT_EQUAL(MixedClassController::FAIL_DIFF, controller->_failMixed);
    CPPUNIT_ASSERT(controller->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT(!controller->bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED));
  }

  void setBusinessClass()
  {
    CabinType cabin;
    cabin.setBusinessClass();
    paxTypeFare->cabin() = cabin;
  }

  void setEconomyClass()
  {
    CabinType cabin;
    cabin.setEconomyClass();
    paxTypeFare->cabin() = cabin;
  }

  void setPremiumFirstClass()
  {
    CabinType cabin;
    cabin.setPremiumFirstClass();
    paxTypeFare->cabin() = cabin;
  }

  void setFirstClass()
  {
    CabinType cabin;
    cabin.setFirstClass();
    paxTypeFare->cabin() = cabin;
  }

  void setUpBkgCodeStatus()
  {
    std::vector<TravelSeg*>::iterator iterTvl = controller->_mkt->travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = controller->_mkt->travelSeg().end();

    for (int i = 0; iterTvl != iterTvlEnd; iterTvl++, i++) // Initialize
    {
      PaxTypeFare::SegmentStatus& segStat = controller->_paxTfare->segmentStatus()[i];
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl))
      {
        if (!i)
        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
        }
        else
        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
        }
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }
    }
  }

  void testServiceDeterminationNormalFareExpectedDIFFERENTIALNoRBDinCOSvcExists()
  {
    // Through fare cabin is 5 - Business
    // First travel segment booked in 'F' - cabin 2 (First Class)
    // Second travel segment booked in 'C' - cabin 5 (Business)
    populateController(
        "testdata/PC1/PaxTypeFare_th.xml", "testdata/PC1/COS_A-1.xml", "testdata/PC1/COS_B-2.xml");
    setUpBkgCodeStatus();

    CPPUNIT_ASSERT_EQUAL(MixedClassController::DIFFERENTIAL, controller->serviceDetermination());
    cleanUpSvcVectors();
  }

  void testServiceDeterminationNormalFareExpectedDIFFERENTIALRBDinCOSvcExists()
  {
    // Through fare cabin is 5 - Business
    // First travel segment booked in 'B' - cabin 2 (First Class)
    // Second travel segment booked in 'C' - cabin 5 (Business)
    populateController(
        "testdata/PC1/PaxTypeFare_th.xml", "testdata/PC1/COS_A-1.xml", "testdata/PC1/COS_B-2.xml");
    controller->_mkt->travelSeg().front()->setBookingCode("B");
    setUpBkgCodeStatus();

    CPPUNIT_ASSERT_EQUAL(MixedClassController::DIFFERENTIAL, controller->serviceDetermination());
    cleanUpSvcVectors();
  }

  void testServiceDeterminationNormalFareExpectedDIFFERENTIALnoRBDinCOSvcExistsWPNC()
  {
    controller->_mccTrx.getRequest()->lowFareRequested() = 'T';
    // Through fare cabin is 5 - Business
    // First travel segment booked in 'J' - cabin 5 (Business)
    // Second travel segment booked in 'C' - cabin 5 (Business)
    populateController(
        "testdata/PC1/PaxTypeFare_th.xml", "testdata/PC1/COS_A-1.xml", "testdata/PC1/COS_K-5.xml");
    controller->_mkt->travelSeg().front()->setBookingCode("J");
    controller->_mkt->travelSeg().front()->bookedCabin().setBusinessClass();
    setUpBkgCodeStatus();

    CPPUNIT_ASSERT_EQUAL(MixedClassController::DIFFERENTIAL, controller->serviceDetermination());
    cleanUpSvcVectors();
  }

  void testServiceDeterminationNormalFareExpectedDIFFERENTIALisRBDinCOSvcExistsWPNCRebooked()
  {
    controller->_mccTrx.getRequest()->lowFareRequested() = 'T';
    // Through fare cabin is 5 - Business
    // First travel segment booked in 'J' - cabin 4 (Premium Business)
    // Second travel segment booked in 'C' - cabin 5 (Business)
    populateController(
        "testdata/PC1/PaxTypeFare_th.xml", "testdata/PC1/COS_A-1.xml", "testdata/PC1/COS_J-4.xml");
    PaxTypeFare::SegmentStatus& segStat = controller->_paxTfare->segmentStatus()[0];
    segStat._bkgCodeReBook = "J";
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);

    setUpBkgCodeStatus();

    CPPUNIT_ASSERT_EQUAL(MixedClassController::DIFFERENTIAL, controller->serviceDetermination());
    cleanUpSvcVectors();
  }

  void testServiceDeterminationSpecialFareExpectedERRORhigherCabin()
  {
    // Through fare cabin is 5 - Business
    // First travel segment booked in 'F'  - cabin 2  (First)
    // Second travel segment booked in 'C' - cabin 5 (Business)
    populateController("testdata/PC1/PaxTypeFare_th_SP.xml",
                       "testdata/PC1/COS_A-1.xml",
                       "testdata/PC1/COS_Y-8.xml");
    controller->_mkt->travelSeg().front()->setBookingCode("Y");
    controller->_mkt->travelSeg().front()->bookedCabin().setEconomyClass();

    setUpBkgCodeStatus();

    CPPUNIT_ASSERT_EQUAL(MixedClassController::ERROR, controller->serviceDetermination());
    cleanUpSvcVectors();
  }

  void testServiceDeterminationSpecialFareExpectedERRORlowCabin()
  {
    // Through fare cabin is 5 - Business
    // First travel segment booked in 'Y' - cabin 5 (Business)
    // Second travel segment booked in 'C' - cabin 5 (Business)
    populateController("testdata/PC1/PaxTypeFare_th_SP.xml",
                       "testdata/PC1/COS_A-1.xml",
                       "testdata/PC1/COS_Y-8.xml");
    controller->_mkt->travelSeg().front()->setBookingCode("Y");
    controller->_mkt->travelSeg().front()->bookedCabin().setEconomyClass();

    setUpBkgCodeStatus();

    CPPUNIT_ASSERT_EQUAL(MixedClassController::ERROR, controller->serviceDetermination());
    cleanUpSvcVectors();
  }

  void populateController(std::string farePax, std::string classSvc1, std::string classSvc2)
  {
    FareMarket* mkt = TestFareMarketFactory::create("testdata/PC1/FMarket_SINCDG-SQ.xml");
    PaxTypeFare* fare = TestPaxTypeFareFactory::create(farePax);
    controller->_paxTfare = fare;
    controller->_mkt = mkt;

    ClassOfService* csv1 = TestClassOfServiceFactory::create(classSvc1);
    ClassOfService* csv2 = TestClassOfServiceFactory::create(classSvc2);

    controller->_mkt->classOfServiceVec().clear();
    std::vector<ClassOfService*>* cos1 = new std::vector<ClassOfService*>;
    std::vector<ClassOfService*>* cos2 = new std::vector<ClassOfService*>;
    *cos1 += csv1, csv2;

    *cos2 = *cos1;
    controller->_mkt->classOfServiceVec().push_back(cos1);
    controller->_mkt->classOfServiceVec().push_back(cos2);

    controller->_paxTfare->bookingCodeStatus().set(PaxTypeFare::BKS_MIXED);
  }

  void cleanUpSvcVectors()
  {
    std::vector<std::vector<ClassOfService*>*>::iterator iter =
        controller->_mkt->classOfServiceVec().begin();
    std::vector<std::vector<ClassOfService*>*>::iterator iterEnd =
        controller->_mkt->classOfServiceVec().end();
    for (; iter != iterEnd; iter++)
    {
      delete *iter;
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MixedClassControllerTest);
}
