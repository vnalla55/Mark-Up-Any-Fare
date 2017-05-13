//----------------------------------------------------------------------------
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//----------------------------------------------------------------------------

#include "ATAE/AtaeRequest.h"

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FrequentFlyerAccount.h"
#include "DataModel/Itin.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ReservationData.h"
#include "DataModel/Traveler.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestBillingFactory.h"

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace tse
{
class AtaeRequestTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AtaeRequestTest);
  CPPUNIT_TEST(testBuild);
  CPPUNIT_TEST(testBuildShopping);

  CPPUNIT_TEST(testAddResBuildRESelement);
  CPPUNIT_TEST(testAddXrayTag);

  CPPUNIT_TEST(testAddRliDoesntBuildRLIWhenNoReservationData);
  CPPUNIT_TEST(testAddRliDoesntBuildRLIWhenRecordLocatorsInReservationData);
  CPPUNIT_TEST(testAddRliBuildRLIWhenRecordLocatorsExistInReservationData);

  CPPUNIT_TEST(testAddDiaDoesntBuildDIAWhenNotDiagnostic195);
  CPPUNIT_TEST(testAddDiaDoesntBuildDIAWhenDiagnostic195ButNoDDparameter);
  CPPUNIT_TEST(testAddDiaDoesntBuildDIAWhenDiagnostic195DDparameterWithoutNumber);
  CPPUNIT_TEST(testAddDiaBuildDIAWhenDiagnostic195DDparameterWithNumber);

  CPPUNIT_TEST(testAddPaxDoesntBuildPAXWhenNoReservationData);
  CPPUNIT_TEST(testAddPaxDoesntBuildPAXWhenNoPassengersInReservationData);
  CPPUNIT_TEST(testAddPaxDoesntBuildPAXWhenPassengersInReservationData);
  CPPUNIT_TEST(testAddPaxHasAttributeFreqFlyerTierLevel);

  CPPUNIT_TEST(testAddPiiDoesntBuildPIIWhenNoReservationData);
  CPPUNIT_TEST(testAddPiiDoesntBuildPIIWhenNoSegsInReservationData);
  CPPUNIT_TEST(testAddPiiBuildPIIWhenSegsExistInReservationData);

  CPPUNIT_TEST(testAddCfiDoesntBuildCFIWhenNoReservationData);
  CPPUNIT_TEST(testAddCfiDoesntBuildCFIWhenNoCorpFrequentFlyer);
  CPPUNIT_TEST(testAddCfiDoesntBuildCFIWhenFreqFlyerAccountNumberEmpty);
  CPPUNIT_TEST(testAddCfiBuildCFIelement);
  CPPUNIT_TEST(testAddCfiBuildCFIelementWithBlankCarrierWhenCarrierNotPresent);

  CPPUNIT_TEST(testAddOciDoesntBuildOCIWhenNoReservationData);
  CPPUNIT_TEST(testAddOciBuildOCIelement);

  CPPUNIT_TEST(testGetActionCode);
  CPPUNIT_TEST(testGetActionCodeEmpty);
  CPPUNIT_TEST(testGetActionCodeOverride);
  CPPUNIT_TEST(testGetActionCodeEmptyOverride);

  CPPUNIT_TEST(testAddBilling);
  CPPUNIT_TEST(testAddBillingAddBack);

  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenNoSegsInFareMarket);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenAnyOpenSegInFareMarket);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenArunkSegNotMultiAirport);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenArunkSegMultiAirportButLast);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueExchangeTrxAndFareMarketHasFlownSeg);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenInterlineSegsInFareMarket);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenAnySegWithFlightNumberZeroInFareMarket);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnFalseForFlowFareMarket);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenMoreThan3SegsInFareMarket);
  CPPUNIT_TEST(
      testStopOversArunkIncludedReturnTrueWhenFlightsInternationalAndConnectTimeMoreThan24hr);
  CPPUNIT_TEST(
      testStopOversArunkIncludedReturnFalseWhenFlightsInternationalAndConnectTimeEqual24hr);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnFalseWhenFlightsInternationalAndConnectLessThan24hr);
  CPPUNIT_TEST(
      testStopOversArunkIncludedReturnTrueWhenFlightsInternationalAAandConnectTimeMoreThan13hr);
  CPPUNIT_TEST(
      testStopOversArunkIncludedReturnFalseWhenFlightsInternationalAAandConnectTimeEqual13hr);
  CPPUNIT_TEST(
      testStopOversArunkIncludedReturnFalseWhenFlightsInternationalAAandConnectLessThan13hr);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenFlightsDomesticAndConnectTimeMoreThan4hr);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnFalseWhenFlightsDomesticAndConnectTimeEqual4hr);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnFalseWhenFlightsDomesticAndConnectTimeLessThan4hr);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenFlightsDomesticNWAndConnectTimeMoreThan24hr);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnFalseWhenFlightsDomesticNWAndConnectTimeEqual24hr);
  CPPUNIT_TEST(
      testStopOversArunkIncludedReturnFalseWhenFlightsDomesticNWAndConnectTimeLessThan24hr);

  CPPUNIT_TEST(testSendResDataReturnFalseWhenNoReservationDataInRequest);
  CPPUNIT_TEST(testSendResDataReturnTrueWhenNoJourneyCarrierInItin);
  CPPUNIT_TEST(testSendResDataReturnTrueWhenJourneyCarrierInItin);

  CPPUNIT_TEST(testBuildUniqueTvlSegsBuildsOnlyUniqueTvlSegs);
  CPPUNIT_TEST(testBuildUniqueFareMktsBuildsOnlyUniqueFareMarkets);

  CPPUNIT_TEST(testAddAsgShoppingBuildsASGandSEGelements);
  CPPUNIT_TEST(testAddAsgShoppingBuildsASGandSEGwithP0TforBestByCarrier);
  CPPUNIT_TEST(testAddAslShoppingBuildsASLelement);
  CPPUNIT_TEST(testAddAsoShoppingBuildsASOelement);

  CPPUNIT_TEST(testAddSgrBuildSGRForFirstFlight);
  CPPUNIT_TEST(testAddSgrBuildSGRForSecondFlightWith2DaysAfterFirstFlight);
  CPPUNIT_TEST(testAddSgrBuildSGRForSecondFlightWith2DaysBeforeFirstFlight);

  CPPUNIT_TEST(testAddSgrShoppingDoesntBuildSGRWhenSegNotFound);
  CPPUNIT_TEST(testAddSgrShoppingBuildSGRForFirstFlight);
  CPPUNIT_TEST(testAddSgrShoppingBuildSGRForSecondFlightWith2DaysAfterFirstFlight);
  CPPUNIT_TEST(testAddSgrShoppingBuildSGRForSecondFlightWith2DaysBeforeFirstFlight);

  CPPUNIT_TEST(testDaysDiffReturnZeroWhenDatesDifferInTimeOnly);
  CPPUNIT_TEST(testDaysDiffReturnOneWhenDayChanged);
  CPPUNIT_TEST(testDaysDiffReturnOneWhenMonthChanged);
  CPPUNIT_TEST(testDaysDiffReturnOneWhenYearChanged);

  CPPUNIT_TEST(testIndexTvlSegReturnZeroWhenSegNotFound);
  CPPUNIT_TEST(testIndexTvlSegReturnIndexNumberWhenSegMatch);

  CPPUNIT_TEST(testPartOfFlowJourneyReturnFalseWhenJourneyNotActivated);
  CPPUNIT_TEST(testPartOfFlowJourneyReturnFalseWhenJourneyNotApplied);
  CPPUNIT_TEST(testPartOfFlowJourneyReturnFalseWhenFareMarketFlow);
  CPPUNIT_TEST(testPartOfFlowJourneyReturnFalseWhenFirstSegNotFlowJourneyCarrier);
  CPPUNIT_TEST(testPartOfFlowJourneyReturnFalseWhenFareMarketAirSegNotFoundInUniqueFareMarkets);
  CPPUNIT_TEST(testPartOfFlowJourneyReturnTrueWhenFareMarketAirSegFoundInUniqueFareMarkets);

  CPPUNIT_TEST(testPartOfFlowJourneyShoppingReturnFalseWhenJourneyNotActivated);
  CPPUNIT_TEST(testPartOfFlowJourneyShoppingReturnFalseWhenJourneyNotApplied);
  CPPUNIT_TEST(testPartOfFlowJourneyShoppingReturnFalseWhenFareMarketFlow);
  CPPUNIT_TEST(testPartOfFlowJourneyShoppingReturnFalseWhenFirstSegNotFlowJourneyCarrier);
  CPPUNIT_TEST(
      testPartOfFlowJourneyShoppingReturnFalseWhenFareMarketAirSegNotFoundInUniqueFareMarkets);
  CPPUNIT_TEST(testPartOfFlowJourneyShoppingReturnTrueWhenFareMarketAirSegFoundInUniqueFareMarkets);

  CPPUNIT_TEST(testCalculateIATACheckDigitReturn9WhenIATANUmber9999999);
  CPPUNIT_TEST(testCalculateIATACheckDigitReturnRemainderOfDivideBy7);

  CPPUNIT_TEST(testDuplicateFareMarketReturnFalseWhenFareMarketSentToAtaeEmpty);
  CPPUNIT_TEST(testDuplicateFareMarketReturnTrueWhenFareMarketSentToAtaeHasSameFareMarketPointer);
  CPPUNIT_TEST(
      testDuplicateFareMarketReturnTrueWhenFareMarketSentToAtaeHasDifferentFareMarketPointerButSameFlights);
  CPPUNIT_TEST(
      testDuplicateFareMarketReturnFalseWhenFareMarketSentToAtaeHasDifferentFareMarketPointerAndDiffFlights);

  CPPUNIT_TEST(testSameFlightReturnFalseWhenFlightNumberNotEqual);
  CPPUNIT_TEST(testSameFlightReturnFalseWhenCarrierNotEqual);
  CPPUNIT_TEST(testSameFlightReturnFalseWhenOrigAirportNotEqual);
  CPPUNIT_TEST(testSameFlightReturnFalseWhenDestAirportNotEqual);
  CPPUNIT_TEST(testSameFlightReturnFalseWhenOperatingCarrierNotEqual);
  CPPUNIT_TEST(testSameFlightReturnFalseWhenOperatingFlightNumberNotEqual);
  CPPUNIT_TEST(testSameFlightReturnFalseWhenDepartTimeNotEqual);
  CPPUNIT_TEST(testSameFlightReturnFalseWhenArrivalTimeNotEqual);
  CPPUNIT_TEST(testSameFlightReturnTrueWhenAllEqual);

  CPPUNIT_TEST(testJourneyCOIncludedReturnFalseWhenFareMarketDoesNotHave3Segs);
  CPPUNIT_TEST(testJourneyCOIncludedReturnFalseWhenFareMarketIsFlow);
  CPPUNIT_TEST(testJourneyCOIncludedReturnFalseWhenJourneyNotActivated);
  CPPUNIT_TEST(testJourneyCOIncludedReturnFalseWhenJourneyNotApplied);
  CPPUNIT_TEST(testJourneyCOIncludedReturnTrueWhenFirst2SegsOfFareMarketMakesJourney);
  CPPUNIT_TEST(testJourneyCOIncludedReturnTrueWhenLast2SegsOfFareMarketMakesJourney);
  CPPUNIT_TEST(testJourneyCOIncludedReturnFalseNo2SegsOfFareMarketMakesJourney);

  CPPUNIT_TEST(testEntryFromAirlinePartitionReturnFalseWhenEntryFromAgent);
  CPPUNIT_TEST(testEntryFromAirlinePartitionReturnTrueWhenEntryFromAirline);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();

    Billing* billing = _memHandle.create<Billing>();
    PricingOptions* options = _memHandle.create<PricingOptions>();
    options->journeyActivatedForPricing() = true;
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = &_customer;
    pRequest->ticketingAgent() = agent;
    _airSeg = _memHandle.create<AirSeg>();
    populateAirSeg(*_airSeg);
    _itin = _memHandle.create<Itin>();
    _fm = _memHandle.create<FareMarket>();
    _fm->travelSeg().push_back(_airSeg);
    _itin->travelSeg().push_back(_airSeg);
    _itin->fareMarket().push_back(_fm);
    _trx->itin().push_back(_itin);
    _trx->setRequest(pRequest);
    _trx->billing() = billing;
    _trx->setOptions(options);
    _freqFlyerAccount.carrier() = "AA";
    _freqFlyerAccount.accountNumber() = "TESTACCOUNT";
    _resData.corpFreqFlyerAccounts().push_back(&_freqFlyerAccount);
    _trx->getRequest()->reservationData() = &_resData;
  }

  void tearDown() { _memHandle.clear(); }

  void testBuild()
  {
    AtaeRequest ataeRequest(*_trx, false);
    std::string request;
    CPPUNIT_ASSERT_NO_THROW(ataeRequest.build(request));
  }

  void testBuildShopping()
  {
    AtaeRequest ataeRequest(*_trx, false);
    std::string request;
    CPPUNIT_ASSERT_NO_THROW(ataeRequest.buildShopping(request));
  }

  void testAddResBuildRESelement()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addRes(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("RES"));
  }

  void testAddXrayTag()
  {
    _trx->assignXrayJsonMessage(xray::JsonMessagePtr(new xray::JsonMessage("mid", "cid")));
    _trx->getXrayJsonMessage()->setId("id");
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addXray(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("XRA"));
    CPPUNIT_ASSERT(elementBuiltInXML("CID=\"cid.id\""));
    CPPUNIT_ASSERT(elementBuiltInXML("MID"));
  }

  void testAddRliDoesntBuildRLIWhenNoReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData() = 0;
    ataeRequest.addRli(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("RLI"));
  }

  void testAddRliDoesntBuildRLIWhenRecordLocatorsInReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addRli(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("RLI"));
  }

  void testAddRliBuildRLIWhenRecordLocatorsExistInReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    RecordLocatorInfo rli;
    rli.originatingCxr() = "BA";
    rli.recordLocator() = "ABCDEF";
    _trx->getRequest()->reservationData()->recordLocators() += &rli;
    ataeRequest.addRli(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("RLI"));
  }

  void testAddDiaDoesntBuildDIAWhenNotDiagnostic195()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->diagnostic().diagnosticType() = DiagnosticNone;
    _trx->diagnostic().activate();
    ataeRequest.addDia(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("DIA"));
  }

  void testAddDiaDoesntBuildDIAWhenDiagnostic195ButNoDDparameter()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->diagnostic().diagnosticType() = Diagnostic195;
    _trx->diagnostic().activate();
    ataeRequest.addDia(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("DIA"));
  }

  void testAddDiaDoesntBuildDIAWhenDiagnostic195DDparameterWithoutNumber()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->diagnostic().diagnosticType() = Diagnostic195;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    ataeRequest.addDia(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("DIA"));
  }

  void testAddDiaBuildDIAWhenDiagnostic195DDparameterWithNumber()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->diagnostic().diagnosticType() = Diagnostic195;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "213"));
    ataeRequest.addDia(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("DIA"));
  }

  void testAddPaxDoesntBuildPAXWhenNoReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData() = 0;
    ataeRequest.addPax(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("PAX"));
  }

  void testAddPaxDoesntBuildPAXWhenNoPassengersInReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addPax(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("PAX"));
  }

  void testAddPaxDoesntBuildPAXWhenPassengersInReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Traveler traveler;
    populateTraveler(traveler);
    FrequentFlyerAccount freq;
    populateFreqFlyer(freq);
    traveler.freqFlyerAccount() = &freq;
    _trx->getRequest()->reservationData()->passengers() += &traveler;
    ataeRequest.addPax(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("PAX"));
  }

  void testAddPaxHasAttributeFreqFlyerTierLevel()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Traveler traveler;
    populateTraveler(traveler);
    FrequentFlyerAccount freq;
    populateFreqFlyer(freq);
    traveler.freqFlyerAccount() = &freq;
    _trx->getRequest()->reservationData()->passengers() += &traveler;
    ataeRequest.addPax(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("FTL"));
  }

  void testAddPiiDoesntBuildPIIWhenNoReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData() = 0;
    ataeRequest.addPii(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("PII"));
  }

  void testAddPiiDoesntBuildPIIWhenNoSegsInReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addPii(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("PII"));
  }

  void testAddPiiBuildPIIWhenSegsExistInReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ReservationSeg resSeg;
    populateReservationSeg(resSeg);
    _trx->getRequest()->reservationData()->reservationSegs() += &resSeg;
    ataeRequest.addPii(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("PII"));
  }

  void testAddCfiDoesntBuildCFIWhenNoReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData() = 0;
    ataeRequest.addCfi(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("CFI"));
  }

  void testAddCfiDoesntBuildCFIWhenNoCorpFrequentFlyer()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData()->corpFreqFlyerAccounts().clear();
    ataeRequest.addCfi(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("CFI"));
  }

  void testAddCfiDoesntBuildCFIWhenFreqFlyerAccountNumberEmpty()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData()->corpFreqFlyerAccounts()[0]->accountNumber().clear();
    ataeRequest.addCfi(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("CFI"));
  }

  void testAddCfiBuildCFIelement()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addCfi(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("CFI"));
    CPPUNIT_ASSERT(elementBuiltInXML("B00=\"AA\""));
    CPPUNIT_ASSERT(elementBuiltInXML("S1A=\"TESTACCOUNT\""));
  }

  void testAddCfiBuildCFIelementWithBlankCarrierWhenCarrierNotPresent()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData()->corpFreqFlyerAccounts()[0]->carrier().clear();
    ataeRequest.addCfi(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("CFI"));
    CPPUNIT_ASSERT(elementBuiltInXML("B00=\"   \""));
    CPPUNIT_ASSERT(elementBuiltInXML("S1A=\"TESTACCOUNT\""));
  }

  void testAddOciDoesntBuildOCIWhenNoReservationData()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData() = 0;
    ataeRequest.addOci(_construct);
    CPPUNIT_ASSERT(!elementBuiltInXML("OCI"));
  }

  void testAddOciBuildOCIelement()
  {
    AtaeRequest ataeRequest(*_trx, false);
    populateReservationData();
    ataeRequest.addOci(_construct);
    CPPUNIT_ASSERT(elementBuiltInXML("OCI"));
    CPPUNIT_ASSERT(elementBuiltInXML("N0Z=\"X\""));
    CPPUNIT_ASSERT(elementBuiltInXML("S1D=\"123\""));
    CPPUNIT_ASSERT(elementBuiltInXML("A20=\"W0H3\""));
    CPPUNIT_ASSERT(elementBuiltInXML("A10=\"DFW\""));
    CPPUNIT_ASSERT(elementBuiltInXML("A40=\"US\""));
    CPPUNIT_ASSERT(elementBuiltInXML("AR0=\"1S\""));
  }

  void testGetActionCode()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->billing()->actionCode() = "WP";

    const std::string expected = "WP";
    const std::string& actionCode = ataeRequest.getActionCode(*_trx->billing());

    CPPUNIT_ASSERT_EQUAL(expected, actionCode);
  }

  void testGetActionCodeEmpty()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->billing()->actionCode() = "";

    const std::string expected = AtaeRequest::DEFAULT_ACTION_CODE;
    const std::string& actionCode = ataeRequest.getActionCode(*_trx->billing());

    CPPUNIT_ASSERT_EQUAL(expected, actionCode);
  }

  void testGetActionCodeOverride()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->billing()->actionCode() = "WP";

    ataeRequest.setOverrideActionCode("WPNC");

    const std::string expected = "WPNC";
    const std::string& actionCode = ataeRequest.getActionCode(*_trx->billing());

    CPPUNIT_ASSERT_EQUAL(expected, actionCode);
  }

  void testGetActionCodeEmptyOverride()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->billing()->actionCode() = "";

    ataeRequest.setOverrideActionCode("WPNC");

    const std::string expected = "WPNC";
    const std::string& actionCode = ataeRequest.getActionCode(*_trx->billing());

    CPPUNIT_ASSERT_EQUAL(expected, actionCode);
  }

  void testAddBilling()
  {
    AtaeRequest ataeRequest(*_trx, false);
    populateBilling();

    ataeRequest.addBilling(_construct);

    CPPUNIT_ASSERT(elementBuiltInXML("BIL"));
    CPPUNIT_ASSERT(elementBuiltInXML("A20=\"W0H3\""));
    CPPUNIT_ASSERT(elementBuiltInXML("A22=\"DFW\""));
    CPPUNIT_ASSERT(elementBuiltInXML("AA0=\"VJI\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q03=\"925\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q02=\"3470\""));
    CPPUNIT_ASSERT(elementBuiltInXML("AE0=\"AA\""));
    CPPUNIT_ASSERT(elementBuiltInXML("AD0=\"7784E6\""));
    CPPUNIT_ASSERT(elementBuiltInXML("A70=\"WP\""));
    CPPUNIT_ASSERT(elementBuiltInXML("C21=\"SISXXCAS\""));
    CPPUNIT_ASSERT(elementBuiltInXML("C00=\"9223372036854775807\""));
    CPPUNIT_ASSERT(elementBuiltInXML("C01=\"1112402\""));
  }

  void testAddBillingAddBack()
  {
    AtaeRequest ataeRequest(*_trx, false);
    populateBilling();

    ataeRequest.setOverrideActionCode("WPNC");
    ataeRequest.addBilling(_construct);

    CPPUNIT_ASSERT(elementBuiltInXML("BIL"));
    CPPUNIT_ASSERT(elementBuiltInXML("A20=\"W0H3\""));
    CPPUNIT_ASSERT(elementBuiltInXML("A22=\"DFW\""));
    CPPUNIT_ASSERT(elementBuiltInXML("AA0=\"VJI\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q03=\"925\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q02=\"3470\""));
    CPPUNIT_ASSERT(elementBuiltInXML("AE0=\"AA\""));
    CPPUNIT_ASSERT(elementBuiltInXML("AD0=\"7784E6\""));
    CPPUNIT_ASSERT(elementBuiltInXML("A70=\"WPNC\""));
    CPPUNIT_ASSERT(elementBuiltInXML("C21=\"SISXXCAS\""));
    CPPUNIT_ASSERT(elementBuiltInXML("C00=\"9223372036854775807\""));
    CPPUNIT_ASSERT(elementBuiltInXML("C01=\"1112402\""));
  }

  void testStopOversArunkIncludedReturnTrueWhenNoSegsInFareMarket()
  {
    AtaeRequest ataeRequest(*_trx, false);
    FareMarket fm;
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(fm));
  }

  void testStopOversArunkIncludedReturnTrueWhenAnyOpenSegInFareMarket()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->travelSeg()[0]->segmentType() = Open;
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[0])));
  }

  void testStopOversArunkIncludedReturnTrueWhenArunkSegNotMultiAirport()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    populateAirSegs(itin->travelSeg());
    ArunkSeg arunkSeg;
    arunkSeg.boardMultiCity() = LOC_KUL;
    arunkSeg.offMultiCity() = LOC_NYC;
    itin->fareMarket()[1]->travelSeg().push_back(&arunkSeg);
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnTrueWhenArunkSegMultiAirportButLast()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    populateAirSegs(itin->travelSeg());
    ArunkSeg arunkSeg;
    arunkSeg.boardMultiCity() = LOC_EWR;
    arunkSeg.offMultiCity() = LOC_NYC;
    itin->fareMarket()[1]->travelSeg().push_back(&arunkSeg);
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnTrueExchangeTrxAndFareMarketHasFlownSeg()
  {
    ExchangePricingTrx excTrx;
    AtaeRequest ataeRequest(excTrx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->travelSeg()[0]->unflown() = false;
    excTrx.itin().push_back(itin);
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[0])));
  }

  void testStopOversArunkIncludedReturnTrueWhenInterlineSegsInFareMarket()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    populateAirSegs(itin->travelSeg());
    AirSeg* airSeg = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    airSeg->carrier() = "AA";
    itin->fareMarket()[1]->setFlowMarket(false);
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnTrueWhenAnySegWithFlightNumberZeroInFareMarket()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    populateAirSegs(itin->travelSeg());
    AirSeg* airSeg = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    airSeg->flightNumber() = 0;
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseForFlowFareMarket()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[0]->setFlowMarket(true);
    populateAirSegs(itin->travelSeg());
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[0])));
  }

  void testStopOversArunkIncludedReturnTrueWhenMoreThan3SegsInFareMarket()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    FareMarket fm;
    fm.travelSeg() = itin->fareMarket()[0]->travelSeg();
    fm.travelSeg() += itin->fareMarket()[0]->travelSeg()[0];
    fm.setFlowMarket(true);
    populateAirSegs(fm.travelSeg());
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(fm));
  }

  void testStopOversArunkIncludedReturnTrueWhenFlightsInternationalAndConnectTimeMoreThan24hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 11, 8, 16, 0);
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseWhenFlightsInternationalAndConnectTimeEqual24hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 11, 8, 15, 0);
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseWhenFlightsInternationalAndConnectLessThan24hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 11, 8, 14, 0);
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnTrueWhenFlightsInternationalAAandConnectTimeMoreThan13hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    dynamic_cast<AirSeg*>(itin->travelSeg()[0])->carrier() = SPECIAL_CARRIER_AA;
    dynamic_cast<AirSeg*>(itin->travelSeg()[1])->carrier() = SPECIAL_CARRIER_AA;
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 10, 21, 16, 0);
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseWhenFlightsInternationalAAandConnectTimeEqual13hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    dynamic_cast<AirSeg*>(itin->travelSeg()[0])->carrier() = SPECIAL_CARRIER_AA;
    dynamic_cast<AirSeg*>(itin->travelSeg()[1])->carrier() = SPECIAL_CARRIER_AA;
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 10, 21, 15, 0);
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseWhenFlightsInternationalAAandConnectLessThan13hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    dynamic_cast<AirSeg*>(itin->travelSeg()[0])->carrier() = SPECIAL_CARRIER_AA;
    dynamic_cast<AirSeg*>(itin->travelSeg()[1])->carrier() = SPECIAL_CARRIER_AA;
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 10, 21, 14, 0);
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnTrueWhenFlightsDomesticAndConnectTimeMoreThan4hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 10, 12, 16, 0);
    itin->travelSeg()[0]->geoTravelType() = GeoTravelType::Domestic;
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::Domestic;
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseWhenFlightsDomesticAndConnectTimeEqual4hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 10, 12, 15, 0);
    itin->travelSeg()[0]->geoTravelType() = GeoTravelType::Domestic;
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::Domestic;
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseWhenFlightsDomesticAndConnectTimeLessThan4hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 10, 12, 14, 0);
    itin->travelSeg()[0]->geoTravelType() = GeoTravelType::Domestic;
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::Domestic;
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnTrueWhenFlightsDomesticNWAndConnectTimeMoreThan24hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 11, 8, 16, 0);
    itin->travelSeg()[0]->geoTravelType() = GeoTravelType::Domestic;
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::Domestic;
    dynamic_cast<AirSeg*>(itin->travelSeg()[0])->carrier() = "NW";
    dynamic_cast<AirSeg*>(itin->travelSeg()[1])->carrier() = "NW";
    Loc loc;
    loc.state() = HAWAII;
    itin->travelSeg()[1]->origin() = &loc;
    CPPUNIT_ASSERT(ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseWhenFlightsDomesticNWAndConnectTimeEqual24hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 11, 8, 15, 0);
    itin->travelSeg()[0]->geoTravelType() = GeoTravelType::Domestic;
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::Domestic;
    dynamic_cast<AirSeg*>(itin->travelSeg()[0])->carrier() = "NW";
    dynamic_cast<AirSeg*>(itin->travelSeg()[1])->carrier() = "NW";
    Loc loc;
    loc.state() = HAWAII;
    itin->travelSeg()[1]->origin() = &loc;
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testStopOversArunkIncludedReturnFalseWhenFlightsDomesticNWAndConnectTimeLessThan24hr()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    populateAirSegs(itin->travelSeg());
    // DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0); First segment arrival date set in populate
    // function
    itin->travelSeg()[1]->departureDT() = DateTime(2009, 4, 11, 8, 14, 0);
    itin->travelSeg()[0]->geoTravelType() = GeoTravelType::Domestic;
    itin->travelSeg()[1]->geoTravelType() = GeoTravelType::Domestic;
    dynamic_cast<AirSeg*>(itin->travelSeg()[0])->carrier() = "NW";
    dynamic_cast<AirSeg*>(itin->travelSeg()[1])->carrier() = "NW";
    Loc loc;
    loc.state() = HAWAII;
    itin->travelSeg()[1]->origin() = &loc;
    CPPUNIT_ASSERT(!ataeRequest.stopOversArunkIncluded(*(itin->fareMarket()[1])));
  }

  void testSendResDataReturnFalseWhenNoReservationDataInRequest()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->reservationData() = 0;
    CPPUNIT_ASSERT(!ataeRequest.sendResData());
  }

  void testSendResDataReturnTrueWhenNoJourneyCarrierInItin()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _carrierPref.flowMktJourneyType() = NO; // not flow journey carrier
    CPPUNIT_ASSERT(ataeRequest.sendResData());
  }

  void testSendResDataReturnTrueWhenJourneyCarrierInItin()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _carrierPref.flowMktJourneyType() = YES; // flow journey carrier
    CPPUNIT_ASSERT(ataeRequest.sendResData());
  }

  void testBuildUniqueTvlSegsBuildsOnlyUniqueTvlSegs()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->callToAvailability() = 'T';
    std::vector<TravelSeg*> uniqueTvlSegs;
    _itin->travelSeg() += _airSeg, _airSeg, _airSeg;
    ataeRequest.buildUniqueTvlSegs(uniqueTvlSegs);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(_airSeg), uniqueTvlSegs[0]);
    size_t expected = 1;
    CPPUNIT_ASSERT_EQUAL(expected, uniqueTvlSegs.size());
  }

  void testBuildUniqueFareMktsBuildsOnlyUniqueFareMarkets()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->callToAvailability() = 'T';
    _itin->fareMarket() += _fm, _fm, _fm;
    std::vector<FareMarket*> uniqueFareMkts;
    ataeRequest.buildUniqueFareMkts(uniqueFareMkts);
    CPPUNIT_ASSERT_EQUAL(_fm, uniqueFareMkts[0]);
    size_t expected = 1;
    CPPUNIT_ASSERT_EQUAL(expected, uniqueFareMkts.size());
  }

  void testAddAsgShoppingBuildsASGandSEGelements()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addAsgShopping(_construct, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("ASG"));
    CPPUNIT_ASSERT(elementBuiltInXML("SEG"));
    CPPUNIT_ASSERT(elementBuiltInXML("P0T=\"F\""));
  }

  void testAddAsgShoppingBuildsASGandSEGwithP0TforBestByCarrier()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _airSeg->bbrCarrier() = true;
    ataeRequest.addAsgShopping(_construct, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("ASG"));
    CPPUNIT_ASSERT(elementBuiltInXML("SEG"));
    CPPUNIT_ASSERT(elementBuiltInXML("P0T=\"T\""));
  }

  void testAddAslShoppingBuildsASLelement()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addAslShopping(_construct, _itin->travelSeg(), _itin->fareMarket());
    CPPUNIT_ASSERT(elementBuiltInXML("ASL"));
    CPPUNIT_ASSERT_EQUAL(_fm, ataeRequest._fareMarketsSentToAtae[0]);
  }

  void testAddAsoShoppingBuildsASOelement()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addAsoShopping(*_fm, _construct, 1, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("ASO"));
    CPPUNIT_ASSERT(elementBuiltInXML("Q3A=\"1\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Z=\"1\""));
    CPPUNIT_ASSERT(elementBuiltInXML("D86="));
    CPPUNIT_ASSERT(elementBuiltInXML(_airSeg->departureDT().dateToString(YYYYMMDD, "")));
    CPPUNIT_ASSERT(elementBuiltInXML("Q3D=\"0\""));
    CPPUNIT_ASSERT(elementBuiltInXML("P50=\"F\""));
  }

  void testAddSgrBuildSGRForFirstFlight()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addSgrShopping(_airSeg, _construct, _airSeg, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("SGR"));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2X=\"1\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Y=\"0\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Z=\"1\""));
  }

  void testAddSgrBuildSGRForSecondFlightWith2DaysAfterFirstFlight()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg firstAirSeg;
    populateAirSeg(firstAirSeg);
    firstAirSeg.departureDT() = _airSeg->departureDT().subtractDays(2);
    ataeRequest.addSgrShopping(_airSeg, _construct, &firstAirSeg, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("SGR"));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2X=\"1\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Y=\"2\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Z=\"1\""));
  }

  void testAddSgrBuildSGRForSecondFlightWith2DaysBeforeFirstFlight()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg firstAirSeg;
    populateAirSeg(firstAirSeg);
    firstAirSeg.departureDT() = _airSeg->departureDT().addDays(2);
    ataeRequest.addSgrShopping(_airSeg, _construct, &firstAirSeg, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("SGR"));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2X=\"1\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Y=\"-2\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Z=\"1\""));
  }

  void testAddSgrShoppingDoesntBuildSGRWhenSegNotFound()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg;
    populateAirSeg(airSeg);
    airSeg.flightNumber() = 2193;
    AirSeg firstAirSeg;
    populateAirSeg(firstAirSeg);
    ataeRequest.addSgrShopping(&airSeg, _construct, &firstAirSeg, _itin->travelSeg());
    CPPUNIT_ASSERT(!elementBuiltInXML("SGR"));
  }

  void testAddSgrShoppingBuildSGRForFirstFlight()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest.addSgrShopping(_airSeg, _construct, _airSeg, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("SGR"));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2X=\"1\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Y=\"0\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Z=\"1\""));
  }

  void testAddSgrShoppingBuildSGRForSecondFlightWith2DaysAfterFirstFlight()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg firstAirSeg;
    populateAirSeg(firstAirSeg);
    firstAirSeg.departureDT() = _airSeg->departureDT().subtractDays(2);
    ataeRequest.addSgrShopping(_airSeg, _construct, &firstAirSeg, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("SGR"));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2X=\"1\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Y=\"2\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Z=\"1\""));
  }

  void testAddSgrShoppingBuildSGRForSecondFlightWith2DaysBeforeFirstFlight()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg firstAirSeg;
    populateAirSeg(firstAirSeg);
    firstAirSeg.departureDT() = _airSeg->departureDT().addDays(2);
    ataeRequest.addSgrShopping(_airSeg, _construct, &firstAirSeg, _itin->travelSeg());
    CPPUNIT_ASSERT(elementBuiltInXML("SGR"));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2X=\"1\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Y=\"-2\""));
    CPPUNIT_ASSERT(elementBuiltInXML("Q2Z=\"1\""));
  }

  void testDaysDiffReturnZeroWhenDatesDifferInTimeOnly()
  {
    AtaeRequest ataeRequest(*_trx, false);
    DateTime oneSecBeforeMidNight = DateTime(2009, 4, 9, 23, 59, 59);
    DateTime oneSecAfterMidNight = DateTime(2009, 4, 9, 0, 0, 1);
    CPPUNIT_ASSERT_EQUAL((int64_t)0,
                         ataeRequest.daysDiff(oneSecBeforeMidNight, oneSecAfterMidNight));
    CPPUNIT_ASSERT_EQUAL((int64_t)0,
                         ataeRequest.daysDiff(oneSecAfterMidNight, oneSecBeforeMidNight));
  }

  void testDaysDiffReturnOneWhenDayChanged()
  {
    AtaeRequest ataeRequest(*_trx, false);
    DateTime oneSecBeforeMidNight = DateTime(2009, 4, 10, 0, 0, 1);
    DateTime oneSecAfterMidNight = DateTime(2009, 4, 9, 23, 59, 59);
    CPPUNIT_ASSERT_EQUAL((int64_t)1,
                         ataeRequest.daysDiff(oneSecBeforeMidNight, oneSecAfterMidNight));
    CPPUNIT_ASSERT_EQUAL((int64_t)-1,
                         ataeRequest.daysDiff(oneSecAfterMidNight, oneSecBeforeMidNight));
  }

  void testDaysDiffReturnOneWhenMonthChanged()
  {
    AtaeRequest ataeRequest(*_trx, false);
    DateTime atMidNight = DateTime(2009, 3, 1, 0, 0, 0);
    DateTime oneSecAfterMidNight = DateTime(2009, 2, 28, 23, 59, 59);
    CPPUNIT_ASSERT_EQUAL((int64_t)1, ataeRequest.daysDiff(atMidNight, oneSecAfterMidNight));
    CPPUNIT_ASSERT_EQUAL((int64_t)-1, ataeRequest.daysDiff(oneSecAfterMidNight, atMidNight));
  }

  void testDaysDiffReturnOneWhenYearChanged()
  {
    AtaeRequest ataeRequest(*_trx, false);
    DateTime oneSecBeforeMidNight = DateTime(2009, 1, 1, 0, 0, 1);
    DateTime oneSecAfterMidNight = DateTime(2008, 12, 31, 23, 59, 59);
    CPPUNIT_ASSERT_EQUAL((int64_t)1,
                         ataeRequest.daysDiff(oneSecBeforeMidNight, oneSecAfterMidNight));
    CPPUNIT_ASSERT_EQUAL((int64_t)-1,
                         ataeRequest.daysDiff(oneSecAfterMidNight, oneSecBeforeMidNight));
  }

  void testIndexTvlSegReturnZeroWhenSegNotFound()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _airSeg->carrier() = "AA";
    uint16_t expected = 0;
    Itin* itin = prepareItinAndFareMarkets();
    CPPUNIT_ASSERT_EQUAL(expected, ataeRequest.indexTvlSeg(_airSeg, itin->travelSeg()));
  }

  void testIndexTvlSegReturnIndexNumberWhenSegMatch()
  {
    AtaeRequest ataeRequest(*_trx, false);
    uint16_t expected = 1;
    CPPUNIT_ASSERT_EQUAL(expected, ataeRequest.indexTvlSeg(_airSeg, _itin->travelSeg()));
  }

  void testPartOfFlowJourneyReturnFalseWhenJourneyNotActivated()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = false;
    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourney(*_fm));
  }

  void testPartOfFlowJourneyReturnFalseWhenJourneyNotApplied()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = false;
    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourney(*_fm));
  }

  void testPartOfFlowJourneyReturnFalseWhenFareMarketFlow()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    _fm->setFlowMarket(true);
    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourney(*_fm));
  }

  void testPartOfFlowJourneyReturnFalseWhenFirstSegNotFlowJourneyCarrier()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    _fm->setFlowMarket(false);
    _carrierPref.flowMktJourneyType() = NO; // not flow journey carrier
    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourney(*_fm));
  }

  void testPartOfFlowJourneyReturnFalseWhenFareMarketAirSegNotFoundInUniqueFareMarkets()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    _fm->setFlowMarket(false);
    _carrierPref.flowMktJourneyType() = YES; // flow journey carrier

    Itin* itin = prepareItinAndFareMarkets();
    Itin* saveItin = _trx->itin()[0];
    _trx->itin()[0] = itin;

    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourney(*_fm));

    _trx->itin()[0] = saveItin;
  }

  void testPartOfFlowJourneyReturnTrueWhenFareMarketAirSegFoundInUniqueFareMarkets()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    _carrierPref.flowMktJourneyType() = YES; // flow journey carrier

    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[0]->setFlowMarket(true);
    itin->fareMarket()[1]->setFlowMarket(false);
    Itin* saveItin = _trx->itin()[0];
    _trx->itin()[0] = itin;
    OAndDMarket odFm;
    odFm.validAfterRebook() = true;
    itin->segmentOAndDMarket()[itin->fareMarket()[1]->travelSeg().front()] = &odFm;

    CPPUNIT_ASSERT(ataeRequest.partOfFlowJourney(*(itin->fareMarket()[1])));
    _trx->itin()[0] = saveItin;
  }

  void testPartOfFlowJourneyShoppingReturnFalseWhenJourneyNotActivated()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForShopping() = false;
    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourneyShopping(*_fm, _itin->fareMarket()));
  }

  void testPartOfFlowJourneyShoppingReturnFalseWhenJourneyNotApplied()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForShopping() = true;
    _trx->getOptions()->applyJourneyLogic() = false;
    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourneyShopping(*_fm, _itin->fareMarket()));
  }

  void testPartOfFlowJourneyShoppingReturnFalseWhenFareMarketFlow()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForShopping() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    _fm->setFlowMarket(true);
    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourneyShopping(*_fm, _itin->fareMarket()));
  }

  void testPartOfFlowJourneyShoppingReturnFalseWhenFirstSegNotFlowJourneyCarrier()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForShopping() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    _fm->setFlowMarket(false);
    _carrierPref.flowMktJourneyType() = NO; // not flow journey carrier
    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourneyShopping(*_fm, _itin->fareMarket()));
  }

  void testPartOfFlowJourneyShoppingReturnFalseWhenFareMarketAirSegNotFoundInUniqueFareMarkets()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForShopping() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    _fm->setFlowMarket(false);
    _carrierPref.flowMktJourneyType() = YES; // flow journey carrier

    Itin* itin = prepareItinAndFareMarkets();

    CPPUNIT_ASSERT(!ataeRequest.partOfFlowJourneyShopping(*_fm, itin->fareMarket()));
  }

  void testPartOfFlowJourneyShoppingReturnTrueWhenFareMarketAirSegFoundInUniqueFareMarkets()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForShopping() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    _carrierPref.flowMktJourneyType() = YES; // flow journey carrier

    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[0]->setFlowMarket(true);
    itin->fareMarket()[1]->setFlowMarket(false);

    CPPUNIT_ASSERT(
        ataeRequest.partOfFlowJourneyShopping(*(itin->fareMarket()[1]), itin->fareMarket()));
  }

  void testCalculateIATACheckDigitReturn9WhenIATANUmber9999999()
  {
    AtaeRequest ataeRequest(*_trx, false);
    std::string iata = "9999999";
    std::string expected = "9";
    CPPUNIT_ASSERT_EQUAL(expected, ataeRequest.calculateIATACheckDigit(iata.c_str()));
  }

  void testCalculateIATACheckDigitReturnRemainderOfDivideBy7()
  {
    AtaeRequest ataeRequest(*_trx, false);
    std::string iata = "21";
    std::string expected = "0"; // remainder of 21/7=0
    CPPUNIT_ASSERT_EQUAL(expected, ataeRequest.calculateIATACheckDigit(iata.c_str()));
  }

  void testDuplicateFareMarketReturnFalseWhenFareMarketSentToAtaeEmpty()
  {
    AtaeRequest ataeRequest(*_trx, false);
    CPPUNIT_ASSERT(!ataeRequest.duplicateFareMarket(_fm));
  }

  void testDuplicateFareMarketReturnTrueWhenFareMarketSentToAtaeHasSameFareMarketPointer()
  {
    AtaeRequest ataeRequest(*_trx, false);
    ataeRequest._fareMarketsSentToAtae += _fm;
    CPPUNIT_ASSERT(ataeRequest.duplicateFareMarket(_fm));
  }

  void
  testDuplicateFareMarketReturnTrueWhenFareMarketSentToAtaeHasDifferentFareMarketPointerButSameFlights()
  {
    AtaeRequest ataeRequest(*_trx, false);
    FareMarket fm;
    fm.travelSeg() = _fm->travelSeg();
    ataeRequest._fareMarketsSentToAtae += &fm;
    CPPUNIT_ASSERT(ataeRequest.duplicateFareMarket(_fm));
  }

  void
  testDuplicateFareMarketReturnFalseWhenFareMarketSentToAtaeHasDifferentFareMarketPointerAndDiffFlights()
  {
    AtaeRequest ataeRequest(*_trx, false);
    FareMarket fm;
    AirSeg airSeg;
    fm.travelSeg() += &airSeg;
    ataeRequest._fareMarketsSentToAtae += &fm;
    CPPUNIT_ASSERT(!ataeRequest.duplicateFareMarket(_fm));
  }

  void testSameFlightReturnFalseWhenFlightNumberNotEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    airSeg2.flightNumber() = 2193;
    CPPUNIT_ASSERT(!ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testSameFlightReturnFalseWhenCarrierNotEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    airSeg2.carrier() = "AA";
    CPPUNIT_ASSERT(!ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testSameFlightReturnFalseWhenOrigAirportNotEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    airSeg2.origAirport() = "DEL";
    CPPUNIT_ASSERT(!ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testSameFlightReturnFalseWhenDestAirportNotEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    airSeg2.destAirport() = "BOM";
    CPPUNIT_ASSERT(!ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testSameFlightReturnFalseWhenOperatingCarrierNotEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    airSeg2.setOperatingCarrierCode("AA");
    CPPUNIT_ASSERT(!ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testSameFlightReturnFalseWhenOperatingFlightNumberNotEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    airSeg2.operatingFlightNumber() = 2193;
    CPPUNIT_ASSERT(!ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testSameFlightReturnFalseWhenDepartTimeNotEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    DateTime depart = DateTime(2009, 4, 9, 9, 15, 0);
    airSeg2.departureDT() = depart;
    CPPUNIT_ASSERT(!ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testSameFlightReturnFalseWhenArrivalTimeNotEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    DateTime arrive = DateTime(2009, 4, 10, 9, 15, 0);
    airSeg2.arrivalDT() = arrive;
    CPPUNIT_ASSERT(!ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testSameFlightReturnTrueWhenAllEqual()
  {
    AtaeRequest ataeRequest(*_trx, false);
    AirSeg airSeg2;
    populateAirSeg(airSeg2);
    CPPUNIT_ASSERT(ataeRequest.sameFlight(_airSeg, &airSeg2));
  }

  void testJourneyCOIncludedReturnFalseWhenFareMarketDoesNotHave3Segs()
  {
    AtaeRequest ataeRequest(*_trx, false);
    CPPUNIT_ASSERT(!ataeRequest.journeyCOIncluded(*_itin, *_fm));
  }

  void testJourneyCOIncludedReturnFalseWhenFareMarketIsFlow()
  {
    AtaeRequest ataeRequest(*_trx, false);
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[0]->setFlowMarket(true);

    CPPUNIT_ASSERT(!ataeRequest.journeyCOIncluded(*itin, *(itin->fareMarket()[0])));
  }

  void testJourneyCOIncludedReturnFalseWhenJourneyNotActivated()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = false;
    Itin* itin = prepareItinAndFareMarkets();

    CPPUNIT_ASSERT(!ataeRequest.journeyCOIncluded(*itin, *(itin->fareMarket()[0])));
  }

  void testJourneyCOIncludedReturnFalseWhenJourneyNotApplied()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = false;
    Itin* itin = prepareItinAndFareMarkets();

    CPPUNIT_ASSERT(!ataeRequest.journeyCOIncluded(*itin, *(itin->fareMarket()[0])));
  }

  void testJourneyCOIncludedReturnTrueWhenFirst2SegsOfFareMarketMakesJourney()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    Itin* itin = prepareItinAndFareMarkets();

    CPPUNIT_ASSERT(ataeRequest.journeyCOIncluded(*itin, *(itin->fareMarket()[0])));
  }

  void testJourneyCOIncludedReturnTrueWhenLast2SegsOfFareMarketMakesJourney()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);

    CPPUNIT_ASSERT(ataeRequest.journeyCOIncluded(*itin, *(itin->fareMarket()[0])));
  }

  void testJourneyCOIncludedReturnFalseNo2SegsOfFareMarketMakesJourney()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getOptions()->journeyActivatedForPricing() = true;
    _trx->getOptions()->applyJourneyLogic() = true;
    Itin* itin = prepareItinAndFareMarkets();
    itin->fareMarket()[1]->setFlowMarket(false);
    itin->fareMarket()[2]->setFlowMarket(false);

    CPPUNIT_ASSERT(!ataeRequest.journeyCOIncluded(*itin, *(itin->fareMarket()[0])));
  }

  void testEntryFromAirlinePartitionReturnFalseWhenEntryFromAgent()
  {
    AtaeRequest ataeRequest(*_trx, false);
    CPPUNIT_ASSERT(!ataeRequest.entryFromAirlinePartition());
  }

  void testEntryFromAirlinePartitionReturnTrueWhenEntryFromAirline()
  {
    AtaeRequest ataeRequest(*_trx, false);
    _trx->getRequest()->ticketingAgent() = 0;
    populateBilling();
    CPPUNIT_ASSERT(ataeRequest.entryFromAirlinePartition());
  }

private:
  PricingTrx* _trx;
  AirSeg* _airSeg;
  Itin* _itin;
  FareMarket* _fm;
  XMLConstruct _construct;
  ReservationData _resData;
  FrequentFlyerAccount _freqFlyerAccount;
  Customer _customer;
  CarrierPreference _carrierPref;

  bool elementBuiltInXML(const std::string& element)
  {
    if (_construct.getXMLData().find(element, 0) == std::string::npos)
      return false;
    return true;
  }

  Itin* prepareItinAndFareMarkets()
  {
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    AirSeg* a3 = _memHandle.create<AirSeg>();
    a1->carrier() = a2->carrier() = a3->carrier() = "CO";
    // a1->flightNumber() = a2->flightNumber() = a3->flightNumber() = 111;
    a1->carrierPref() = a2->carrierPref() = a3->carrierPref() = &_carrierPref;

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg() += a1, a2, a3;

    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->travelSeg() += a1, a2, a3;

    FareMarket* fm2 = _memHandle.create<FareMarket>();
    fm2->travelSeg() += a1, a2;
    fm2->setFlowMarket(true);

    FareMarket* fm3 = _memHandle.create<FareMarket>();
    fm3->travelSeg() += a2, a3;
    fm3->setFlowMarket(true);

    itin->fareMarket() += fm1, fm2, fm3;
    return itin;
  }

  void populateAirSegs(std::vector<TravelSeg*>& tvlSegs)
  {
    std::vector<TravelSeg*>::iterator tvlI = tvlSegs.begin();
    std::vector<TravelSeg*>::iterator tvlE = tvlSegs.end();
    AirSeg* airSeg = 0;
    for (; tvlI != tvlE; ++tvlI)
    {
      airSeg = dynamic_cast<AirSeg*>(*tvlI);
      populateAirSeg(*airSeg);
    }
  }

  void populateAirSeg(AirSeg& airSeg)
  {
    airSeg.pnrSegment() = 1;
    airSeg.origAirport() = "DFW";
    airSeg.destAirport() = "LGW";
    airSeg.carrier() = "BA";
    airSeg.setOperatingCarrierCode("BA");
    airSeg.flightNumber() = 2192;
    airSeg.operatingFlightNumber() = 2192;
    DateTime depart = DateTime(2009, 4, 9, 8, 15, 0);
    airSeg.departureDT() = depart;
    DateTime arrive = DateTime(2009, 4, 10, 8, 15, 0);
    airSeg.arrivalDT() = arrive;
    airSeg.carrierPref() = &_carrierPref;
    airSeg.setBookingCode("Y");
  }

  void populateBilling()
  {
    _trx->billing() = TestBillingFactory::create("/vobs/atseintl/test/testdata/data/Billing.xml");
  }

  void populateReservationData()
  {
    _resData.agent() = "W0H3";
    _resData.agentInd() = 'X';
    _resData.agentIATA() = "123";
    _resData.agentPCC() = "W0H3";
    _resData.agentCity() = "DFW";
    _resData.agentNation() = "US";
    _resData.agentCRS() = "1S";
  }

  void populateReservationSeg(ReservationSeg& reservSeg)
  {
    reservSeg.carrier() = "BA";
    reservSeg.flightNumber() = 2192;
    reservSeg.bookingCode() = "Y";
    reservSeg.pssDepartureDate() = "2009-04-10";
    reservSeg.pssDepartureTime() = "200";
    reservSeg.origAirport() = "DFW";
    reservSeg.pssArrivalDate() = "2009-04-10";
    reservSeg.pssArrivalTime() = "400";
    reservSeg.destAirport() = "TUL";
    reservSeg.actionCode() = "HK";
    reservSeg.numInParty() = 1;
    reservSeg.marriedSegCtrlId() = 'X';
    reservSeg.marriedGrpNo() = 1;
    reservSeg.marriedSeqNo() = 2;
    reservSeg.pollingInd() = 'X';
    reservSeg.eticket() = 'E';
  }

  void populateTraveler(Traveler& traveler)
  {
    traveler.lastNameQualifier() = "FOURTH";
    traveler.lastName() = "JOHN";
    traveler.referenceNumber() = 123;
    traveler.firstNameQualifier() = "MR";
    traveler.travelWithInfant() = true;
    traveler.firstName() = "JOHN";
    traveler.otherName() = "JOHNY";
  }

  void populateFreqFlyer(FrequentFlyerAccount& freqFlyerAcct)
  {
    freqFlyerAcct.vipType() = 'X';
    freqFlyerAcct.carrier() = "BA";
    freqFlyerAcct.accountNumber() = "123";
    freqFlyerAcct.partner().push_back("ABC");
    freqFlyerAcct.tierLevel() = "XYZ";
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AtaeRequestTest);

} // tse
