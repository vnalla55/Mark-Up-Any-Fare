#include "Fares/test/FareValidatorOrchestratorTestCommon.h"

#include "Common/DateTime.h"
#include "Common/ShoppingUtil.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Fares/FareValidatorOrchestrator.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FareValidatorOrchestratorForTest : public FareValidatorOrchestrator
{
public:
  FareValidatorOrchestratorForTest(const std::string& name, TseServer& server)
    : FareValidatorOrchestrator(name, server)
  {
  }

  static void fareTypes(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
  {
    FareValidatorOrchestrator::fareTypes(trx, itin, fareMarket);
  }
};

namespace
{
void setupRefundPricingTrx(RefundPricingTrx& trx, TestMemHandle& memH)

{
  trx.itin().push_back(memH.create<Itin>());
  trx.setOptions(memH.create<PricingOptions>());
  trx.setRequest(memH.create<PricingRequest>());
}
} // namespace

class PaxTypeFareDerived : public PaxTypeFare
{
public:
  PaxTypeFareDerived():PaxTypeFare(){}

  void setCarrier(CarrierCode cc) { _carrier = cc; }

};

class FareValidatorOrchestratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareValidatorOrchestratorTest);
  CPPUNIT_SKIP_TEST(testHistoricalFareValidated);
  CPPUNIT_SKIP_TEST(testTravelCommencementFareValidated);
  CPPUNIT_SKIP_TEST(testCurrentFareValidated);
  CPPUNIT_SKIP_TEST(testPrimaryValidationForKeepFare);
  CPPUNIT_TEST(testFareTypesNotProcessed);
  CPPUNIT_TEST(testFareTypesValid);
  CPPUNIT_TEST(testFareTypesNotValid);
  CPPUNIT_TEST(testDisplayValidatingCarriers);
  CPPUNIT_TEST(test_duplicatedFareCheck);
  CPPUNIT_TEST(testProcessFRRAdjustedSellingLevelPricingTrx);
  CPPUNIT_TEST(testProcessFRRAdjustedSellingLevelFDTrx);
  CPPUNIT_TEST(testProcessTicketingDateOriginal);
  CPPUNIT_TEST(testProcessTicketingDateD95Exists);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _server = memH.create<MockTseServer>();
    trx.setOriginalTktIssueDT() = DateTime(2007, 10, 1);
    trx.currentTicketingDT() = DateTime(2007, 12, 1);
    excItin.travelCommenceDate() = DateTime(2008, 1, 1);
    trx.exchangeItin().push_back(&excItin);
    trx.newItin().push_back(&newItin);

    paxTypeFare.fareMarket() = &fm;
    fm.allPaxTypeFare().push_back(&paxTypeFare);
  }
    void tearDown()
    {
      memH.clear();
    }

  void testFareTypesNotProcessed()
  {
    NoPNRPricingTrx trx;

    PaxTypeFare fare;
    fare.bookingCodeStatus().clear(PaxTypeFare::BKS_NOT_YET_PROCESSED);

    FareMarket fm1;
    fm.allPaxTypeFare().push_back(&fare);

    int val = fare.bookingCodeStatus().value();

    Itin itin;
    FareValidatorOrchestratorForTest::fareTypes(trx, itin, fm);

    CPPUNIT_ASSERT(fare.bookingCodeStatus().value() == val);

    fare.bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED);
    fare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
    val = fare.bookingCodeStatus().value();

    FareValidatorOrchestratorForTest::fareTypes(trx, itin, fm);
    CPPUNIT_ASSERT(fare.bookingCodeStatus().value() == val);
  }

  void testFareTypesValid()
  {
    NoPNRPricingTrx trx;
    NoPNRPricingTrx::FareTypes& fareTypes = trx.fareTypes();

    FareType ft = "AA";
    NoPNRPricingTrx::FareTypes::FTGroup ftg = NoPNRPricingTrx::FareTypes::FTG_BUSINESS;
    fareTypes._fareTypes.insert(
        std::pair<FareType, NoPNRPricingTrx::FareTypes::FareTypeGroup>(ft, ftg));

    PaxTypeFare fare;
    FareClassAppInfo info;
    info._fareType = ft;
    (FareClassAppInfo*&)(fare.fareClassAppInfo()) = &info;
    fare.bookingCodeStatus().clear(PaxTypeFare::BKS_NOT_YET_PROCESSED);
    fare.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);

    FareMarket fm;
    fm.allPaxTypeFare().push_back(&fare);

    Itin itin;
    FareValidatorOrchestratorForTest::fareTypes(trx, itin, fm);

    CPPUNIT_ASSERT(fare.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS));
  }

  void testFareTypesNotValid()
  {
    NoPNRPricingTrx trx;
    NoPNRPricingTrx::FareTypes& fareTypes = trx.fareTypes();

    FareType ft = "AA", ft1 = "BB";
    NoPNRPricingTrx::FareTypes::FTGroup ftg = NoPNRPricingTrx::FareTypes::FTG_BUSINESS;
    fareTypes._fareTypes.insert(
        std::pair<FareType, NoPNRPricingTrx::FareTypes::FareTypeGroup>(ft, ftg));

    PaxTypeFare fare;
    FareClassAppInfo info;
    info._fareType = ft1;
    (FareClassAppInfo*&)(fare.fareClassAppInfo()) = &info;
    fare.bookingCodeStatus().clear(PaxTypeFare::BKS_NOT_YET_PROCESSED);
    fare.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);

    FareMarket fm;
    fm.allPaxTypeFare().push_back(&fare);

    Itin itin;
    FareValidatorOrchestratorForTest::fareTypes(trx, itin, fm);

    CPPUNIT_ASSERT(fare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL) &&
                   fare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_FARE_TYPES));
  }

  void testHistoricalFareValidated()
  {
    FareValidatorOrchestratorDerived fvo("FVO", *_server);
    trx.markFareRetrievalMethodHistorical(true);
    fvo.primaryValidationForNonKeepFare(trx);
    CPPUNIT_ASSERT_EQUAL(trx.originalTktIssueDT(), fvo.validatedFareDate);
  }

  void testTravelCommencementFareValidated()
  {
    FareValidatorOrchestratorDerived fvo("FVO", *_server);
    trx.markFareRetrievalMethodTvlCommence(true);
    fvo.primaryValidationForNonKeepFare(trx);
    CPPUNIT_ASSERT_EQUAL(excItin.travelCommenceDate(), fvo.validatedFareDate);
  }

  void testCurrentFareValidated()
  {
    FareValidatorOrchestratorDerived fvo("FVO", *_server);
    trx.markFareRetrievalMethodCurrent(true);
    fvo.primaryValidationForNonKeepFare(trx);
    CPPUNIT_ASSERT_EQUAL(trx.currentTicketingDT(), fvo.validatedFareDate);
  }

  void testPrimaryValidationForKeepFare()
  {
    FareMarket::RetrievalInfo retrievalInfo;
    retrievalInfo._flag = FareMarket::RetrievKeep;
    retrievalInfo._date = DateTime(2007, 9, 30);
    paxTypeFare.retrievalInfo() = &retrievalInfo;

    trx.newItinKeepFares().insert(std::make_pair(&paxTypeFare, &fm));
    FareValidatorOrchestratorDerived fvo("FVO", *_server);
    fvo.primaryValidationForNonKeepFare(trx);
    CPPUNIT_ASSERT(&paxTypeFare == fvo.validatedFare);
    CPPUNIT_ASSERT_EQUAL(retrievalInfo._date, fvo.validatedFareDate);
  }
  void testDisplayValidatingCarriers()
  {
    fm.validatingCarriers().push_back("AA");
    fm.validatingCarriers().push_back("BA");
    fm.validatingCarriers().push_back("US");
    paxTypeFare.validatingCarriers().push_back("AA");
    paxTypeFare.validatingCarriers().push_back("BA");
    trx.setValidatingCxrGsaApplicable(true);
    fm.allPaxTypeFare().push_back(&paxTypeFare);
    paxTypeFare.fareMarket() = &fm;
    FareValidatorOrchestratorDerived fvo("FVO", *_server);
    DiagCollector& diag = createDiag();
    fvo.displayValidatingCarriers(paxTypeFare, diag);
    CPPUNIT_ASSERT_EQUAL(std::string("VAL-CXR:  AA:P BA:P US:F \n"), diag.str());

  }
  DiagCollector& createDiag()
  {
    DiagCollector* diag = memH(new DiagCollector);
    diag->activate();
    return *diag;
  }
  void test_duplicatedFareCheck()
  {
    PricingTrx trx;
    PaxTypeFare bf;
    PaxTypeFareDerived ptf1, ptf2, ptf3;
    Fare f1, f2;
    FareInfo fi1, fi2;
    FareMarket fm;
    PaxTypeFareRuleData rd;

    rd.baseFare() = &bf;

    fi1.originalFareAmount() = 100;
    fi2.originalFareAmount() = 200;

    fi1.fareClass()="FAREX1";
    fi2.fareClass()="FAREX2";

    fi1.fareTariff() = 100;
    fi2.fareTariff() = 200;

    ptf1.setCarrier("DL");
    ptf2.setCarrier("DL");
    ptf3.setCarrier("AA");

    f1.setFareInfo(&fi1);
    f2.setFareInfo(&fi2);

    ptf1.setFare(&f1);
    ptf2.setFare(&f1);
    ptf3.setFare(&f2);

    ptf1.setRuleData(25, trx.dataHandle(), &rd);
    ptf2.setRuleData(25, trx.dataHandle(), &rd);
    ptf3.setRuleData(25, trx.dataHandle(), &rd);

    fm.allPaxTypeFare().push_back(&ptf1);
    fm.allPaxTypeFare().push_back(&ptf2);
    fm.allPaxTypeFare().push_back(&ptf3);

    FareValidatorOrchestratorDerived fvo("FVO", *_server);
    Itin itin;

    PaxTypeFareDerived *ptf = &ptf2;
    for (int i=0; i< 10000; i++)
      fm.allPaxTypeFare().push_back(ptf);

    fvo.duplicatedFareCheckDerived(trx, itin, fm);

    CPPUNIT_ASSERT(ptf1.isMipUniqueFare()==true);
    CPPUNIT_ASSERT(ptf2.isMipUniqueFare()==false);
    CPPUNIT_ASSERT(ptf3.isMipUniqueFare()==true);
  }

  void testProcessFRRAdjustedSellingLevelPricingTrx()
  {
    PricingTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);

    FareValidatorOrchestratorDerived fvo("FVO", *_server);
    CPPUNIT_ASSERT(!fvo.processFRRAdjustedSellingLevel(trx, false));

    PricingOptions options;
    options.setXRSForFRRule(true);
    trx.setOptions(&options);
    trx.setExcTrxType(PricingTrx::NOT_EXC_TRX);

    CPPUNIT_ASSERT(!fvo.processFRRAdjustedSellingLevel(trx, false));

    options.setXRSForFRRule(false);
    PricingRequest request;
    Agent agent;
    request.ticketingAgent() = &agent;
    trx.setRequest(&request);

    CPPUNIT_ASSERT(fvo.processFRRAdjustedSellingLevel(trx, false));
  }

  void testProcessFRRAdjustedSellingLevelFDTrx()
  {
    FareDisplayTrx fdTrx;
    FareDisplayRequest request;
    fdTrx.setRequest(&request);
    FareDisplayOptions options;
    fdTrx.setOptions(&options);
    Agent agent;
    request.ticketingAgent() = &agent;

    fdTrx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    FareValidatorOrchestratorDerived fvo("FVO", *_server);
    CPPUNIT_ASSERT(!fvo.processFRRAdjustedSellingLevel(fdTrx, true));

    fdTrx.setExcTrxType(PricingTrx::NOT_EXC_TRX);
    options.setXRSForFRRule(true);
    CPPUNIT_ASSERT(!fvo.processFRRAdjustedSellingLevel(fdTrx, true));

    options.setXRSForFRRule(false);
    request.requestType() = ENHANCED_RD_REQUEST;
    CPPUNIT_ASSERT(!fvo.processFRRAdjustedSellingLevel(fdTrx, true));

    request.requestType() = FARE_DISPLAY_REQUEST;
    CPPUNIT_ASSERT(fvo.processFRRAdjustedSellingLevel(fdTrx, true));
  }

  void testProcessTicketingDateOriginal()
  {
    RefundPricingTrx trx;
    setupRefundPricingTrx(trx, memH);
    FareValidatorOrchestratorDerived fvo("FVO", *_server);

    trx.setOriginalTktIssueDT() = DateTime(2016, 5, 10);
    trx.previousExchangeDT() = DateTime::emptyDate();
    CPPUNIT_ASSERT(!trx.originalTktIssueDT().isEmptyDate());
    CPPUNIT_ASSERT(trx.previousExchangeDT().isEmptyDate());

    fvo.process(trx);
    CPPUNIT_ASSERT(trx.ticketingDate() == DateTime(2016, 5, 10));
  }

  void testProcessTicketingDateD95Exists()
  {
    RefundPricingTrx trx;
    setupRefundPricingTrx(trx, memH);
    FareValidatorOrchestratorDerived fvo("FVO", *_server);

    trx.setOriginalTktIssueDT() = DateTime(2016, 5, 10);
    trx.previousExchangeDT() = DateTime(2016, 6, 14);
    CPPUNIT_ASSERT(!trx.originalTktIssueDT().isEmptyDate());
    CPPUNIT_ASSERT(!trx.previousExchangeDT().isEmptyDate());

    fvo.process(trx);
    CPPUNIT_ASSERT(trx.ticketingDate() == DateTime(2016, 6, 14));
  }

protected:
  ExcItin excItin;
  Itin newItin;
  RexPricingTrx trx;
  PaxTypeFare paxTypeFare;
  FareMarket fm;
  DiagCollector* diag;
  MockTseServer* _server;
  TestMemHandle memH;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareValidatorOrchestratorTest);
}
