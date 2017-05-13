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

#include "Common/Config/ConfigMan.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CustomerActivationUtil.h"
#include "Common/Global.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Diversity.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Fare.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RequestResponse/InputGeoPathMapping.h"
#include "DataModel/RequestResponse/InputYqYrPath.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/BankerSellRate.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "Taxes/AtpcoTaxes/Common/SafeEnumToString.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder2.h"
#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "gmock/gmock.h"

using namespace tse;

CPPUNIT_NS_BEGIN

template <class U>
struct assertion_traits<std::vector<U> >
{
  typedef std::vector<U> T;

  static bool equal(const T& x, const T& y) { return x == y; }

  static std::string toString(const T& x)
  {
    OStringStream ost;

    ost << "[";
    for (size_t i = 0; i < x.size(); ++i)
    {
      if (i)
        ost << ", ";
      ost << assertion_traits<U>::toString(x[i]);
    }
    ost << "]";
    return ost.str();
  }
};

CPPUNIT_NS_END

namespace tax
{
class TaxRequestBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxRequestBuilderTest);
  CPPUNIT_TEST(testPricingRequest);
  CPPUNIT_TEST(testShoppingRequest);
  CPPUNIT_TEST(testFareDisplayRequest);

  CPPUNIT_TEST(testBuildYqYrMappings);
  CPPUNIT_TEST(testCreateYqYr);
  CPPUNIT_TEST(testProcessYqYrTaxItem_notYqYr);
  CPPUNIT_TEST(testProcessYqYrTaxItem);
  CPPUNIT_TEST(mappingPhaseToTutPhase0);
  CPPUNIT_TEST(mappingPhaseToTutPhase1);
  CPPUNIT_TEST(mappingPhaseToTutPhase2);
  CPPUNIT_TEST(mappingPhaseToTutPhase3);
  CPPUNIT_TEST(arunkCounting);

  CPPUNIT_TEST_SUITE_END();

  static bool containsGroup(const std::vector<InputApplyOn>& vec, int group)
  {
    for (const InputApplyOn& applyOn : vec)
    {
      if (applyOn._group == group)
        return true;
    }

    return false;
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void mappingPhaseToTutPhase0()
  {
    InputProcessingOptions options;
    const std::vector<InputApplyOn>& ans = options.getApplicableGroups();

    const AtpcoTaxesActivationStatus status;

    tse::PricingTrx trx;
    TaxRequestBuilder builder(trx, status);
    builder.processApplicableGroups(options);
    CPPUNIT_ASSERT(ans.empty());
  }

  void mappingPhaseToTutPhase1()
  {
    InputProcessingOptions options;
    const std::vector<InputApplyOn>& ans = options.getApplicableGroups();

    AtpcoTaxesActivationStatus status;
    status.setTaxOnOC(true);
    status.setTaxOnBaggage(true);

    tse::PricingTrx trx;
    TaxRequestBuilder builder(trx, status);
    builder.processApplicableGroups(options);
    CPPUNIT_ASSERT_EQUAL(size_t(2), ans.size());
    CPPUNIT_ASSERT(containsGroup(ans, 0));
    CPPUNIT_ASSERT(containsGroup(ans, 4));
  }

  void mappingPhaseToTutPhase2()
  {
    InputProcessingOptions options;
    const std::vector<InputApplyOn>& ans = options.getApplicableGroups();

    AtpcoTaxesActivationStatus status;
    status.setTaxOnChangeFee(true);

    tse::PricingTrx trx;
    TaxRequestBuilder builder(trx, status);
    builder.processApplicableGroups(options);
    CPPUNIT_ASSERT_EQUAL(size_t(2), ans.size());
    CPPUNIT_ASSERT(containsGroup(ans, 1));
    CPPUNIT_ASSERT(containsGroup(ans, 2));
  }

  void mappingPhaseToTutPhase3()
  {
    InputProcessingOptions options;
    const std::vector<InputApplyOn>& ans = options.getApplicableGroups();

    AtpcoTaxesActivationStatus status;
    status.setTaxOnItinYqYrTaxOnTax(true);

    tse::PricingTrx trx;
    TaxRequestBuilder builder(trx, status);
    builder.processApplicableGroups(options);
    CPPUNIT_ASSERT_EQUAL(size_t(1), ans.size());
    CPPUNIT_ASSERT(containsGroup(ans, 3));
  }

  void testFareDisplayRequest()
  {
    // prepare data
    tse::Billing billing;
    tse::PricingTrx trx;
    tse::Itin itin;
    tse::PricingUnit pricingUnit;
    tse::FareUsage fareUsage;
    tse::FarePath farePath;
    tse::FareInfo fareInfo, baseFareInfo;
    tse::Fare fare, baseFare;
    tse::Agent ticketingAgent;
    tse::PricingRequest pricingRequest;
    tse::PricingOptions pricingOptions;
    tse::AirSeg airSeg1, airSeg2;
    tse::Loc hidden1, hidden2;
    tse::Loc loc1, loc2, loc3, loc4;
    tse::PaxTypeFare paxTypeFare, basePaxTypeFare;
    tse::FareByRuleApp fareByRuleApp;
    tse::FBRPaxTypeFareRuleData ruleData;
    tse::FareByRuleItemInfo ruleInfo;
    tse::PaxTypeFare::PaxTypeFareAllRuleData allRuleData;
    tse::PaxType farePaxType;
    tse::PaxType fareUsagePaxType;

    trx.billing() = &billing;
    trx.setRequest(&pricingRequest);
    trx.getRequest()->ticketingAgent() = &ticketingAgent;
    trx.setOptions(&pricingOptions);
    trx.itin().push_back(&itin);
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg()[0]->origin() = &loc1;
    itin.travelSeg()[0]->destination() = &loc2;
    itin.travelSeg()[1]->origin() = &loc3;
    itin.travelSeg()[1]->destination() = &loc4;
    itin.farePath().push_back(&farePath);
    farePath.itin() = &itin;
    farePath.paxType() = &farePaxType;
    farePath.pricingUnit().push_back(&pricingUnit);
    pricingUnit.fareUsage().push_back(&fareUsage);
    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.travelSeg().push_back(&airSeg1);
    fareUsage.travelSeg().push_back(&airSeg2);
    ruleData.fbrApp() = &fareByRuleApp;
    ruleData.ruleItemInfo() = &ruleInfo;
    ruleData.baseFare() = &basePaxTypeFare;
    allRuleData.fareRuleData = &ruleData;
    paxTypeFare.setFare(&fare);
    paxTypeFare.segmentStatus().resize(2);
    paxTypeFare.actualPaxType() = &fareUsagePaxType;
    farePaxType.paxType() = "ADT";
    fareUsagePaxType.paxType() = "INF";
    fare.setFareInfo(&fareInfo);
    basePaxTypeFare.setFare(&baseFare);
    baseFare.setFareInfo(&baseFareInfo);

    billing.userPseudoCityCode() = "KRK";
    pricingOptions.currencyOverride() = "PLN";
    pricingRequest.ticketPointOverride() = "KTW";
    ticketingAgent.currencyCodeAgent() = "USD";
    trx.ticketingDate() = tse::DateTime(2014, 9, 19, 14, 16, 0);
    airSeg1.equipmentType() = "DH4";
    airSeg1.setOperatingCarrierCode("LA");
    airSeg1.setMarketingCarrierCode("LX");
    airSeg1.marketingFlightNumber() = 1111;
    airSeg1.departureDT() = tse::DateTime(2014, 9, 20, 14, 30, 0);
    airSeg1.arrivalDT() = tse::DateTime(2014, 9, 20, 15, 45, 0);
    airSeg2.equipmentType() = "767";
    airSeg2.setOperatingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("LX");
    airSeg2.marketingFlightNumber() = 2222;
    airSeg2.departureDT() = tse::DateTime(2014, 9, 21, 8, 30, 0);
    airSeg2.arrivalDT() = tse::DateTime(2014, 9, 22, 6, 00, 0);
    airSeg2.hiddenStops().push_back(&hidden1);
    airSeg2.hiddenStops().push_back(&hidden2);
    loc1.loc() = "KRK";
    loc1.city() = "KRK";
    loc1.nation() = "PL";
    loc2.loc() = "WAW";
    loc2.city() = "WAW";
    loc2.nation() = "PL";
    loc3.loc() = "WAW";
    loc3.city() = "WAW";
    loc3.nation() = "PL";
    loc4.loc() = "NRT";
    loc4.city() = "TYO";
    loc4.nation() = "JP";
    hidden1.loc() = "AAA";
    hidden1.city() = "AAA";
    hidden1.nation() = "RU";
    hidden2.loc() = "BBB";
    hidden2.city() = "BBB";
    hidden2.nation() = "CN";
    itin.validatingCarrier() = "LA";
    farePath.setTotalNUCAmount(100);
    farePath.baseFareCurrency() = "USD";
    farePath.calculationCurrency() = "USD";

    tse::Loc agentLocation;
    agentLocation.loc() = std::string("DFW");
    agentLocation.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &agentLocation;
    trx.getRequest()->ticketingAgent()->tvlAgencyPCC() = "A123";
    trx.getRequest()->ticketingAgent()->agentCity() = "DFW";
    trx.getRequest()->ticketingAgent()->vendorCrsCode() = "X";
    trx.getRequest()->ticketingAgent()->cxrCode() = "XX";

    fareByRuleApp.accountCode() = "DEF";
    paxTypeFare.status().set(tse::PaxTypeFare::PTF_FareByRule);
    (*paxTypeFare.paxTypeFareRuleDataMap())[25] = &allRuleData;
    fare.nucFareAmount() = 100;
    fareInfo.owrt() = 'O';
    fareInfo.directionality() = tse::BOTH;
    fareInfo.market1() = "KRK";
    fareInfo.market2() = "TYO";

    baseFare.nucFareAmount() = 120;
    baseFareInfo.owrt() = 'O';
    baseFareInfo.directionality() = tse::BOTH;
    baseFareInfo.market1() = "KRK";
    baseFareInfo.market2() = "TYO";

    // Passengers init
    tse::PaxType inputPaxType0;
    trx.paxType().push_back(&inputPaxType0);

    tse::PaxType outputPaxType;
    trx.itin()[0]->farePath()[0]->paxType() = &outputPaxType;

    // convert
    // trx.atpcoTaxesActivationStatus().setTaxOnItinYqYrTaxOnTax(true);
    const ServicesFeesMap servicesFees;
    tax::V2TrxMappingDetails mapping;
    TaxRequestBuilder builder(trx, trx.atpcoTaxesActivationStatus());
    tax::InputRequest* request = builder.buildInputRequest(servicesFees, mapping);
    // TODO Add exempt

    // TODO check
    CPPUNIT_ASSERT(request != 0);
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("DFW"), request->pointsOfSale()[0].loc());
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("KTW"), request->ticketingOptions().ticketingPoint());
    CPPUNIT_ASSERT_EQUAL(tax::type::CurrencyCode("PLN"), request->ticketingOptions().paymentCurrency());

    // Check fares
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), request->farePaths().size());
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount((int)(100 * MyDataHandle::RatePln2Usd)),
                         request->farePaths()[0]._totalAmount);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount((int)(120 * MyDataHandle::RatePln2Usd)),
                         request->farePaths()[0]._totalAmountBeforeDiscount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), request->farePaths()[0]._fareUsages.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(0),
                         request->farePaths()[0]._fareUsages[0]._fareRefId);
    CPPUNIT_ASSERT_EQUAL(std::string(""), request->fares()[0]._basis);
    CPPUNIT_ASSERT_EQUAL(std::string(""), request->fares()[0]._type);
    CPPUNIT_ASSERT_EQUAL(type::OneWayRoundTrip('O'), request->fares()[0]._oneWayRoundTrip);
    CPPUNIT_ASSERT_EQUAL(type::Directionality::Both, request->fares()[0]._directionality);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount((int)(100 * MyDataHandle::RatePln2Usd)),
                         request->fares()[0]._amount);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(0), request->fares()[0]._sellAmount);
    CPPUNIT_ASSERT_EQUAL(false, request->fares()[0]._isNetRemitAvailable);
    CPPUNIT_ASSERT_EQUAL(std::string(""), request->fares()[0]._rule);
    CPPUNIT_ASSERT_EQUAL(tax::type::FareTariff(0), request->fares()[0]._tariff);
    CPPUNIT_ASSERT(request->fares()[0]._tariffInd.empty());
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), std::get<3>(mapping._itinFarePathMapping[0]));
  }

  void testPricingRequest()
  {
    TestConfigInitializer::setValue("ATPCO_TAXES_ONCHANGEFEE_ENABLED", "Y", "TAX_SVC");

    // prepare data
    tse::RexPricingTrx trx;
    tse::Billing billing;
    tse::PricingRequest pricingRequest;
    tse::Agent ticketingAgent;
    tse::PricingOptions pricingOptions;
    tse::Itin itin;
    tse::AirSeg airSeg1, airSeg2;
    tse::ArunkSeg arunk;
    tse::Loc loc1, loc2, loc3, loc4, loc5, loc6;
    tse::Loc hidden1, hidden2;
    tse::FarePath farePath;
    tse::PricingUnit pricingUnit;
    tse::FareUsage fareUsage;
    tse::PaxTypeFare paxTypeFare;
    tse::Fare fare;
    tse::FareInfo fareInfo;
    tse::ActivationResult acResult;
    tse::PaxType farePaxType;
    tse::PaxType fareUsagePaxType;

    acResult.finalActvDate() = DateTime(2014, 8, 19); // activation date before ticketing date
    acResult.isActivationFlag() = true;
    trx.projCACMapData().insert(std::make_pair("TOE", &acResult));
    trx.billing() = &billing;
    trx.setRequest(&pricingRequest);
    trx.getRequest()->ticketingAgent() = &ticketingAgent;
    trx.setOptions(&pricingOptions);
    trx.itin().push_back(&itin);
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&arunk);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg()[0]->origin() = &loc1;
    itin.travelSeg()[0]->destination() = &loc2;
    itin.travelSeg()[0]->forcedStopOver() = 'T';
    itin.travelSeg()[1]->origin() = &loc3;
    itin.travelSeg()[1]->destination() = &loc4;
    itin.travelSeg()[2]->origin() = &loc5;
    itin.travelSeg()[2]->destination() = &loc6;
    itin.farePath().push_back(&farePath);
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pricingUnit);
    farePath.paxType() = &farePaxType;
    pricingUnit.fareUsage().push_back(&fareUsage);
    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.travelSeg().push_back(&airSeg1);
    fareUsage.travelSeg().push_back(&arunk);
    fareUsage.travelSeg().push_back(&airSeg2);
    paxTypeFare.setFare(&fare);
    paxTypeFare.segmentStatus().resize(3);
    paxTypeFare.actualPaxType() = &fareUsagePaxType;
    farePaxType.paxType() = "ADT";
    fareUsagePaxType.paxType() = "INF";
    fare.setFareInfo(&fareInfo);

    billing.userPseudoCityCode() = "KRK";
    pricingOptions.currencyOverride() = "PLN";
    pricingRequest.ticketPointOverride() = "KTW";
    ticketingAgent.currencyCodeAgent() = "USD";
    trx.ticketingDate() = tse::DateTime(2014, 9, 19, 14, 16, 0);
    airSeg1.equipmentType() = "DH4";
    airSeg1.setOperatingCarrierCode("LA");
    airSeg1.setMarketingCarrierCode("LX");
    airSeg1.marketingFlightNumber() = 1111;
    airSeg1.departureDT() = tse::DateTime(2014, 9, 20, 14, 30, 0);
    airSeg1.arrivalDT() = tse::DateTime(2014, 9, 20, 15, 45, 0);
    airSeg2.equipmentType() = "767";
    airSeg2.setOperatingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("LX");
    airSeg2.marketingFlightNumber() = 2222;
    airSeg2.departureDT() = tse::DateTime(2014, 9, 21, 8, 30, 0);
    airSeg2.arrivalDT() = tse::DateTime(2014, 9, 22, 6, 00, 0);
    airSeg2.hiddenStops().push_back(&hidden1);
    airSeg2.hiddenStops().push_back(&hidden2);
    loc1.loc() = "PRG";
    loc1.city() = "PRG";
    loc1.nation() = "CZ";
    loc2.loc() = "KRK";
    loc2.city() = "KRK";
    loc2.nation() = "PL";
    loc3.loc() = "KRK";
    loc3.city() = "KRK";
    loc3.nation() = "PL";
    loc4.loc() = "WAW";
    loc4.city() = "WAW";
    loc4.nation() = "PL";
    loc5.loc() = "WAW";
    loc5.city() = "WAW";
    loc5.nation() = "PL";
    loc6.loc() = "NRT";
    loc6.city() = "TYO";
    loc6.nation() = "JP";
    hidden1.loc() = "AAA";
    hidden1.city() = "AAA";
    hidden1.nation() = "RU";
    hidden2.loc() = "BBB";
    hidden2.city() = "BBB";
    hidden2.nation() = "CN";
    itin.validatingCarrier() = "LA";
    farePath.setTotalNUCAmount(100);
    farePath.baseFareCurrency() = "USD";
    farePath.calculationCurrency() = "USD";

    tse::Loc agentLocation;
    agentLocation.loc() = std::string("DFW");
    agentLocation.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &agentLocation;
    trx.getRequest()->ticketingAgent()->tvlAgencyPCC() = "A123";
    trx.getRequest()->ticketingAgent()->agentCity() = "DFW";
    trx.getRequest()->ticketingAgent()->vendorCrsCode() = "X";
    trx.getRequest()->ticketingAgent()->cxrCode() = "XX";

    fareInfo.owrt() = 'O';
    fareInfo.directionality() = tse::BOTH;
    fareInfo.market1() = "KRK";
    fareInfo.market2() = "TYO";
    fare.nucFareAmount() = 100;

    // Passengers init
    tse::PaxType inputPaxType0;
    trx.paxType().push_back(&inputPaxType0);

    tse::PaxType outputPaxType;
    trx.itin()[0]->farePath()[0]->paxType() = &outputPaxType;
    // trx.itin()[0]->farePath()[0]->paxType()->paxType() = "ABC";

    // ChangeFee
    const tse::MoneyAmount changeFee = 20;
    tse::ReissueCharges reissueCharges;
    reissueCharges.changeFee = changeFee;
    reissueCharges.changeFeeCurrency = "PLN";
    farePath.reissueCharges() = &reissueCharges;

    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);

    // exempts
    pricingRequest.exemptSpecificTaxes() = 'T';
    pricingRequest.taxIdExempted().push_back("AY");
    pricingRequest.taxIdExempted().push_back("US1");

    // convert
    trx.atpcoTaxesActivationStatus().setTaxOnOC(true);
    trx.atpcoTaxesActivationStatus().setTaxOnChangeFee(true);
    trx.atpcoTaxesActivationStatus().setTaxOnItinYqYrTaxOnTax(true);
    const ServicesFeesMap servicesFees;
    tax::V2TrxMappingDetails ignoreMapping;
    TaxRequestBuilder builder(trx, trx.atpcoTaxesActivationStatus());
    tax::InputRequest* request = builder.buildInputRequest(servicesFees, ignoreMapping);

    // check
    CPPUNIT_ASSERT(request != 0);
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("DFW"), request->pointsOfSale()[0].loc());
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("KTW"), request->ticketingOptions().ticketingPoint());
    CPPUNIT_ASSERT_EQUAL(tax::type::CurrencyCode("PLN"), request->ticketingOptions().paymentCurrency());
    CPPUNIT_ASSERT_EQUAL(boost::gregorian::date(2014, 9, 19),
                         request->ticketingOptions().ticketingDate());

    boost::ptr_vector<tax::InputGeo>& geos = request->geoPaths()[0]._geos;
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), geos.size());
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("KRK"), geos[1]._loc.code());
    CPPUNIT_ASSERT(geos[1]._unticketedTransfer == tax::type::UnticketedTransfer::No);
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("WAW"), geos[2]._loc.code());
    CPPUNIT_ASSERT(geos[2]._unticketedTransfer == tax::type::UnticketedTransfer::No);
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("BBB"), geos[6]._loc.code());
    CPPUNIT_ASSERT(geos[6]._unticketedTransfer == tax::type::UnticketedTransfer::Yes);
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("NRT"), geos[7]._loc.code());
    CPPUNIT_ASSERT(geos[7]._unticketedTransfer == tax::type::UnticketedTransfer::No);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), request->fares().size());
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount((int)(100 * MyDataHandle::RatePln2Usd)),
                         request->fares()[0]._amount);
    CPPUNIT_ASSERT_EQUAL(type::OneWayRoundTrip('O'), request->fares()[0]._oneWayRoundTrip);
    CPPUNIT_ASSERT_EQUAL(type::Directionality::Both, request->fares()[0]._directionality);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), request->flights().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0), request->flights()[0]._arrivalDateShift);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0), request->flights()[1]._arrivalDateShift);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0), request->flights()[2]._arrivalDateShift);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(1), request->flights()[3]._arrivalDateShift);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), request->itins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), request->flightPaths().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), request->itins()[0]._flightPathRefId);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), request->flightPaths()[0]._flightUsages.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0),
                         request->flightPaths()[0]._flightUsages[0]._connectionDateShift);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(1),
                         request->flightPaths()[0]._flightUsages[1]._connectionDateShift);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0),
                         request->flightPaths()[0]._flightUsages[2]._connectionDateShift);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0),
                         request->flightPaths()[0]._flightUsages[3]._connectionDateShift);

    CPPUNIT_ASSERT_EQUAL(type::ForcedConnection::Stopover,
                         request->flightPaths()[0]._flightUsages[0]._forcedConnection);
    CPPUNIT_ASSERT_EQUAL(type::ForcedConnection(type::ForcedConnection::Blank),
                         request->flightPaths()[0]._flightUsages[1]._forcedConnection);

    tax::InputFlight* flight =
        &request->flights()[request->flightPaths()[0]._flightUsages[3]._flightRefId];
    CPPUNIT_ASSERT(0 != flight);
    CPPUNIT_ASSERT_EQUAL(boost::posix_time::time_duration(8, 30, 0, 0), flight->_departureTime);
    CPPUNIT_ASSERT_EQUAL(boost::posix_time::time_duration(6, 00, 0, 0), flight->_arrivalTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(2222), flight->_marketingCarrierFlightNumber);
    CPPUNIT_ASSERT_EQUAL(type::CarrierCode("LX"), flight->_marketingCarrier);
    CPPUNIT_ASSERT_EQUAL(type::CarrierCode("LA"), flight->_operatingCarrier);
    CPPUNIT_ASSERT_EQUAL(std::string("767"), flight->_equipment);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(1), flight->_arrivalDateShift);

    CPPUNIT_ASSERT(request->itins()[0]._changeFeeRefId.has_value());
    CPPUNIT_ASSERT_EQUAL(type::Index(0), request->itins()[0]._changeFeeRefId.value());
    CPPUNIT_ASSERT_EQUAL(tax::doubleToAmount(changeFee), request->changeFees()[0]._amount);

    const boost::ptr_vector<InputCalculationRestriction>& calcRestrictions =
        request->processing().calculationRestrictions();
    CPPUNIT_ASSERT_EQUAL(type::Index(1), calcRestrictions.size());
    CPPUNIT_ASSERT(calcRestrictions[0].restrictionType() ==
                   type::CalcRestriction::ExemptSpecifiedTaxes);
    CPPUNIT_ASSERT_EQUAL(type::Index(2), calcRestrictions[0].calculationRestrictionTax().size());
    CPPUNIT_ASSERT_EQUAL(std::string("AY"),
                         calcRestrictions[0].calculationRestrictionTax()[0].sabreTaxCode());
    CPPUNIT_ASSERT_EQUAL(std::string("US1"),
                         calcRestrictions[0].calculationRestrictionTax()[1].sabreTaxCode());
  }

  void testShoppingRequest()
  {
    // prepare data
    TestConfigInitializer::setValue("ALT_DATE_DIVERSITY_FARE_LEVELS", 5, "SHOPPING_DIVERSITY");
    TestConfigInitializer::setValue(
        "ALT_DATE_DIVERSITY_OPTIONS_PER_CARRIER", 1, "SHOPPING_DIVERSITY");
    TestConfigInitializer::setValue("ALT_DATE_DIVERSITY_CUTOFF_COEF", 0.1, "SHOPPING_DIVERSITY");
    TestConfigInitializer::setValue(
        "ALT_DATE_DIVERSITY_DATEPAIR_CUTOFF_COEF", 0.9, "SHOPPING_DIVERSITY");

    tse::ShoppingTrx trx;
    tse::Billing billing;
    tse::PricingRequest pricingRequest;
    pricingRequest.setSettlementMethod("01");
    tse::Agent ticketingAgent;
    tse::PricingOptions pricingOptions;
    tse::Itin itin, itin2;
    tse::AirSeg airSeg1, airSeg2, airSeg3, airSeg4;
    tse::Loc loc1, loc2, loc3;
    tse::FarePath farePath, farePath2;
    tse::PricingUnit pricingUnit, pricingUnit2;
    tse::FareUsage fareUsage, fareUsage2;
    tse::PaxTypeFare paxTypeFare, paxTypeFare2;
    tse::Fare fare, fare2;
    tse::FareInfo fareInfo, fareInfo2;
    tse::PaxType farePaxType;
    tse::PaxType fareUsagePaxType;

    trx.billing() = &billing;
    trx.setRequest(&pricingRequest);
    trx.getRequest()->ticketingAgent() = &ticketingAgent;
    trx.setOptions(&pricingOptions);
    trx.itin().push_back(&itin);
    trx.itin().push_back(&itin2);
    // itin 1
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg()[0]->origin() = &loc1;
    itin.travelSeg()[0]->destination() = &loc2;
    itin.travelSeg()[1]->origin() = &loc2;
    itin.travelSeg()[1]->destination() = &loc3;
    itin.farePath().push_back(&farePath);
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pricingUnit);
    pricingUnit.fareUsage().push_back(&fareUsage);
    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.travelSeg().push_back(&airSeg1);
    fareUsage.travelSeg().push_back(&airSeg2);
    paxTypeFare.setFare(&fare);
    paxTypeFare.segmentStatus().resize(2);
    paxTypeFare.actualPaxType() = &fareUsagePaxType;
    farePaxType.paxType() = "ADT";
    fareUsagePaxType.paxType() = "INF";
    fare.setFareInfo(&fareInfo);
    // itin 2
    itin2.travelSeg().push_back(&airSeg3);
    itin2.travelSeg().push_back(&airSeg4);
    itin2.travelSeg()[0]->origin() = &loc3;
    itin2.travelSeg()[0]->destination() = &loc2;
    itin2.travelSeg()[1]->origin() = &loc2;
    itin2.travelSeg()[1]->destination() = &loc1;
    itin2.farePath().push_back(&farePath2);
    farePath2.itin() = &itin2;
    farePath2.pricingUnit().push_back(&pricingUnit2);
    pricingUnit2.fareUsage().push_back(&fareUsage2);
    fareUsage2.paxTypeFare() = &paxTypeFare2;
    fareUsage2.travelSeg().push_back(&airSeg3);
    fareUsage2.travelSeg().push_back(&airSeg4);
    paxTypeFare2.setFare(&fare2);
    paxTypeFare2.segmentStatus().resize(2);
    paxTypeFare2.actualPaxType() = &fareUsagePaxType;
    farePaxType.paxType() = "ADT";
    fareUsagePaxType.paxType() = "INF";
    fare2.setFareInfo(&fareInfo2);

    billing.userPseudoCityCode() = "KRK";
    pricingOptions.currencyOverride() = "PLN";
    pricingRequest.ticketPointOverride() = "KTW";
    ticketingAgent.currencyCodeAgent() = "USD";
    trx.ticketingDate() = tse::DateTime(2014, 9, 19, 14, 16, 0);
    loc1.loc() = "KRK";
    loc1.city() = "KRK";
    loc1.nation() = "PL";
    loc2.loc() = "WAW";
    loc2.city() = "WAW";
    loc2.nation() = "PL";
    loc3.loc() = "NRT";
    loc3.city() = "TYO";
    loc3.nation() = "JP";
    // itin 1
    airSeg1.equipmentType() = "DH4";
    airSeg1.setOperatingCarrierCode("LA");
    airSeg1.setMarketingCarrierCode("LX");
    airSeg1.marketingFlightNumber() = 1111;
    airSeg1.departureDT() = tse::DateTime(2014, 9, 20, 14, 30, 0);
    airSeg1.arrivalDT() = tse::DateTime(2014, 9, 20, 15, 45, 0);
    airSeg2.equipmentType() = "767";
    airSeg2.setOperatingCarrierCode("LA");
    airSeg2.setMarketingCarrierCode("LX");
    airSeg2.marketingFlightNumber() = 2222;
    airSeg2.departureDT() = tse::DateTime(2014, 9, 21, 8, 30, 0);
    airSeg2.arrivalDT() = tse::DateTime(2014, 9, 22, 6, 00, 0);
    itin.validatingCarrier() = "LA";
    farePath.setTotalNUCAmount(100);
    farePath.baseFareCurrency() = "PLN";
    farePath.calculationCurrency() = "PLN";

    tse::Loc agentLocation;
    agentLocation.loc() = std::string("DFW");
    agentLocation.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &agentLocation;
    trx.getRequest()->ticketingAgent()->tvlAgencyPCC() = "A123";
    trx.getRequest()->ticketingAgent()->agentCity() = "DFW";
    trx.getRequest()->ticketingAgent()->vendorCrsCode() = "X";
    trx.getRequest()->ticketingAgent()->cxrCode() = "XX";

    fareInfo.owrt() = 'O';
    fareInfo.directionality() = tse::BOTH;
    fareInfo.market1() = "KRK";
    fareInfo.market2() = "TYO";
    fare.nucFareAmount() = 100;
    // itin 2
    airSeg3.equipmentType() = "767";
    airSeg3.setOperatingCarrierCode("EY");
    airSeg3.setMarketingCarrierCode("EY");
    airSeg3.marketingFlightNumber() = 1234;
    airSeg3.departureDT() = tse::DateTime(2014, 9, 22, 22, 30, 0);
    airSeg3.arrivalDT() = tse::DateTime(2014, 9, 23, 8, 45, 0);
    airSeg4.equipmentType() = "DH4";
    airSeg4.setOperatingCarrierCode("EY");
    airSeg4.setMarketingCarrierCode("EX");
    airSeg4.marketingFlightNumber() = 5678;
    airSeg4.departureDT() = tse::DateTime(2014, 9, 24, 14, 30, 0);
    airSeg4.arrivalDT() = tse::DateTime(2014, 9, 24, 18, 00, 0);
    itin2.validatingCarrier() = "EY";
    farePath2.setTotalNUCAmount(200);
    farePath2.baseFareCurrency() = "PLN";
    farePath2.calculationCurrency() = "PLN";

    fareInfo2.owrt() = 'X';
    fareInfo2.directionality() = tse::BOTH;
    fareInfo2.market1() = "TYO";
    fareInfo2.market2() = "KRK";
    fare2.nucFareAmount() = 200;

    // Passengers init
    tse::PaxType inputPaxType;
    trx.paxType().push_back(&inputPaxType);

    tse::PaxType outputPaxType0;
    trx.itin()[0]->farePath()[0]->paxType() = &outputPaxType0;

    tse::PaxType outputPaxType1;
    trx.itin()[1]->farePath()[0]->paxType() = &outputPaxType1;
    // trx.itin()[0]->farePath()[0]->paxType()->paxType() = "ABC";

    // convert
    const ServicesFeesMap servicesFees;
    tax::V2TrxMappingDetails ignoreV2Mapping;
    TaxRequestBuilder builder(trx, trx.atpcoTaxesActivationStatus());
    tax::InputRequest* request = builder.buildInputRequest(servicesFees, ignoreV2Mapping);

    // check
    CPPUNIT_ASSERT(request != 0);
    CPPUNIT_ASSERT(!request->processing()._tch);
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("DFW"), request->pointsOfSale()[0].loc());
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("KTW"), request->ticketingOptions().ticketingPoint());
    CPPUNIT_ASSERT_EQUAL(tax::type::CurrencyCode("PLN"), request->ticketingOptions().paymentCurrency());
    CPPUNIT_ASSERT_EQUAL(boost::gregorian::date(2014, 9, 19),
                         request->ticketingOptions().ticketingDate());

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), request->itins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), request->fares().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), request->flights().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), request->geoPaths().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), request->flightPaths().size());

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), request->itins()[1]._flightPathRefId);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), request->flightPaths()[1]._flightUsages.size());

    // itin 1
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), request->geoPaths()[0]._geos.size());
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("NRT"), request->geoPaths()[0]._geos[3]._loc.code());

    CPPUNIT_ASSERT_EQUAL(static_cast<long unsigned>(0), request->itins()[0]._geoPathRefId);
    CPPUNIT_ASSERT_EQUAL(static_cast<long unsigned>(0), request->itins()[0]._farePathRefId);
    CPPUNIT_ASSERT(request->itins()[0]._farePathGeoPathMappingRefId.has_value());
    CPPUNIT_ASSERT_EQUAL(type::Index(0), request->itins()[0]._farePathGeoPathMappingRefId.value());

    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(100), request->fares()[0]._amount);
    CPPUNIT_ASSERT_EQUAL(type::OneWayRoundTrip('O'), request->fares()[0]._oneWayRoundTrip);
    CPPUNIT_ASSERT_EQUAL(type::Directionality::Both, request->fares()[0]._directionality);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), request->itins()[0]._flightPathRefId);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), request->flightPaths()[0]._flightUsages.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(1),
                         request->flightPaths()[0]._flightUsages[1]._connectionDateShift);

    tax::InputFlight* flight =
        &request->flights()[request->flightPaths()[0]._flightUsages[1]._flightRefId];
    CPPUNIT_ASSERT(0 != flight);
    CPPUNIT_ASSERT_EQUAL(boost::posix_time::time_duration(8, 30, 0, 0), flight->_departureTime);
    CPPUNIT_ASSERT_EQUAL(boost::posix_time::time_duration(6, 00, 0, 0), flight->_arrivalTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(2222), flight->_marketingCarrierFlightNumber);
    CPPUNIT_ASSERT_EQUAL(type::CarrierCode("LX"), flight->_marketingCarrier);
    CPPUNIT_ASSERT_EQUAL(type::CarrierCode("LA"), flight->_operatingCarrier);
    CPPUNIT_ASSERT_EQUAL(std::string("767"), flight->_equipment);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(1), flight->_arrivalDateShift);

    // itin 2
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), request->geoPaths()[1]._geos.size());
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("KRK"), request->geoPaths()[1]._geos[3]._loc.code());

    CPPUNIT_ASSERT_EQUAL(static_cast<long unsigned>(1), request->itins()[1]._geoPathRefId);
    CPPUNIT_ASSERT_EQUAL(static_cast<long unsigned>(1), request->itins()[1]._farePathRefId);
    CPPUNIT_ASSERT(request->itins()[1]._farePathGeoPathMappingRefId.has_value());
    CPPUNIT_ASSERT_EQUAL(type::Index(0), request->itins()[1]._farePathGeoPathMappingRefId.value());

    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(200), request->fares()[1]._amount);
    CPPUNIT_ASSERT_EQUAL(type::OneWayRoundTrip('X'), request->fares()[1]._oneWayRoundTrip);
    CPPUNIT_ASSERT_EQUAL(type::Directionality::Both, request->fares()[0]._directionality);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), request->itins()[1]._flightPathRefId);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), request->flightPaths()[1]._flightUsages.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(1),
                         request->flightPaths()[1]._flightUsages[1]._connectionDateShift);

    flight = &request->flights()[request->flightPaths()[1]._flightUsages[1]._flightRefId];
    CPPUNIT_ASSERT(0 != flight);
    CPPUNIT_ASSERT_EQUAL(boost::posix_time::time_duration(14, 30, 0, 0), flight->_departureTime);
    CPPUNIT_ASSERT_EQUAL(boost::posix_time::time_duration(18, 00, 0, 0), flight->_arrivalTime);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(5678), flight->_marketingCarrierFlightNumber);
    CPPUNIT_ASSERT_EQUAL(type::CarrierCode("EX"), flight->_marketingCarrier);
    CPPUNIT_ASSERT_EQUAL(type::CarrierCode("EY"), flight->_operatingCarrier);
    CPPUNIT_ASSERT_EQUAL(std::string("DH4"), flight->_equipment);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0), flight->_arrivalDateShift);
  }

  void testBuildYqYrMappings()
  {
    tse::TaxItem taxItem;
    taxItem.setTravelSegStartIndex(0);
    taxItem.setTravelSegEndIndex(1);
    std::vector<type::Index> arunksBeforeSeg(5, 0);
    arunksBeforeSeg[3] = 1;
    arunksBeforeSeg[4] = 1;
    std::vector<type::Index> hiddenBeforeSeg(5, 0);
    hiddenBeforeSeg[1] = 1;
    hiddenBeforeSeg[2] = 2;
    hiddenBeforeSeg[3] = 2;
    hiddenBeforeSeg[4] = 2;
    std::vector<TravelSeg*> travelSegs;
    AirSeg seg1;
    AirSeg seg2;
    ArunkSeg seg3;
    AirSeg seg4;
    AirSeg seg5;
    Loc hidden1;
    Loc hidden2;
    seg1.hiddenStops().push_back(&hidden1);
    seg2.hiddenStops().push_back(&hidden2);
    travelSegs.push_back(&seg1);
    travelSegs.push_back(&seg2);
    travelSegs.push_back(&seg3);
    travelSegs.push_back(&seg4);
    travelSegs.push_back(&seg5);

    InputGeoPathMapping geoPathMapping;

    PricingTrx trx;
    TaxRequestBuilder builder(trx, trx.atpcoTaxesActivationStatus());

    builder.buildYqYrMappings(taxItem, geoPathMapping, arunksBeforeSeg, hiddenBeforeSeg, travelSegs);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), geoPathMapping._mappings.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(8), geoPathMapping._mappings[0].maps().size());

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), geoPathMapping._mappings[0].maps()[0]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), geoPathMapping._mappings[0].maps()[1]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), geoPathMapping._mappings[0].maps()[2]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), geoPathMapping._mappings[0].maps()[3]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), geoPathMapping._mappings[0].maps()[4]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(5), geoPathMapping._mappings[0].maps()[5]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(6), geoPathMapping._mappings[0].maps()[6]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(7), geoPathMapping._mappings[0].maps()[7]._geoRefId);

    taxItem.setTravelSegStartIndex(3);
    taxItem.setTravelSegEndIndex(4);
    builder.buildYqYrMappings(taxItem, geoPathMapping, arunksBeforeSeg, hiddenBeforeSeg, travelSegs);

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), geoPathMapping._mappings.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), geoPathMapping._mappings[1].maps().size());

    CPPUNIT_ASSERT_EQUAL(std::size_t(8), geoPathMapping._mappings[1].maps()[0]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(9), geoPathMapping._mappings[1].maps()[1]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(10), geoPathMapping._mappings[1].maps()[2]._geoRefId);
    CPPUNIT_ASSERT_EQUAL(std::size_t(11), geoPathMapping._mappings[1].maps()[3]._geoRefId);
  }

  void testCreateYqYr()
  {
    tax::InputRequest inputRequest;

    tse::TaxItem taxItem;
    taxItem.taxAmount() = 10;
    taxItem.taxCode() = "YQI";
    taxItem.taxType() = 'I';
    taxItem.taxIncluded() = false;
    PricingTrx trx;
    TaxRequestBuilder builder(trx, trx.atpcoTaxesActivationStatus());
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0),
                         builder.addYqYr(taxItem, inputRequest.yqYrs()));

    taxItem.taxAmount() = 20;
    taxItem.taxCode() = "YRF";
    taxItem.taxType() = 'F';
    taxItem.taxIncluded() = true;
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(1),
                         builder.addYqYr(taxItem, inputRequest.yqYrs()));

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), inputRequest.yqYrs().size());

    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), inputRequest.yqYrs()[0]._id);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(10), inputRequest.yqYrs()[0]._amount);
    CPPUNIT_ASSERT_EQUAL(false, inputRequest.yqYrs()[0]._taxIncluded);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(1), inputRequest.yqYrs()[1]._id);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(20), inputRequest.yqYrs()[1]._amount);
    CPPUNIT_ASSERT_EQUAL(true, inputRequest.yqYrs()[1]._taxIncluded);
  }

  void testProcessYqYrTaxItem_notYqYr()
  {
    tse::TaxItem taxItem;
    taxItem.taxCode() = "XYZ";
    taxItem.setTravelSegStartIndex(0);
    taxItem.setTravelSegEndIndex(0);

    tax::InputRequest inputRequest;
    tax::InputGeoPathMapping geoPathMapping;
    tax::InputYqYrPath yqYrPath;
    std::vector<type::Index> arunksBeforeSeg(2, 0);
    std::vector<type::Index> hiddenBeforeSeg(2, 0);
    std::vector<TravelSeg*> travelSegs;
    AirSeg seg1;
    travelSegs.push_back(&seg1);

    PricingTrx trx;
    TaxRequestBuilder builder(trx, trx.atpcoTaxesActivationStatus());
    builder.processYqYrTaxItem(
        taxItem, inputRequest, geoPathMapping, yqYrPath, arunksBeforeSeg, hiddenBeforeSeg, travelSegs);

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), inputRequest.yqYrs().size());
  }

  void testProcessYqYrTaxItem()
  {
    tse::TaxItem taxItem;
    taxItem.taxAmount() = 11;
    taxItem.taxCode() = "YQF";
    taxItem.setTravelSegStartIndex(0);
    taxItem.setTravelSegEndIndex(0);

    tax::InputRequest inputRequest;
    tax::InputGeoPathMapping geoPathMapping;
    tax::InputYqYrPath yqYrPath;
    std::vector<type::Index> arunksBeforeSeg(2, 0);
    std::vector<type::Index> hiddenBeforeSeg(2, 0);
    std::vector<TravelSeg*> travelSegs;
    AirSeg seg1;
    travelSegs.push_back(&seg1);

    PricingTrx trx;
    TaxRequestBuilder builder(trx, trx.atpcoTaxesActivationStatus());
    builder.processYqYrTaxItem(
        taxItem, inputRequest, geoPathMapping, yqYrPath, arunksBeforeSeg, hiddenBeforeSeg, travelSegs);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), inputRequest.yqYrs().size());
    CPPUNIT_ASSERT_EQUAL(tax::doubleToAmount(taxItem.taxAmount()), yqYrPath._totalAmount);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), yqYrPath._yqYrUsages.size());
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), yqYrPath._yqYrUsages[0]._yqYrRefId);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), geoPathMapping._mappings.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), geoPathMapping._mappings[0].maps().size());
  }

  void arunkCounting()
  {
    tse::AirSeg air_;
    tse::ArunkSeg arunk_;
    tse::TravelSeg* air = &air_, *arunk = &arunk_;

    std::vector<tse::TravelSeg*> vec;
    std::vector<type::Index> ans;
    std::vector<type::Index> ref;

    PricingTrx trx;
    TaxRequestBuilder builder(trx, trx.atpcoTaxesActivationStatus());

    ans = builder.computeArunksBeforeSeg(vec);
    CPPUNIT_ASSERT_EQUAL(ref, ans);

    vec = {air};
    ref = {0};
    ans = builder.computeArunksBeforeSeg(vec);
    CPPUNIT_ASSERT_EQUAL(ref, ans);

    vec = {air, arunk};
    ref = {0, 0};
    ans = builder.computeArunksBeforeSeg(vec);
    CPPUNIT_ASSERT_EQUAL(ref, ans);

    vec = {air, air};
    ref = {0, 0};
    ans = builder.computeArunksBeforeSeg(vec);
    CPPUNIT_ASSERT_EQUAL(ref, ans);

    vec = {arunk, air, air};
    ref = {0, 1, 1};
    ans = builder.computeArunksBeforeSeg(vec);
    CPPUNIT_ASSERT_EQUAL(ref, ans);

    vec = {arunk, arunk};
    ref = {0, 1};
    ans = builder.computeArunksBeforeSeg(vec);
    CPPUNIT_ASSERT_EQUAL(ref, ans);

    vec = {air, arunk, air, arunk, air};
    ref = {0, 0, 1, 1, 2};
    ans = builder.computeArunksBeforeSeg(vec);
    CPPUNIT_ASSERT_EQUAL(ref, ans);

    vec = {air, arunk, air, arunk};
    ref = {0, 0, 1, 1};
    ans = builder.computeArunksBeforeSeg(vec);
    CPPUNIT_ASSERT_EQUAL(ref, ans);
  }

  class MyDataHandle : public DataHandleMock
  {
  public:
    static constexpr double RatePln2Usd = 3.5;

    TestMemHandle _memHandle;

    const std::vector<BankerSellRate*>&
    getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
    {
      std::vector<BankerSellRate*>* ret = _memHandle.create<std::vector<BankerSellRate*> >();
      BankerSellRate* bsr = _memHandle.create<BankerSellRate>();
      bsr->primeCur() = primeCur;
      bsr->cur() = cur;
      bsr->rateType() = 'B';
      bsr->agentSine() = "FXR";
      if (primeCur == "USD" && cur == "PLN")
      {
        bsr->rate() = RatePln2Usd;
        bsr->rateNodec() = 5;
      }
      else
      {
        bsr->rate() = 0.1;
        bsr->rateNodec() = 1;
      }

      ret->push_back(bsr);
      return *ret;
    }
  };

