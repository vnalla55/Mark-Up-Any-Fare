//--------------------------------------------------------------
//
//  File:        Diag198CollectorTest.cpp
//  Created:     July 2014
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Common/CabinType.h"
#include "Common/TseEnums.h"
#include "Diagnostic/Diag198Collector.h"
#include "DataModel/AmVatTaxRatesOnCharges.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RefundPricingRequest.h"
#include "DataModel/RefundPricingTrx.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "DataModel/Agent.h"
#include "DBAccess/Customer.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{
FALLBACKVALUE_DECL(azPlusUp);
FALLBACKVALUE_DECL(excDiscDiag23XImprovements);

class Diag198CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag198CollectorTest);
  CPPUNIT_TEST(testBrandedFaresTNShoppingRequestFalse);
  CPPUNIT_TEST(testBrandedFaresTNShoppingRequestTrue);
  CPPUNIT_TEST(testBrandedFaresTNShoppingRequestTrue_withBrandNumber);
  CPPUNIT_TEST(testBrandedFaresTNShoppingRequest1);
  CPPUNIT_TEST(testAddChargesTaxesHasNoData);
  CPPUNIT_TEST(testAddChargesTaxesHasData);
  CPPUNIT_TEST(testAddDiscountAmount);
  CPPUNIT_TEST(testAddDiscountAmountOld);
  CPPUNIT_TEST(testAddPlusUpAmount);
  CPPUNIT_TEST(testAddDiscountPercentage);
  CPPUNIT_TEST(testAddDiscountPercentageOld);
  CPPUNIT_TEST(testAddPlusUpPercentage);
  CPPUNIT_TEST(testaddOptionsRequestedCabin);
  CPPUNIT_TEST(testDisplayCabinInclusionCode_AB);
  CPPUNIT_TEST(testDisplayCabinInclusionCode_FB);
  CPPUNIT_TEST(testDisplayCabinInclusionCode_FBBB);
  CPPUNIT_TEST(testDisplayCabinInclusionCode_PBFBBB);
  CPPUNIT_TEST(testDisplayCabinInclusionCode_CTRW);
  CPPUNIT_TEST(testDisplayCabinInclusionCode_NLX);
  CPPUNIT_TEST(testAddExchangePlusUpInfoRex);
  CPPUNIT_TEST(testAddExchangePlusUpInfoRefund);
  CPPUNIT_TEST(testAddAncillaryPriceModifiersToItinInfo_Empty);
  CPPUNIT_TEST(testAddAncillaryPriceModifiersToItinInfo_SingleEntry);
  CPPUNIT_TEST(testAddAncillaryPriceModifiersToItinInfo_SingleEntry_OptionalDataNotSet);
  CPPUNIT_TEST(testAddAncillaryPriceModifiersToItinInfo_RepeatedAid);
  CPPUNIT_TEST(testAddAncillaryPriceModifiersToItinInfo_DoubleEntry);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _collector = new Diag198Collector;
    _collector->_active = true;
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _fdRequest = _memHandle.create<FareDisplayRequest>();
    Customer* customer = _memHandle.create<Customer>();
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = customer;
    _request->ticketingAgent() = agent;
    _trx->setRequest(_request);
    _trx->setOptions(_memHandle.create<PricingOptions>());
  }

  void tearDown() { delete _collector; _memHandle.clear();}

  void testBrandedFaresTNShoppingRequestFalse()
  {
    _collector->addBrandedFaresTNShoppingInd(*_trx);
    CPPUNIT_ASSERT_EQUAL(string("***** START BRANDS FOR TN SHOPPING OPTIONS *****\n"
                                "BRTNS REQUEST: F\n"
                                "BRALL REQUEST: F\n"
                                "NBR OF BRANDS REQUESTED: UNLIMITED\n"
                                "***** END BRANDS FOR TN SHOPPING OPTIONS *****\n"),
                         _collector->str());
  }

  void testBrandedFaresTNShoppingRequestTrue()
  {
    _trx->setTnShoppingBrandingMode(TnShoppingBrandingMode::MULTIPLE_BRANDS);
    _collector->addBrandedFaresTNShoppingInd(*_trx);
    CPPUNIT_ASSERT_EQUAL(string("***** START BRANDS FOR TN SHOPPING OPTIONS *****\n"
                                "BRTNS REQUEST: F\n"
                                "BRALL REQUEST: T\n"
                                "NBR OF BRANDS REQUESTED: UNLIMITED\n"
                                "***** END BRANDS FOR TN SHOPPING OPTIONS *****\n"),
                         _collector->str());
  }

  void testBrandedFaresTNShoppingRequestTrue_withBrandNumber()
  {
    _trx->setTnShoppingBrandingMode(TnShoppingBrandingMode::MULTIPLE_BRANDS);
    _trx->setNumberOfBrands(13);
    _collector->addBrandedFaresTNShoppingInd(*_trx);
    CPPUNIT_ASSERT_EQUAL(string("***** START BRANDS FOR TN SHOPPING OPTIONS *****\n"
                                "BRTNS REQUEST: F\n"
                                "BRALL REQUEST: T\n"
                                "NBR OF BRANDS REQUESTED: 13\n"
                                "***** END BRANDS FOR TN SHOPPING OPTIONS *****\n"),
                         _collector->str());
  }


  void testBrandedFaresTNShoppingRequest1()
  {
    _trx->setTnShoppingBrandingMode(TnShoppingBrandingMode::SINGLE_BRAND);
    _collector->addBrandedFaresTNShoppingInd(*_trx);
    _trx->setNumberOfBrands(0);
    CPPUNIT_ASSERT_EQUAL(string("***** START BRANDS FOR TN SHOPPING OPTIONS *****\n"
                                "BRTNS REQUEST: T\n"
                                "BRALL REQUEST: F\n"
                                "NBR OF BRANDS REQUESTED: UNLIMITED\n"
                                "***** END BRANDS FOR TN SHOPPING OPTIONS *****\n"),
                         _collector->str());
  }

  void testAddChargesTaxesHasNoData()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _collector->addChargesTaxes(*_trx);
    CPPUNIT_ASSERT( _collector->str().empty());
  }

  void testAddChargesTaxesHasData()
  {
    std::string amChargesTaxesData = "MX/IVA/16|BO/BOA/13";
    TestConfigInitializer::setValue("AM_CHARGES_TAXES", amChargesTaxesData, "SERVICE_FEES_SVC");
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->loadAmVatTaxRatesOnCharges();
    _collector->addChargesTaxes(*_trx);

    CPPUNIT_ASSERT_EQUAL(string("********** AM CHARGES TAXES CONFIGURATION DATA **********\n"
                                "POS  TAX CODE  TAX RATE\n"
                                "BO   BOA       13\n"
                                "MX   IVA       16\n"),
                         _collector->str());
    std::cout << _collector->str() << std::endl;
  }

  void testAddDiscountAmount()
  {
    _request->addDiscountAmountNew(2, 2, 1.5 , "USD");
    _request->addDiscountAmountNew(1, 1, 3.14, "USD");
    _collector->addDiscountPlusUpInfo(*_request, *_trx);
    CPPUNIT_ASSERT_EQUAL(string("DISCOUNT AMOUNT:\n"
                                "  SEGMENT: 1-1 AMOUNT: 3.14 USD\n"
                                "  SEGMENT: 2-2 AMOUNT: 1.5 USD\n"
                                "DISCOUNT PERCENTAGE:\n"
                                "MARK UP AMOUNT:\n"
                                "MARK UP PERCENTAGE:\n"),
                         _collector->str());
  }

  void testAddDiscountAmountOld()
  {
    fallback::value::azPlusUp.set(true);

    _request->addDiscAmount(2, 2, 1.5 , "USD");
    _request->addDiscAmount(1, 1, 3.14, "USD");
    _collector->addDiscountPlusUpInfo(*_request, *_trx);
    CPPUNIT_ASSERT_EQUAL(string("DISCOUNT AMOUNT:\n"
                                "  SEGMENT: 1-1 AMOUNT: 3.14 CURRENCY CODE: USD\n"
                                "  SEGMENT: 2-2 AMOUNT: 1.5 CURRENCY CODE: USD\n"
                                "DISCOUNT PERCENTAGE:\n"),
                         _collector->str());
  }

  void testAddPlusUpAmount()
  {
    _request->addDiscountAmountNew(2, 2, -1.5 , "USD");
    _request->addDiscountAmountNew(1, 1, -3.14, "USD");
    _collector->addDiscountPlusUpInfo(*_request, *_trx);
    CPPUNIT_ASSERT_EQUAL(string("DISCOUNT AMOUNT:\n"
                                "DISCOUNT PERCENTAGE:\n"
                                "MARK UP AMOUNT:\n"
                                "  SEGMENT: 1-1 AMOUNT: 3.14 USD\n"
                                "  SEGMENT: 2-2 AMOUNT: 1.5 USD\n"
                                "MARK UP PERCENTAGE:\n"),
                         _collector->str());
  }

  void testAddDiscountPercentage()
  {
    _request->addDiscountPercentage(2, 1.5);
    _request->addDiscountPercentage(1, 3.14);
    _collector->addDiscountPlusUpInfo(*_request, *_trx);
    CPPUNIT_ASSERT_EQUAL(string("DISCOUNT AMOUNT:\n"
                                "DISCOUNT PERCENTAGE:\n"
                                "  SEGMENT: 1 PERCENT: 3.14\n"
                                "  SEGMENT: 2 PERCENT: 1.5\n"
                                "MARK UP AMOUNT:\n"
                                "MARK UP PERCENTAGE:\n"),
                         _collector->str());
  }

  void testAddDiscountPercentageOld()
  {
    fallback::value::azPlusUp.set(true);

    _request->discPercentages().insert(std::pair<uint16_t, Percent>(2, 1.5));
    _request->discPercentages().insert(std::pair<uint16_t, Percent>(1, 3.14));
    _collector->addDiscountPlusUpInfo(*_request, *_trx);
    CPPUNIT_ASSERT_EQUAL(string("DISCOUNT AMOUNT:\n"
                                "DISCOUNT PERCENTAGE:\n"
                                "  SEGMENT: 1 PERCENT: 3.14\n"
                                "  SEGMENT: 2 PERCENT: 1.5\n"),
                         _collector->str());
  }

  void testAddPlusUpPercentage()
  {
    _request->addDiscountPercentage(2, -1.5);
    _request->addDiscountPercentage(1, -3.14);
    _collector->addDiscountPlusUpInfo(*_request, *_trx);
    CPPUNIT_ASSERT_EQUAL(string("DISCOUNT AMOUNT:\n"
                                "DISCOUNT PERCENTAGE:\n"
                                "MARK UP AMOUNT:\n"
                                "MARK UP PERCENTAGE:\n"
                                "  SEGMENT: 1 PERCENT: 3.14\n"
                                "  SEGMENT: 2 PERCENT: 1.5\n"),
                         _collector->str());
  }

  void testaddOptionsRequestedCabin()
  {
    _trx->getOptions()->cabin().setBusinessClass();
    _collector->addOptions(*_trx);
    CPPUNIT_ASSERT(_collector->str().find("CABIN REQUESTED : BB - BUSINESS") != std::string::npos);
  }

  void testDisplayCabinInclusionCode_AB()
  {
    _fdRequest->requestedInclusionCode() = "PBFBJBBBSBYB";
    _collector->displayCabinInclusionCode(*_fdRequest);
    CPPUNIT_ASSERT(_collector->str().find("CABIN INCLUSION CODE REQUESTED : AB") != std::string::npos);
  }

  void testDisplayCabinInclusionCode_FB()
  {
    _fdRequest->requestedInclusionCode() = "FB";
    _collector->displayCabinInclusionCode(*_fdRequest);
    CPPUNIT_ASSERT(_collector->str().find("CABIN INCLUSION CODE REQUESTED : FB") != std::string::npos);
  }

  void testDisplayCabinInclusionCode_FBBB()
  {
    _fdRequest->requestedInclusionCode() = "FBBB";
    _collector->displayCabinInclusionCode(*_fdRequest);
    CPPUNIT_ASSERT(_collector->str().find("CABIN INCLUSION CODES REQUESTED : FB BB ") != std::string::npos);
  }

  void testDisplayCabinInclusionCode_PBFBBB()
  {
    _fdRequest->requestedInclusionCode() = "PBFBBB";
    _collector->displayCabinInclusionCode(*_fdRequest);
    CPPUNIT_ASSERT(_collector->str().find("CABIN INCLUSION CODES REQUESTED : PB FB BB ") != std::string::npos);
  }

  void testDisplayCabinInclusionCode_CTRW()
  {
    _fdRequest->requestedInclusionCode() = "CTRW";
    _collector->displayCabinInclusionCode(*_fdRequest);
    CPPUNIT_ASSERT(_collector->str().find("CABIN INCLUSION CODES REQUESTED : ") == std::string::npos);
  }

  void testDisplayCabinInclusionCode_NLX()
  {
    _fdRequest->requestedInclusionCode() = "NLX";
    _collector->displayCabinInclusionCode(*_fdRequest);
    CPPUNIT_ASSERT(_collector->str().find("CABIN INCLUSION CODES REQUESTED : ") == std::string::npos);
  }

  void testAddExchangePlusUpInfoRex()
  {
    ExcItin excItin;

    RexPricingOptions options;
    RexPricingRequest request;
    RexPricingTrx trx;
    trx.setOptions(&options);
    trx.setRequest(&request);
    trx.exchangeItin().push_back(&excItin);

    trx.setRexPrimaryProcessType('A');
    trx.previousExchangeDT() = DateTime("2016-04-01 13:13:13.000");
    options.excTotalFareAmt() = "12.99 USD";

    _collector->addExchangePlusUpInfo(trx);
    CPPUNIT_ASSERT_EQUAL(string("****************** START EXCHANGE PLUS UP *****************\n"
                                "PROCESS TYPE INDICATOR FOR PRIMARY REQUEST TYPE N25: true\n"
                                "PREVIOUS EXCHANGE DATE D95: 2016-Apr-01 13:13:13\n"
                                "BASE FARE AMOUNT C5A: 12.99 USD\n"
                                "DISCOUNT AMOUNT:\n"
                                "DISCOUNT PERCENTAGE:\n"
                                "MARK UP AMOUNT:\n"
                                "MARK UP PERCENTAGE:\n"
                                "******************* END EXCHANGE PLUS UP ******************\n"),
                         _collector->str());
  }

  void testAddExchangePlusUpInfoRefund()
  {
    ExcItin excItin;

    RexPricingOptions options;
    RefundPricingRequest request;
    RefundPricingTrx trx;
    trx.setOptions(&options);
    trx.setRequest(&request);
    trx.exchangeItin().push_back(&excItin);

    trx.setRexPrimaryProcessType('A');
    trx.previousExchangeDT() = DateTime("2016-04-01 13:13:13.000");
    options.excTotalFareAmt() = "12.99 USD";

    _collector->addExchangePlusUpInfo(trx);
    CPPUNIT_ASSERT_EQUAL(string("****************** START EXCHANGE PLUS UP *****************\n"
                                "PROCESS TYPE INDICATOR FOR PRIMARY REQUEST TYPE N25: true\n"
                                "PREVIOUS EXCHANGE DATE D95: 2016-Apr-01 13:13:13\n"
                                "BASE FARE AMOUNT C5A: 12.99 USD\n"
                                "DISCOUNT AMOUNT:\n"
                                "DISCOUNT PERCENTAGE:\n"
                                "MARK UP AMOUNT:\n"
                                "MARK UP PERCENTAGE:\n"
                                "******************* END EXCHANGE PLUS UP ******************\n"),
                         _collector->str());
  }


  void testAddAncillaryPriceModifiersToItinInfo_Empty()
  {
    Itin ancillaryItin;
    _collector->addAncillaryPriceModifiersToItinInfo(&ancillaryItin);
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testAddAncillaryPriceModifiersToItinInfo_SingleEntry()
  {
    Itin ancillaryItin;
    AncillaryIdentifier ancillaryIdentifier{"1S|C|0DF|1.2|1000"};
    AncillaryPriceModifier ancillaryPriceModifier{std::string("0"), 15};
    ancillaryItin.addAncillaryPriceModifier(ancillaryIdentifier, ancillaryPriceModifier);

    _collector->addAncillaryPriceModifiersToItinInfo(&ancillaryItin);
    CPPUNIT_ASSERT_EQUAL(string("  PRICE MODIFIERS:\n"
                                "    ANCILLARY IDENTIFIER: 1S|C|0DF|1.2|1000\n"
                                "      PRICE MODIFICATION:\n"
                                "        IDENTIFIER: 0\n"
                                "        QUANTITY: 15\n"), _collector->str());
  }

  void testAddAncillaryPriceModifiersToItinInfo_SingleEntry_OptionalDataNotSet()
  {
    Itin ancillaryItin;
    AncillaryIdentifier ancillaryIdentifier{"1S|C|0DF|1.2|1000"};
    AncillaryPriceModifier ancillaryPriceModifier{boost::none, 15};
    ancillaryItin.addAncillaryPriceModifier(ancillaryIdentifier, ancillaryPriceModifier);

    _collector->addAncillaryPriceModifiersToItinInfo(&ancillaryItin);
    CPPUNIT_ASSERT_EQUAL(string("  PRICE MODIFIERS:\n"
                                "    ANCILLARY IDENTIFIER: 1S|C|0DF|1.2|1000\n"
                                "      PRICE MODIFICATION:\n"
                                "        QUANTITY: 15\n"), _collector->str());
  }

  void testAddAncillaryPriceModifiersToItinInfo_RepeatedAid()
  {
    Itin ancillaryItin;
    AncillaryIdentifier ancillaryIdentifier{"1S|C|0DF|1.2|1000"};
    AncillaryPriceModifier ancillaryPriceModifier1{std::string("0"), 15};
    AncillaryPriceModifier ancillaryPriceModifier2{std::string("1"), 30};
    ancillaryItin.addAncillaryPriceModifier(ancillaryIdentifier, ancillaryPriceModifier1);
    ancillaryItin.addAncillaryPriceModifier(ancillaryIdentifier, ancillaryPriceModifier2);

    _collector->addAncillaryPriceModifiersToItinInfo(&ancillaryItin);
    CPPUNIT_ASSERT_EQUAL(string("  PRICE MODIFIERS:\n"
                                "    ANCILLARY IDENTIFIER: 1S|C|0DF|1.2|1000\n"
                                "      PRICE MODIFICATION:\n"
                                "        IDENTIFIER: 0\n"
                                "        QUANTITY: 15\n"
                                "      PRICE MODIFICATION:\n"
                                "        IDENTIFIER: 1\n"
                                "        QUANTITY: 30\n"), _collector->str());
  }

  void testAddAncillaryPriceModifiersToItinInfo_DoubleEntry()
  {
    Itin ancillaryItin;
    AncillaryIdentifier ancillaryIdentifier{"1S|C|0DF|1.2|1000"};
    AncillaryIdentifier ancillaryIdentifier2{"1S|C|0DF|1.2|2000"};
    AncillaryPriceModifier ancillaryPriceModifier1{std::string("0"), 15};
    AncillaryPriceModifier ancillaryPriceModifier2{std::string("1"), 30};
    ancillaryItin.addAncillaryPriceModifier(ancillaryIdentifier, ancillaryPriceModifier1);
    ancillaryItin.addAncillaryPriceModifier(ancillaryIdentifier2, ancillaryPriceModifier2);

    _collector->addAncillaryPriceModifiersToItinInfo(&ancillaryItin);
    CPPUNIT_ASSERT_EQUAL(string("  PRICE MODIFIERS:\n"
                                "    ANCILLARY IDENTIFIER: 1S|C|0DF|1.2|1000\n"
                                "      PRICE MODIFICATION:\n"
                                "        IDENTIFIER: 0\n"
                                "        QUANTITY: 15\n"
                                "    ANCILLARY IDENTIFIER: 1S|C|0DF|1.2|2000\n"
                                "      PRICE MODIFICATION:\n"
                                "        IDENTIFIER: 1\n"
                                "        QUANTITY: 30\n"), _collector->str());
  }

private:
  Diag198Collector* _collector;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _options;
  FareDisplayRequest* _fdRequest;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag198CollectorTest);
} // tse
