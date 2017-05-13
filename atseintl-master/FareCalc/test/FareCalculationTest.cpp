#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "FareCalc/FareCalculation.h"

#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "FareCalc/FareCalcCollector.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

namespace
{
enum agentIdEnum
{
  SABRE_AGENT,
  INFINI_AGENT,
  AXESS_AGENT,
  ABACUS_AGENT
};

enum matchingEnum
{
  MATCH_ONLY,
  MATCH_NOMATCH,
  NOMATCH_ONLY
};

const std::string strWpaXm = "SEE OTHER FARES - USE WPA$XM\n";
const std::string strWpaAl = "SEE ALL FARES - USE WPA$AL\n";
const std::string strWpasXm = "SEE OTHER FARES - USE WPAS$XM\n";
const std::string strWpasAl = "SEE ALL FARES - USE WPAS$AL\n";
}

class FareCalculationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareCalculationTest);
  CPPUNIT_TEST(testGetNetRemitFbc_Fail_NoFareUsage);
  CPPUNIT_TEST(testGetNetRemitFbc_Fail_NoNetRemitPscResults);
  CPPUNIT_TEST(testGetNetRemitFbc_Fail_NoTravelSegFound);
  CPPUNIT_TEST(testGetNetRemitFbc_Fail_NotEnabled);
  CPPUNIT_TEST(testGetNetRemitFbc_Pass_On_First);
  CPPUNIT_TEST(testGetNetRemitFbc_Pass_On_Middle);
  CPPUNIT_TEST(testGetNetRemitFbc_Pass_On_Last);

  CPPUNIT_TEST(testDisplayBookingCodeWarning);
  CPPUNIT_TEST(testDisplayIndustryAndGoverningWarning_gov);
  CPPUNIT_TEST(testDisplayIndustryAndGoverningWarning_ind);
  CPPUNIT_TEST(testDisplayCommandPricingAndVariousMessages_DAEntry);
  CPPUNIT_TEST(testDisplayCommandPricingAndVariousMessages_DAEntry2);
  CPPUNIT_TEST(testDisplayCommandPricingAndVariousMessages_SegmentFee);
  CPPUNIT_TEST(testDisplayCommandPricingAndVariousMessages_empty);

  // Override
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WP);

  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPA_MatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPA_MatchNoMatch);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPA_NoMatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAcXM_MatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAcXM_MatchNoMatch);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAcXM_NoMatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAcAL_MatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAcAL_MatchNoMatch);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAcAL_NoMatchOnly);

  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAS_MatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAS_MatchNoMatch);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAS_NoMatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAScXM_MatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAScXM_MatchNoMatch);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAScXM_NoMatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAScAL_MatchOnly);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAScAL_MatchNoMatch);
  CPPUNIT_TEST(testDisplayWpaTrailerMessage_WPAScAL_NoMatchOnly);

  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FareUsage* _fareUsage;
  PricingOptions* _options;
  PaxTypeFare* _paxTypeFare;
  FareCalculation* _fareCalculation;

  void setupGetNetRemitFbc()
  {
    _trx = _memHandle.create<PricingTrx>();
    _options = _memHandle.create<PricingOptions>();

    _options->fareCalculationDisplay() = '1';
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _trx->setOptions(_options);
    _fareUsage = _memHandle.create<FareUsage>();
    AirSeg* first = _memHandle.create<AirSeg>();
    first->segmentOrder() = 1;
    AirSeg* second = _memHandle.create<AirSeg>();
    second->segmentOrder() = 2;
    AirSeg* last = _memHandle.create<AirSeg>();
    last->segmentOrder() = 3;

    _fareUsage->travelSeg().push_back(first);
    _fareUsage->travelSeg().push_back(second);
    _fareUsage->travelSeg().push_back(last);
    _fareUsage->paxTypeFare() = _paxTypeFare;

    NegFareRestExtSeq* nfrExtSeq = _memHandle.create<NegFareRestExtSeq>();
    nfrExtSeq->uniqueFareBasis() = "UNFBC";

    _fareUsage->netRemitPscResults().push_back(
        FareUsage::TktNetRemitPscResult(first, last, nfrExtSeq, NULL));

    TestConfigInitializer::setValue("ACTIVATE_OPTIMUS_NET_REMIT", "Y", "PRICING_SVC", true);
    TrxUtil::enableAbacus();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    constructFareCalculation();
  }

  void tearDown() { _memHandle.clear(); }

  void constructFareCalculation()
  {
    _trx = constructTrx();
    _fareCalculation = _memHandle.create<FareCalculation>();
    FareCalcConfig* fcConfig = _memHandle.create<FareCalcConfig>();
    FareCalcCollector* fareCalcCollector = _memHandle.create<FareCalcCollector>();
    _fareCalculation->initialize(_trx, fcConfig, fareCalcCollector);
    fcConfig->wpaPermitted() = FareCalcConsts::FC_YES;
  }

  FareCalculation* makeFareCalculation()
  {
    PricingTrx* trx = constructTrx();
    FareCalculation* fareCalculation = _memHandle.create<FareCalculation>();
    FareCalcConfig* fcConfig = _memHandle.create<FareCalcConfig>();
    FareCalcCollector* fareCalcCollector = _memHandle.create<FareCalcCollector>();

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->paxType() = _memHandle.create<PaxType>();
    fareCalcCollector->getCalcTotals(trx, farePath, fcConfig);

    fareCalculation->initialize(trx, fcConfig, fareCalcCollector);
    return fareCalculation;
  }

  PricingTrx* constructTrx()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingRequest* pRequest = _memHandle.insert(new PricingRequest);
    Agent* pAgent = _memHandle.insert(new Agent);
    Customer* pCustomer = _memHandle.insert(new Customer);
    Billing* pBilling = _memHandle.insert(new Billing);
    Itin* itin = _memHandle.insert(new Itin);
    PricingOptions* options = _memHandle.create<PricingOptions>();

    trx->setOptions(options);
    pRequest->ticketingAgent() = pAgent;
    pRequest->ticketingAgent()->agentTJR() = pCustomer;
    pRequest->majorSchemaVersion() = 1;
    pRequest->minorSchemaVersion() = 1;
    pRequest->revisionSchemaVersion() = 1;

    trx->setRequest(pRequest);
    trx->itin().push_back(itin);
    trx->billing() = pBilling;

    return trx;
  }

  // TESTS
  void testGetNetRemitFbc_Fail_NoFareUsage()
  {
    setupGetNetRemitFbc();
    CPPUNIT_ASSERT_EQUAL(
        std::string(""),
        FareCalculation::getNetRemitFbc(*_trx, NULL, _fareUsage->travelSeg().front()));
  }

  void testGetNetRemitFbc_Fail_NoNetRemitPscResults()
  {
    setupGetNetRemitFbc();
    _fareUsage->netRemitPscResults().clear();
    CPPUNIT_ASSERT_EQUAL(
        std::string(""),
        FareCalculation::getNetRemitFbc(*_trx, _fareUsage, _fareUsage->travelSeg().front()));
  }

  void testGetNetRemitFbc_Fail_NoTravelSegFound()
  {
    AirSeg airseg;
    setupGetNetRemitFbc();
    CPPUNIT_ASSERT_EQUAL(std::string(""),
                         FareCalculation::getNetRemitFbc(*_trx, _fareUsage, &airseg));
  }

  void testGetNetRemitFbc_Fail_NotEnabled()
  {

    setupGetNetRemitFbc();
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "NOT";
    CPPUNIT_ASSERT_EQUAL(
        std::string(""),
        FareCalculation::getNetRemitFbc(*_trx, _fareUsage, _fareUsage->travelSeg().front()));
  }

  void testGetNetRemitFbc_Pass_On_First()
  {

    setupGetNetRemitFbc();
    CPPUNIT_ASSERT_EQUAL(
        std::string("UNFBC"),
        FareCalculation::getNetRemitFbc(*_trx, _fareUsage, _fareUsage->travelSeg().front()));
  }

  void testGetNetRemitFbc_Pass_On_Middle()
  {
    setupGetNetRemitFbc();
    CPPUNIT_ASSERT_EQUAL(
        std::string("UNFBC"),
        FareCalculation::getNetRemitFbc(*_trx, _fareUsage, *(_fareUsage->travelSeg().begin() + 1)));
  }

  void testGetNetRemitFbc_Pass_On_Last()
  {
    setupGetNetRemitFbc();
    CPPUNIT_ASSERT_EQUAL(
        std::string("UNFBC"),
        FareCalculation::getNetRemitFbc(*_trx, _fareUsage, _fareUsage->travelSeg().back()));
  }

  void compareFcMsg(const FcMessage& first, const FcMessage& second) const
  {
    CPPUNIT_ASSERT_EQUAL(first.messageType(), second.messageType());
    CPPUNIT_ASSERT_EQUAL(first.messageText(), second.messageText());
    CPPUNIT_ASSERT_EQUAL(first.messagePrefix(), second.messagePrefix());
  }

  void testDisplayBookingCodeWarning()
  {
    _fareCalculation->_trx->getOptions()->bookingCodeOverride() = true;
    CalcTotals* calcTotals = createCalcTotals();
    _fareCalculation->displayCommandPricingAndVariousMessages(calcTotals);

    std::vector<FcMessage>& fcMessage = calcTotals->fcMessage;
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "  ", false), fcMessage[0]);
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "**", false), fcMessage[1]);
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "PRICED USING BOOKING CODE OVERRIDE", false),
                 fcMessage[2]);
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "FARE NOT GUARANTEED IF TICKETED", false),
                 fcMessage[3]);
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "**", false), fcMessage[4]);

    CPPUNIT_ASSERT_EQUAL(std::string("  \n"
                                     "**\n"
                                     "PRICED USING BOOKING CODE OVERRIDE\n"
                                     "FARE NOT GUARANTEED IF TICKETED\n"
                                     "**\n"),
                         _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayIndustryAndGoverningWarning_gov()
  {
    FareCalculation* fareCalculation = makeFareCalculation();
    std::map<int16_t, CarrierCode>& over =
        fareCalculation->_trx->getRequest()->governingCarrierOverrides();
    over[1] = "AA";

    CPPUNIT_ASSERT_EQUAL(1, (int)fareCalculation->_fcCollector->calcTotalsMap().size());
    fareCalculation->displayCommandPricingAndVariousMessages();

    FareCalcCollector::CalcTotalsMap::const_iterator it =
        fareCalculation->_fcCollector->calcTotalsMap().begin();
    for (; it != fareCalculation->_fcCollector->calcTotalsMap().end(); ++it)
    {
      std::vector<FcMessage>& fcMessage = it->second->fcMessage;
      CPPUNIT_ASSERT_EQUAL(5, (int)fcMessage.size());
      compareFcMsg(FcMessage(FcMessage::WARNING, 0, "  ", false), fcMessage[0]);
      compareFcMsg(FcMessage(FcMessage::WARNING, 0, "**", false), fcMessage[1]);
      compareFcMsg(FcMessage(FcMessage::WARNING, 0, "PRICED USING AA FARE OVERRIDE", false),
                   fcMessage[2]);
      compareFcMsg(FcMessage(FcMessage::WARNING, 0, "FARE NOT GUARANTEED IF TICKETED", false),
                   fcMessage[3]);
      compareFcMsg(FcMessage(FcMessage::WARNING, 0, "**", false), fcMessage[4]);
    }

    CPPUNIT_ASSERT_EQUAL(std::string("  \n"
                                     "**\n"
                                     "PRICED USING AA FARE OVERRIDE\n"
                                     "FARE NOT GUARANTEED IF TICKETED\n"
                                     "**\n"),
                         fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayIndustryAndGoverningWarning_ind()
  {
    std::vector<int16_t>& indFO = _fareCalculation->_trx->getRequest()->industryFareOverrides();
    indFO.push_back(1);

    CalcTotals* calcTotals = createCalcTotals();
    _fareCalculation->displayCommandPricingAndVariousMessages(calcTotals);

    std::vector<FcMessage>& fcMessage = calcTotals->fcMessage;

    CPPUNIT_ASSERT_EQUAL(5, (int)fcMessage.size());
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "  ", false), fcMessage[0]);
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "**", false), fcMessage[1]);
    compareFcMsg(
        FcMessage(FcMessage::WARNING, 0, "PRICED USING YY / INDUSTRY FARES / OVERRIDE", false),
        fcMessage[2]);
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "FARE NOT GUARANTEED IF TICKETED", false),
                 fcMessage[3]);
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "**", false), fcMessage[4]);

    CPPUNIT_ASSERT_EQUAL(std::string("  \n"
                                     "**\n"
                                     "PRICED USING YY / INDUSTRY FARES / OVERRIDE\n"
                                     "FARE NOT GUARANTEED IF TICKETED\n"
                                     "**\n"),
                         _fareCalculation->_fareCalcDisp.str());
  }

  CalcTotals* createCalcTotals()
  {
    CalcTotals* calcTotals = _memHandle.create<CalcTotals>();
    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = _memHandle.create<Itin>();
    calcTotals->farePath = farePath;
    return calcTotals;
  }

  void testDisplayCommandPricingAndVariousMessages_DAEntry()
  {
    std::vector<DiscountAmount> discAmounts = _fareCalculation->_trx->getRequest()->getDiscountAmountsNew();
    discAmounts.push_back(DiscountAmount(1, "USD", 1, 1));
    _fareCalculation->_trx->getRequest()->setDiscountAmountsNew(discAmounts);
    _fareCalculation->_warning = "TEST ";

    CalcTotals* calcTotals = createCalcTotals();
    _fareCalculation->displayCommandPricingAndVariousMessages(calcTotals);

    std::vector<FcMessage>& fcMessage = calcTotals->fcMessage;
    CPPUNIT_ASSERT_EQUAL(1, (int)fcMessage.size());
    compareFcMsg(FcMessage(FcMessage::WARNING, 0, "MANUAL DISCOUNT APPLIED/VERIFY ALL RULES", true),
                 fcMessage[0]);

    CPPUNIT_ASSERT_EQUAL(std::string("TEST MANUAL DISCOUNT APPLIED/VERIFY ALL RULES\n"),
                         _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayCommandPricingAndVariousMessages_DAEntry2()
  {
    FareCalculation* fareCalculation = makeFareCalculation();

    std::vector<DiscountAmount> discAmounts = fareCalculation->_trx->getRequest()->getDiscountAmountsNew();
    discAmounts.push_back(DiscountAmount(1, "USD", 1, 1));
    fareCalculation->_trx->getRequest()->setDiscountAmountsNew(discAmounts);
    fareCalculation->_warning = "TEST ";

    CPPUNIT_ASSERT_EQUAL(1, (int)fareCalculation->_fcCollector->calcTotalsMap().size());
    fareCalculation->displayCommandPricingAndVariousMessages();

    FareCalcCollector::CalcTotalsMap::const_iterator it =
        fareCalculation->_fcCollector->calcTotalsMap().begin();
    for (; it != fareCalculation->_fcCollector->calcTotalsMap().end(); ++it)
    {
      std::vector<FcMessage>& fcMessage = it->second->fcMessage;
      CPPUNIT_ASSERT_EQUAL(1, (int)fcMessage.size());
      compareFcMsg(
          FcMessage(FcMessage::WARNING, 0, "MANUAL DISCOUNT APPLIED/VERIFY ALL RULES", true),
          fcMessage[0]);
    }

    CPPUNIT_ASSERT_EQUAL(std::string("TEST MANUAL DISCOUNT APPLIED/VERIFY ALL RULES\n"),
                         fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayCommandPricingAndVariousMessages_SegmentFee()
  {
    _fareCalculation->_dispSegmentFeeMsg = true;
    _fareCalculation->_warning = "TEST2 ";

    CalcTotals* calcTotals = createCalcTotals();
    _fareCalculation->displayCommandPricingAndVariousMessages(calcTotals);

    std::vector<FcMessage>& fcMessage = calcTotals->fcMessage;
    CPPUNIT_ASSERT_EQUAL(1, (int)fcMessage.size());
    compareFcMsg(
        FcMessage(FcMessage::SEGMENT_FEE, 0, "TOTAL INCLUDES CARRIER IMPOSED SURCHARGES", true),
        fcMessage[0]);

    CPPUNIT_ASSERT_EQUAL(std::string("TEST2 TOTAL INCLUDES CARRIER IMPOSED SURCHARGES\n"),
                         _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayCommandPricingAndVariousMessages_empty()
  {
    _fareCalculation->_dispSegmentFeeMsg = true;
    _fareCalculation->_trx->getOptions()->bookingCodeOverride() = true;
    _fareCalculation->displayCommandPricingAndVariousMessages();

    CPPUNIT_ASSERT_EQUAL(0, (int)_fareCalculation->_fcCollector->calcTotalsMap().size());
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void addRequestedFareBasis()
  {
    TravelSeg::RequestedFareBasis rfb;
    TravelSeg* ts = _memHandle.create<AirSeg>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    ts->requestedFareBasis().push_back(rfb);
    fm->travelSeg().push_back(ts);
    _trx->fareMarket().push_back(fm);
  }

  void commonOverride(matchingEnum matching = NOMATCH_ONLY, agentIdEnum agentId = SABRE_AGENT)
  {
    switch (matching)
    {
      case MATCH_ONLY:
      {
        _fareCalculation->_allRequireRebook = false;
        _fareCalculation->_optionsToRebook.clear();
        break;
      }
      case MATCH_NOMATCH:
      {
        _fareCalculation->_allRequireRebook = false;
        _fareCalculation->_optionsToRebook.push_back(1);
        break;
      }
      case NOMATCH_ONLY:
      {
        _fareCalculation->_allRequireRebook = true;
        _fareCalculation->_optionsToRebook.clear();
        break;
      }
    }

    Customer* agent = _trx->getRequest()->ticketingAgent()->agentTJR();
    switch (agentId)
    {
      case SABRE_AGENT:
      {
        agent->crsCarrier() = SABRE_MULTIHOST_ID;
        agent->hostName() = SABRE_USER;
        break;
      }
      case INFINI_AGENT:
      {
        agent->crsCarrier() = INFINI_MULTIHOST_ID;
        agent->hostName() = INFINI_USER;
        break;
      }
      case AXESS_AGENT:
      {
        agent->crsCarrier() = AXESS_MULTIHOST_ID;
        agent->hostName() = AXESS_USER;
        break;
      }
      case ABACUS_AGENT:
      {
        agent->crsCarrier() = ABACUS_MULTIHOST_ID;
        agent->hostName() = ABACUS_USER;
        break;
      }
      default:
      {
        agent->crsCarrier() = SABRE_MULTIHOST_ID;
        agent->hostName() = SABRE_USER;
      }
    }

    _trx->altTrxType() = PricingTrx::WPA;
    _trx->getRequest()->wpas() = 'F';

    // cross XM
    _trx->getRequest()->lowFareRequested() = 'F';
    _trx->getRequest()->wpaXm() = false;
    // cross AL
    _trx->getOptions()->setMatchAndNoMatchRequested(false);
  }

  void testDisplayWpaTrailerMessage_WP()
  {
    constructFareCalculation();
    commonOverride();
    _trx->altTrxType() = PricingTrx::WP;

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPA_MatchOnly()
  {
    constructFareCalculation();
    commonOverride(MATCH_ONLY);

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(strWpaXm + strWpaAl, _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPA_MatchNoMatch()
  {
    constructFareCalculation();
    commonOverride(MATCH_NOMATCH);

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(strWpaXm + strWpaAl, _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPA_NoMatchOnly()
  {
    constructFareCalculation();
    commonOverride(NOMATCH_ONLY);

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAcXM_MatchOnly()
  {
    constructFareCalculation();
    commonOverride(MATCH_ONLY);
    _trx->getRequest()->lowFareRequested() = 'T';
    _trx->getRequest()->wpaXm() = true;

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAcXM_MatchNoMatch()
  {
    constructFareCalculation();
    commonOverride(MATCH_NOMATCH);
    _trx->getRequest()->lowFareRequested() = 'T';
    _trx->getRequest()->wpaXm() = true;

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAcXM_NoMatchOnly()
  {
    constructFareCalculation();
    commonOverride(NOMATCH_ONLY);
    _trx->getRequest()->lowFareRequested() = 'T';
    _trx->getRequest()->wpaXm() = true;

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAcAL_MatchOnly()
  {
    constructFareCalculation();
    commonOverride(MATCH_ONLY);
    _trx->getOptions()->setMatchAndNoMatchRequested(true);

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAcAL_MatchNoMatch()
  {
    constructFareCalculation();
    commonOverride(MATCH_NOMATCH);
    _trx->getOptions()->setMatchAndNoMatchRequested(true);

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAcAL_NoMatchOnly()
  {
    constructFareCalculation();
    commonOverride(NOMATCH_ONLY);
    _trx->getOptions()->setMatchAndNoMatchRequested(true);

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAS_MatchOnly()
  {
    constructFareCalculation();
    commonOverride(MATCH_ONLY);
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(strWpasXm + strWpasAl, _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAS_MatchNoMatch()
  {
    constructFareCalculation();
    commonOverride(MATCH_NOMATCH);
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(strWpasXm + strWpasAl, _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAS_NoMatchOnly()
  {
    constructFareCalculation();
    commonOverride(NOMATCH_ONLY);
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAScXM_MatchOnly()
  {
    constructFareCalculation();
    commonOverride(MATCH_ONLY);
    _trx->getRequest()->lowFareRequested() = 'T';
    _trx->getRequest()->wpaXm() = true;
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAScXM_MatchNoMatch()
  {
    constructFareCalculation();
    commonOverride(MATCH_NOMATCH);
    _trx->getRequest()->lowFareRequested() = 'T';
    _trx->getRequest()->wpaXm() = true;
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAScXM_NoMatchOnly()
  {
    constructFareCalculation();
    commonOverride(NOMATCH_ONLY);
    _trx->getRequest()->lowFareRequested() = 'T';
    _trx->getRequest()->wpaXm() = true;
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAScAL_MatchOnly()
  {
    constructFareCalculation();
    commonOverride(MATCH_ONLY);
    _trx->getOptions()->setMatchAndNoMatchRequested(true);
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAScAL_MatchNoMatch()
  {
    constructFareCalculation();
    commonOverride(MATCH_NOMATCH);
    _trx->getOptions()->setMatchAndNoMatchRequested(true);
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

  void testDisplayWpaTrailerMessage_WPAScAL_NoMatchOnly()
  {
    constructFareCalculation();
    commonOverride(NOMATCH_ONLY);
    _trx->getOptions()->setMatchAndNoMatchRequested(true);
    _trx->getRequest()->wpas() = 'T';

    _fareCalculation->displayWpaTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _fareCalculation->_fareCalcDisp.str());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(FareCalculationTest);
}
