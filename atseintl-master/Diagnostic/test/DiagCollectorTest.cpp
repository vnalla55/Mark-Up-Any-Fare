//-----------------------------------------------------------------------------
//
//  File:     DiagCollectorTest.cpp
//
//  Author :  Konrad Koch
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include <time.h>
#include <vector>

#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "Diagnostic/DiagCollector.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{
class DiagCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiagCollectorTest);
  CPPUNIT_TEST(testGetRelationString);
  CPPUNIT_TEST(testGetRelationStringDefaultsToUNKNForUnrecognizedInput);

  CPPUNIT_TEST(testDisplayRequestedMarketReturnTrueFareMarketIsNotselected);
  CPPUNIT_TEST(testDisplayRequestedMarketReturnTrueCorrectFareMarketIsSelected);
  CPPUNIT_TEST(testDisplayRequestedMarketReturnTrueMultyCityFareMarketIsSelected);
  CPPUNIT_TEST(testDisplayRequestedMarketReturnFalseIncorrectFareMarketIsSelected);

  CPPUNIT_TEST(testDisplayFltTktMerchInd_F);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_T);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_R);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_M);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_A);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_C);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_P);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_B);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_E);
  CPPUNIT_TEST(testDisplayFltTktMerchInd_Blank);

  CPPUNIT_TEST(testDisplayTFDPSCForNetRemitPscResults);
  CPPUNIT_TEST(testDisplayTFDPSCForDisplayTicketedFareData);

  CPPUNIT_TEST(testPrintStopAtFirstMatchMsg);

  CPPUNIT_TEST(testFarePathAltDates);

  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  DiagCollector* _collector;

  PricingTrx* _trx;

  // Locations
  //
  const Loc* mel;
  const Loc* syd;
  const Loc* hkg; // Hong Kong
  const Loc* nrt; // Tokyo, Narita
  const Loc* nyc;
  const Loc* lga;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _collector = _memHandle.create<DiagCollector>();
    _collector->activate();

    mel = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEL.xml");
    syd = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    hkg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    nrt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    nyc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    lga = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGA.xml");
  }

  void tearDown() { _memHandle.clear(); }

  void testGetRelationString()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("IF"),
                         _collector->getRelationString(CategoryRuleItemInfo::IF));
    CPPUNIT_ASSERT_EQUAL(std::string("THEN"),
                         _collector->getRelationString(CategoryRuleItemInfo::THEN));
    CPPUNIT_ASSERT_EQUAL(std::string("OR"),
                         _collector->getRelationString(CategoryRuleItemInfo::OR));
    CPPUNIT_ASSERT_EQUAL(std::string("AND"),
                         _collector->getRelationString(CategoryRuleItemInfo::AND));
    CPPUNIT_ASSERT_EQUAL(std::string("ELSE"),
                         _collector->getRelationString(CategoryRuleItemInfo::ELSE));
  }

  void testGetRelationStringDefaultsToUNKNForUnrecognizedInput()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKN"), _collector->getRelationString(999));
  }

  void testDisplayRequestedMarketReturnTrueFareMarketIsNotselected()
  {
    // Create a fare market
    FareMarket fm;
    // Attach the travel segments to the fare market
    CPPUNIT_ASSERT_EQUAL(true, _collector->parseFareMarket(*_trx, fm));
  }

  void testDisplayRequestedMarketReturnTrueCorrectFareMarketIsSelected()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "MELHKG"));
    // Create test fare market
    FareMarket tfm;
    // Attach the travel segments to the fare market
    tfm.origin() = mel;
    tfm.destination() = hkg;
    tfm.boardMultiCity() = mel->loc();
    tfm.offMultiCity() = hkg->loc();

    CPPUNIT_ASSERT_EQUAL(true, _collector->parseFareMarket(*_trx, tfm));
  }

  void testDisplayRequestedMarketReturnTrueMultyCityFareMarketIsSelected()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "HKGLGA"));
    // Create test fare market
    FareMarket tfm;
    // Attach the travel segments to the fare market
    tfm.origin() = hkg;
    tfm.destination() = nyc;
    tfm.boardMultiCity() = hkg->loc();
    tfm.offMultiCity() = lga->loc();

    CPPUNIT_ASSERT_EQUAL(true, _collector->parseFareMarket(*_trx, tfm));
  }

  void testDisplayRequestedMarketReturnFalseIncorrectFareMarketIsSelected()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "MELHKG"));
    // Create test fare market
    FareMarket tfm;
    tfm.origin() = mel;
    tfm.destination() = nrt;
    tfm.boardMultiCity() = mel->loc();
    tfm.offMultiCity() = nrt->loc();

    CPPUNIT_ASSERT_EQUAL(false, _collector->parseFareMarket(*_trx, tfm));
  }

  void testDisplayFltTktMerchInd_F()
  {
    _collector->displayFltTktMerchInd(FLIGHT_RELATED_SERVICE);
    CPPUNIT_ASSERT_EQUAL(std::string("FLIGHT   "), _collector->str());
  }

  void testDisplayFltTktMerchInd_T()
  {
    _collector->displayFltTktMerchInd(TICKET_RELATED_SERVICE);
    CPPUNIT_ASSERT_EQUAL(std::string("TICKET   "), _collector->str());
  }

  void testDisplayFltTktMerchInd_R()
  {
    _collector->displayFltTktMerchInd(RULE_BUSTER_SERVICE);
    CPPUNIT_ASSERT_EQUAL(std::string("RULE     "), _collector->str());
  }

  void testDisplayFltTktMerchInd_M()
  {
    _collector->displayFltTktMerchInd(MERCHANDISE_SERVICE);
    CPPUNIT_ASSERT_EQUAL(std::string("MERCHANT "), _collector->str());
  }

  void testDisplayFltTktMerchInd_A()
  {
    _collector->displayFltTktMerchInd(BAGGAGE_ALLOWANCE);
    CPPUNIT_ASSERT_EQUAL(std::string("ALLOWANCE"), _collector->str());
  }

  void testDisplayFltTktMerchInd_C()
  {
    _collector->displayFltTktMerchInd(BAGGAGE_CHARGE);
    CPPUNIT_ASSERT_EQUAL(std::string("CHARGE   "), _collector->str());
  }

  void testDisplayFltTktMerchInd_P()
  {
    _collector->displayFltTktMerchInd(PREPAID_BAGGAGE);
    CPPUNIT_ASSERT_EQUAL(std::string("PREPAID  "), _collector->str());
  }

  void testDisplayFltTktMerchInd_B()
  {
    _collector->displayFltTktMerchInd(CARRY_ON_ALLOWANCE);
    CPPUNIT_ASSERT_EQUAL(std::string("CARRYFBA "), _collector->str());
  }

  void testDisplayFltTktMerchInd_E()
  {
    _collector->displayFltTktMerchInd(BAGGAGE_EMBARGO);
    CPPUNIT_ASSERT_EQUAL(std::string("EMBARGO  "), _collector->str());
  }

  void testDisplayFltTktMerchInd_Blank()
  {
    _collector->displayFltTktMerchInd(' ');
    CPPUNIT_ASSERT_EQUAL(std::string("         "), _collector->str());
  }

  void prepareDisplayTFDPSCCase(std::vector<const NegFareRestExtSeq*>& negFareRestExtSeq)
  {
    NegFareRestExtSeq* restExtSeq1 = _memHandle.create<NegFareRestExtSeq>();
    NegFareRestExtSeq* restExtSeq2 = _memHandle.create<NegFareRestExtSeq>();

    restExtSeq1->seqNo() = 100;
    restExtSeq1->cityFrom() = "SYD";
    restExtSeq1->cityTo() = "KUL";
    restExtSeq1->carrier() = "MH";
    restExtSeq1->viaCity1() = "AAA";
    restExtSeq1->viaCity2() = "BBB";
    restExtSeq1->viaCity3() = "CCC";
    restExtSeq1->viaCity4() = "DDD";
    restExtSeq1->publishedFareBasis() = "Y2345678";
    restExtSeq1->uniqueFareBasis() = "U2345678";
    restExtSeq1->suppressNvbNva() = 'X';

    restExtSeq2->seqNo() = 200;
    restExtSeq2->cityFrom() = "BLR";
    restExtSeq2->cityTo() = "SIN";
    restExtSeq2->carrier() = "SQ";
    restExtSeq2->viaCity1() = "EEE";
    restExtSeq2->viaCity2() = "FFF";
    restExtSeq2->publishedFareBasis() = "C-";
    restExtSeq2->uniqueFareBasis() = "TEST1234";

    negFareRestExtSeq.push_back(restExtSeq1);
    negFareRestExtSeq.push_back(restExtSeq2);
  }

  void testDisplayTFDPSCForNetRemitPscResults()
  {
    std::vector<const NegFareRestExtSeq*> negFareRestExtSeq;
    prepareDisplayTFDPSCCase(negFareRestExtSeq);

    _collector->displayTFDPSC(*_collector, negFareRestExtSeq, true);

    CPPUNIT_ASSERT_EQUAL(
        std::string("---------- MATCHED SEQUENCES FOR TICKETED FARE DATA ---------- \n"
                    "SEQNO  FRM TO  CXR VIA POINTS      FARE BASIS   SPNV\n"
                    "100    SYD KUL MH  AAA/BBB/CCC/DDD Y2345678     Y\n"
                    "200    BLR SIN SQ  EEE/FFF         C-           N\n"),
        _collector->str());
  }

  void testDisplayTFDPSCForDisplayTicketedFareData()
  {
    std::vector<const NegFareRestExtSeq*> negFareRestExtSeq;
    prepareDisplayTFDPSCCase(negFareRestExtSeq);

    _collector->displayTFDPSC(*_collector, negFareRestExtSeq, false);

    CPPUNIT_ASSERT_EQUAL(
        std::string("  TICKETED FARE DATA PER SEGMENT/COMPONENT :\n"
                    "  SEQNO  FRM TO  CXR VIA POINTS      FARE BASIS  UNFBC    SPNV\n"
                    "  100    SYD KUL MH  AAA/BBB/CCC/DDD Y2345678    U2345678 Y\n"
                    "  200    BLR SIN SQ  EEE/FFF         C-          TEST1234 N\n"),
        _collector->str());
  }

  void testPrintStopAtFirstMatchMsg()
  {
    _collector->printStopAtFirstMatchMsg();
    CPPUNIT_ASSERT_EQUAL(
        std::string(" *** STOP AFTER FIRST MATCH - NO GROUP CODE IN THE ENTRY ***\n"),
        _collector->str());
  }

  void testFarePathAltDates()
  {
    _collector->trx() = _trx;
    _trx->setAltDates(true);

    PricingOptions pricingOptions;
    _trx->setOptions(&pricingOptions);

    FarePath farePath;
    farePath.setTotalNUCAmount(1253.26);

    PaxType paxType;
    paxType.paxType() = "NEG";
    farePath.paxType() = &paxType;

    Itin itin;
    std::pair<DateTime, DateTime> datePair;
    datePair.first = DateTime(2012, Nov, 24);
    datePair.second = DateTime(2012, Feb, 16);
    itin.datePair() = &datePair;
    farePath.itin() = &itin;

    (*_collector) << farePath;

    std::string expected(" AMOUNT: 1253.26 REQUESTED PAXTYPE: NEG DT-PAIR: 24NOV-16FEB");
    std::string actual(_collector->str());

    actual.resize(expected.size());

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DiagCollectorTest);

} // namespace