private:
  ::tse::TestMemHandle _memHandle;
  MyDataHandle* _mdh;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxRequestBuilderTest);

class GetItinFromTransaction : public testing::Test {
  tse::ExcItin excItin[2];
  tse::Itin itin[3];

public:
  tse::RexPricingTrx trx;

  void
  SetUp()
  {
    trx.exchangeItin().push_back(&excItin[0]);
    trx.exchangeItin().push_back(&excItin[1]);

    trx.itin().push_back(&itin[0]);
  }
};

TEST_F(GetItinFromTransaction, IsExchItinForRepriceExchItinPhase)
{
  trx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
  ItinSelector itinSelector(trx);
  ASSERT_TRUE(itinSelector.isExcItin());
}

TEST_F(GetItinFromTransaction, IsNewItinForRepriceNewItinPhase)
{
  trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  ItinSelector itinSelector(trx);
  ASSERT_TRUE(itinSelector.isNewItin());
}

TEST_F(GetItinFromTransaction, IsExcItinNumberEqualsToTwo)
{
  trx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
  ItinSelector itinSelector(trx);
  ASSERT_THAT(itinSelector.get().size(), testing::Eq(2));
}

TEST_F(GetItinFromTransaction, IsNewItinNumberEqualsToOne)
{
  trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  ItinSelector itinSelector(trx);
  ASSERT_THAT(itinSelector.get().size(), testing::Eq(1));
}

} // namespace tax
