//-----------------------------------------------------------------------------
//
//  File:     Diag335CollectorTest.cpp
//
//  Author :  Slawek Machowicz
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
#include <time.h>
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag335Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Rules/RuleConst.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "DBAccess/NegFareCalcInfo.h"
#include "DBAccess/MarkupCalculate.h"
#include "DBAccess/NegFareRest.h"

using namespace std;
namespace tse
{
class Diag335CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag335CollectorTest);

  CPPUNIT_TEST(testDisplayLocKeys);
  CPPUNIT_TEST(testDisplayLocKeysEmpty);

  CPPUNIT_TEST(testDisplayCategoryRuleInfo);
  CPPUNIT_TEST(testDisplayCategoryRuleInfoNotApplicable);
  CPPUNIT_TEST(testDisplayCategoryRuleInfoNotActive);

  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintHeaderNotActive);

  CPPUNIT_TEST(testDisplayFareMarket);
  CPPUNIT_TEST(testDisplayFareMarketNotActive);

  CPPUNIT_TEST(testDisplayPaxTypeFare);
  CPPUNIT_TEST(testDisplayPaxTypeFareNotActive);

  CPPUNIT_TEST(testDisplayCategoryRuleItemInfo);
  CPPUNIT_TEST(testDisplayCategoryRuleItemInfoNotActive);

  CPPUNIT_TEST(testDisplayNegFareSecurityInfo);
  CPPUNIT_TEST(testDisplayNegFareSecurityInfoNotActive);

  CPPUNIT_TEST(testDisplayNegFareCalcInfoInvalidIndicator);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoNotActive);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoC);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoA);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoM);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoS);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoP);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoN);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoT);
  CPPUNIT_TEST(testDisplayNegFareCalcInfoR);

  CPPUNIT_TEST(testDisplayMarkupCalculateInvalidIndicator);
  CPPUNIT_TEST(testDisplayMarkupCalculateNotActive);
  CPPUNIT_TEST(testDisplayMarkupCalculateCP);
  CPPUNIT_TEST(testDisplayMarkupCalculateAN);
  CPPUNIT_TEST(testDisplayMarkupCalculateMT);
  CPPUNIT_TEST(testDisplayMarkupCalculateSR);

  CPPUNIT_TEST(testDisplayNegFareRest);
  CPPUNIT_TEST(testDisplayNegFareRestNotActive);

  CPPUNIT_TEST(testDoNetRemitTkt);
  CPPUNIT_TEST(testDoNetRemitTktAxess);
  CPPUNIT_TEST(testDoNetRemitTktRecurring);
  CPPUNIT_TEST(testDoNetRemitTktRecurringAxess);
  CPPUNIT_TEST(testDoNetRemitTktNotActive);

  CPPUNIT_TEST(testDisplayFailCodePass);
  CPPUNIT_TEST(testDisplayFailCodeNotActive);
  CPPUNIT_TEST(testDisplayFailCodeWithoutCxr);
  CPPUNIT_TEST(testDisplayFailCodeWithCxr);
  CPPUNIT_TEST(testDisplayFailCodeWithCxrNotAppl);
  CPPUNIT_TEST(testDisplayMessageMoveInItemString);
  CPPUNIT_TEST(testDisplayMessageAndMovetoCollector);

  CPPUNIT_TEST_SUITE_END();

private:
  Diag335Collector* _collector;
  Diagnostic* _diagroot;
  PricingTrx* _trx;
  DataHandle _dataHandle;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    try
    {
      _diagroot = _memHandle(new Diagnostic(Diagnostic335));
      _diagroot->activate();
      _collector = _memHandle(new Diag335Collector(*_diagroot));
      _collector->enable(Diagnostic335);
    }
    catch (...) { CPPUNIT_ASSERT(false); }

    _dataHandle.get(_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void testDisplayLocKeys()
  {
    LocKey loc1, loc2;
    loc1.loc() = "AAA";
    loc2.loc() = "BBB";
    loc1.locType() = 'A';
    loc2.locType() = 'B';

    _collector->displayLocKeys(loc1, loc2);
    string response = "\nLOC1TYPE-   ALOC1-   AAA        LOC2TYPE-   BLOC2-   BBB\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayLocKeysEmpty()
  {
    LocKey loc1, loc2;
    loc1.loc() = "AAA";
    loc2.loc() = "BBB";
    loc1.locType() = RuleConst::ANY_LOCATION_TYPE;
    loc2.locType() = RuleConst::ANY_LOCATION_TYPE;

    _collector->displayLocKeys(loc1, loc2);

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDisplayCategoryRuleInfo()
  {
    CategoryRuleInfo ruleInfo;
    ruleInfo.sequenceNumber() = 1;

    *_collector << ruleInfo;
    string response = "R2:SEQ NBR 1       \n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayCategoryRuleInfoNotApplicable()
  {
    CategoryRuleInfo ruleInfo;
    ruleInfo.sequenceNumber() = 1;

    ruleInfo.applInd() = RuleConst::STRING_DOES_NOT_APPLY;

    *_collector << ruleInfo;
    string response = "R2:SEQ NBR 1               *** NOT APPLICABLE ***\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayCategoryRuleInfoNotActive()
  {
    CategoryRuleInfo ruleInfo;
    _collector->disable(Diagnostic335);

    *_collector << ruleInfo;

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testPrintHeader()
  {
    _collector->printHeader();
    string response = "***************  NET FARE MARK-UP DIAGNOSTICS  ****************\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintHeaderNotActive()
  {
    _collector->disable(Diagnostic335);

    _collector->printHeader();

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testDisplayFareMarket()
  {
    FareMarket fareMarket;
    fareMarket.boardMultiCity() = "AAA";
    fareMarket.governingCarrier() = "CC";
    fareMarket.offMultiCity() = "BBB";

    *_collector << fareMarket;
    string response =
        "\n***************************************************************\nAAA-CC-BBB\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testDisplayFareMarketNotActive()
  {
    FareMarket fareMarket;
    _collector->disable(Diagnostic335);

    *_collector << fareMarket;

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testDisplayPaxTypeFare()
  {
    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fca;
    FareClassAppSegInfo fcas;
    ptf.fareClassAppInfo() = &fca;
    ptf.fareClassAppSegInfo() = &fcas;
    fare.setFareInfo(&fareInfo);

    fareInfo.fareClass() = "FC";
    fareInfo.vendor() = "ATP";
    fareInfo.ruleNumber() = "RN";
    fareInfo.fareTariff() = 1;
    fareInfo.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo.fareAmount() = 100;
    fareInfo.currency() = "PLN";

    ptf.setFare(&fare);

    fca._fareType = "FT";
    fcas._paxType = "PT";
    fca._displayCatType = 'X';

    *_collector << ptf;
    string response = "---------------------------------------------------------------\n"
                      "FC       ATP  RN   1   RT   100.00 PLN FT-FT  PTC-PT  DCT-X \n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayPaxTypeFareNotActive()
  {
    PaxTypeFare ptf;
    _collector->disable(Diagnostic335);

    *_collector << ptf;

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDisplayCategoryRuleItemInfo()
  {
    CategoryRuleItemInfo criInfo;
    criInfo.setRelationalInd(CategoryRuleItemInfo::IF);
    criInfo.setItemNo(1);
    criInfo.setDirectionality('X');
    criInfo.setInOutInd('Y');

    *_collector << criInfo;
    string response = "R2S: IF   0   1          DIR-X  I/O-Y \n";
    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayCategoryRuleItemInfoNotActive()
  {
    CategoryRuleItemInfo criInfo;
    _collector->disable(Diagnostic335);

    *_collector << criInfo;

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDisplayNegFareSecurityInfo()
  {
    NegFareSecurityInfo nfsInfo;
    nfsInfo.seqNo() = 1;
    nfsInfo.updateInd() = 'A';
    nfsInfo.redistributeInd() = 'B';
    nfsInfo.sellInd() = 'C';
    nfsInfo.ticketInd() = 'D';
    nfsInfo.secondarySellerId() = 2;

    *_collector << nfsInfo;
    string response = "T983:SEQ NBR 1       UPD-A REDIS-B SELL-C TKT-D T980-2\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareSecurityInfoNotActive()
  {
    NegFareSecurityInfo nfsInfo;
    _collector->disable(Diagnostic335);

    *_collector << nfsInfo;

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoInvalidIndicator()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'B';

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-B\nunknown fare indicator\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoNotActive()
  {
    NegFareCalcInfo nfcInfo;
    _collector->disable(Diagnostic335);

    *_collector << nfcInfo;

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoC()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'C';
    nfcInfo.sellingPercent() = 1;

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-C\n"
                      "PCT-1.00   \n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoA()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'A';
    nfcInfo.sellingPercent() = 1;
    nfcInfo.sellingFareAmt1() = 2;
    nfcInfo.sellingCur1() = "PLN";
    nfcInfo.sellingFareAmt2() = 3;
    nfcInfo.sellingCur2() = "USD";

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-A\n"
                      "PCT-1.00   FARE1-      2.00 PLN FARE2-      3.00 USD\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoM()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'M';
    nfcInfo.sellingPercent() = 1;
    nfcInfo.sellingFareAmt1() = 2;
    nfcInfo.sellingCur1() = "PLN";
    nfcInfo.sellingFareAmt2() = 3;
    nfcInfo.sellingCur2() = "USD";

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-M\n"
                      "PCT-1.00   FARE1-      2.00 PLN FARE2-      3.00 USD\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoS()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'S';
    nfcInfo.sellingFareAmt1() = 2;
    nfcInfo.sellingCur1() = "PLN";
    nfcInfo.sellingFareAmt2() = 3;
    nfcInfo.sellingCur2() = "USD";

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-S\n"
                      "FARE1-      2.00 PLN FARE2-      3.00 USD\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoP()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'P';
    nfcInfo.calcPercentMin() = 1;
    nfcInfo.calcPercentMax() = 2;

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-P\n"
                      "MIN PCT-1.00    MAX PCT-2.00   \n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoN()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'N';
    nfcInfo.calcPercentMin() = 1;
    nfcInfo.calcPercentMax() = 2;
    nfcInfo.calcMinFareAmt1() = 3;
    nfcInfo.calcMaxFareAmt1() = 4;
    nfcInfo.calcMinFareAmt2() = 5;
    nfcInfo.calcMaxFareAmt2() = 6;
    nfcInfo.calcCur1() = "PLN";
    nfcInfo.calcCur2() = "USD";

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-N\n"
                      "MIN PCT-1.00    MAX PCT-2.00   \n"
                      "MIN FARE1-      3.00 PLN MAX FARE1-      4.00 PLN\n"
                      "MIN FARE2-      5.00 USD MAX FARE2-      6.00 USD\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoT()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'T';
    nfcInfo.calcPercentMin() = 1;
    nfcInfo.calcPercentMax() = 2;
    nfcInfo.calcMinFareAmt1() = 3;
    nfcInfo.calcMaxFareAmt1() = 4;
    nfcInfo.calcMinFareAmt2() = 5;
    nfcInfo.calcMaxFareAmt2() = 6;
    nfcInfo.calcCur1() = "PLN";
    nfcInfo.calcCur2() = "USD";

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-T\n"
                      "MIN PCT-1.00    MAX PCT-2.00   \n"
                      "MIN FARE1-      3.00 PLN MAX FARE1-      4.00 PLN\n"
                      "MIN FARE2-      5.00 USD MAX FARE2-      6.00 USD\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareCalcInfoR()
  {
    NegFareCalcInfo nfcInfo;
    nfcInfo.seqNo() = 1;
    nfcInfo.netSellingInd() = 'A';
    nfcInfo.fareInd() = 'R';
    nfcInfo.calcMinFareAmt1() = 3;
    nfcInfo.calcMaxFareAmt1() = 4;
    nfcInfo.calcMinFareAmt2() = 5;
    nfcInfo.calcMaxFareAmt2() = 6;
    nfcInfo.calcCur1() = "PLN";
    nfcInfo.calcCur2() = "USD";

    *_collector << nfcInfo;
    string response = "T979:SEQ NBR 1       N/S-NET       F/IND-R\n"
                      "MIN FARE1-      3.00 PLN MAX FARE1-      4.00 PLN\n"
                      "MIN FARE2-      5.00 USD MAX FARE2-      6.00 USD\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayMarkupCalculateNotActive()
  {
    MarkupCalculate mc;
    _collector->disable(Diagnostic335);

    *_collector << mc;

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDisplayMarkupCalculateInvalidIndicator()
  {
    MarkupCalculate mc;
    mc.orderNo() = 1;
    mc.sellingFareInd() = ' ';
    mc.markupFareInd() = ' ';

    *_collector << mc;
    string response = "T980:ORDER NBR 1       F/IND- \n"
                      "MU/IND- \n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayMarkupCalculateCP()
  {
    MarkupCalculate mc;
    mc.orderNo() = 1;
    mc.sellingFareInd() = 'C';
    mc.markupFareInd() = 'P';
    mc.sellingPercent() = 1;
    mc.percentMin() = 2;
    mc.percentMax() = 3;

    *_collector << mc;
    string response = "T980:ORDER NBR 1       F/IND-C\n"
                      "PCT-1.00   \n"
                      "MU/IND-P\n"
                      "MIN PCT-2.00    MAX PCT-3.00   \n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayMarkupCalculateAN()
  {
    MarkupCalculate mc;
    mc.orderNo() = 1;
    mc.sellingFareInd() = 'A';
    mc.markupFareInd() = 'N';
    mc.sellingPercent() = 1;
    mc.percentMin() = 2;
    mc.percentMax() = 3;

    mc.sellingFareAmt1() = 4;
    mc.sellingCur1() = "PLN";
    mc.sellingFareAmt2() = 5;
    mc.sellingCur2() = "USD";

    mc.markupMinAmt1() = 6;
    mc.markupMaxAmt1() = 7;
    mc.markupCur1() = "EUR";
    mc.markupMinAmt2() = 8;
    mc.markupMaxAmt2() = 9;
    mc.markupCur2() = "JPY";

    *_collector << mc;
    string response = "T980:ORDER NBR 1       F/IND-A\n"
                      "PCT-1.00   FARE1-      4.00 PLN FARE2-      5.00 USD\n"
                      "MU/IND-N\n"
                      "MIN PCT-2.00    MAX PCT-3.00   \n"
                      "MIN FARE1-      6.00 EUR MAX FARE1-      7.00 EUR\n"
                      "MIN FARE2-         8 JPY MAX FARE2-         9 JPY\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayMarkupCalculateMT()
  {
    MarkupCalculate mc;
    mc.orderNo() = 1;
    mc.sellingFareInd() = 'M';
    mc.markupFareInd() = 'T';
    mc.sellingPercent() = 1;
    mc.percentMin() = 2;
    mc.percentMax() = 3;

    mc.sellingFareAmt1() = 4;
    mc.sellingCur1() = "PLN";
    mc.sellingFareAmt2() = 5;
    mc.sellingCur2() = "USD";

    mc.markupMinAmt1() = 6;
    mc.markupMaxAmt1() = 7;
    mc.markupCur1() = "EUR";
    mc.markupMinAmt2() = 8;
    mc.markupMaxAmt2() = 9;
    mc.markupCur2() = "JPY";

    *_collector << mc;
    string response = "T980:ORDER NBR 1       F/IND-M\n"
                      "PCT-1.00   FARE1-      4.00 PLN FARE2-      5.00 USD\n"
                      "MU/IND-T\n"
                      "MIN PCT-2.00    MAX PCT-3.00   \n"
                      "MIN FARE1-      6.00 EUR MAX FARE1-      7.00 EUR\n"
                      "MIN FARE2-         8 JPY MAX FARE2-         9 JPY\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayMarkupCalculateSR()
  {
    MarkupCalculate mc;
    mc.orderNo() = 1;
    mc.sellingFareInd() = 'S';
    mc.markupFareInd() = 'R';

    mc.sellingFareAmt1() = 4;
    mc.sellingCur1() = "PLN";
    mc.sellingFareAmt2() = 5;
    mc.sellingCur2() = "USD";

    mc.markupMinAmt1() = 6;
    mc.markupMaxAmt1() = 7;
    mc.markupCur1() = "EUR";
    mc.markupMinAmt2() = 8;
    mc.markupMaxAmt2() = 9;
    mc.markupCur2() = "JPY";

    *_collector << mc;
    string response = "T980:ORDER NBR 1       F/IND-S\n"
                      "FARE1-      4.00 PLN FARE2-      5.00 USD\n"
                      "MU/IND-R\n"
                      "MIN FARE1-      6.00 EUR MAX FARE1-      7.00 EUR\n"
                      "MIN FARE2-         8 JPY MAX FARE2-         9 JPY\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareRest()
  {
    NegFareRest nfRest;
    nfRest.itemNo() = 1;
    nfRest.psgType() = "ADT";
    nfRest.overrideDateTblItemNo() = 2;
    nfRest.negFareSecurityTblItemNo() = 3;
    nfRest.negFareCalcTblItemNo() = 4;

    *_collector << nfRest;
    string response = "\nR3:1       PTC-ADT  T994-2       T983-3       T979-4       \n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayNegFareRestNotActive()
  {
    NegFareRest nfRest;
    _collector->disable(Diagnostic335);

    *_collector << nfRest;

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDoNetRemitTkt()
  {
    NegFareRest nfRest;
    nfRest.netRemitMethod() = 'A';
    nfRest.tktFareDataInd1() = 'B';
    nfRest.owrt1() = 'C';
    nfRest.seasonType1() = 'D';
    nfRest.dowType1() = 'E';
    nfRest.globalDir1() = GlobalDirection::XX;
    nfRest.carrier11() = "AA";
    nfRest.rule1() = "R1";
    nfRest.ruleTariff1() = 1;
    nfRest.fareType1() = "FT";
    nfRest.fareClass1() = "FC";
    nfRest.betwCity1() = "AAA";
    nfRest.andCity1() = "BBB";
    nfRest.commPercent() = 2;
    nfRest.cur1() = "";
    nfRest.cur2() = "";

    _collector->doNetRemitTkt(nfRest, false, false);
    string response = " TICKETING FARE DATA: \n"
                      " SEGMENT 1 \n"
                      "  NET REMIT METHOD   - A\n"
                      "  TKT FARE INDICATOR - B       OWRT INDICATOR     - C\n"
                      "  SEASON TYPE        - D       DOW INDICATOR      - E\n"
                      "  GLOBAL DIRECTION   - XX      CARRIER CODE       - AA\n"
                      "  RULE NUMBER        -   R1    RULE TARIFF        -    1\n"
                      "  FARE TYPE          -   FT    FARE CLASS CODE    -       FC\n"
                      "  BTW CITY CODE      - AAA     AND CITY CODE      - BBB\n"
                      " COMMISSIONS DATA:\n"
                      "  COMM PERCENT       -        2\n"
                      "  COMMISSIONS 1      -     NONE\n"
                      "  COMMISSIONS 2      -     NONE\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDoNetRemitTktAxess()
  {
    NegFareRest nfRest;
    nfRest.netRemitMethod() = 'A';
    nfRest.tktFareDataInd1() = 'B';
    nfRest.owrt1() = 'C';
    nfRest.seasonType1() = 'D';
    nfRest.dowType1() = 'E';
    nfRest.globalDir1() = GlobalDirection::XX;
    nfRest.carrier11() = "AA";
    nfRest.rule1() = "R1";
    nfRest.ruleTariff1() = 1;
    nfRest.fareType1() = "FT";
    nfRest.fareClass1() = "FC";
    nfRest.betwCity1() = "AAA";
    nfRest.andCity1() = "BBB";
    nfRest.commPercent() = 2;
    nfRest.cur1() = "";
    nfRest.cur2() = "";

    _collector->doNetRemitTkt(nfRest, true, false);
    string response = " TICKETING FARE DATA: \n"
                      " SEGMENT 1 \n"
                      "  NET REMIT METHOD   - A\n"
                      "  TKT FARE INDICATOR - B       OWRT INDICATOR     - C\n"
                      "  SEASON TYPE      *X- D       DOW INDICATOR    *X- E\n"
                      "  GLOBAL DIRECTION   - XX      CARRIER CODE       - AA\n"
                      "  RULE NUMBER        -   R1    RULE TARIFF        -    1\n"
                      "  FARE TYPE        *X-   FT    FARE CLASS CODE  *X-       FC\n"
                      "  BTW CITY CODE    *X- AAA     AND CITY CODE    *X- BBB\n"
                      " COMMISSIONS DATA:\n"
                      "  COMM PERCENT       -        2\n"
                      "  COMMISSIONS 1    *X-     NONE\n"
                      "  COMMISSIONS 2    *X-     NONE\n"
                      "          ** NOTE: *X- IS IGNORED FOR AXESS USER\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDoNetRemitTktRecurring()
  {
    NegFareRest nfRest;
    nfRest.netRemitMethod() = 'A';
    nfRest.tktFareDataInd1() = 'B';
    nfRest.owrt1() = 'C';
    nfRest.seasonType1() = 'D';
    nfRest.dowType1() = 'E';
    nfRest.globalDir1() = GlobalDirection::XX;
    nfRest.carrier11() = "AA";
    nfRest.rule1() = "R1";
    nfRest.ruleTariff1() = 1;
    nfRest.fareType1() = "FT";
    nfRest.fareClass1() = "FC";
    nfRest.betwCity1() = "AAA";
    nfRest.andCity1() = "BBB";
    nfRest.commPercent() = 2;
    nfRest.cur1() = "";
    nfRest.cur2() = "";

    nfRest.tktFareDataInd2() = 'F';
    nfRest.owrt2() = 'G';
    nfRest.seasonType2() = 'H';
    nfRest.dowType2() = 'I';
    nfRest.carrier21() = "BB";
    nfRest.rule2() = "R2";
    nfRest.ruleTariff2() = 3;
    nfRest.fareType2() = "FT2";
    nfRest.fareClass2() = "FC2";
    nfRest.betwCity2() = "CCC";
    nfRest.andCity2() = "DDD";

    _collector->doNetRemitTkt(nfRest, false, true);
    string response = " TICKETING FARE DATA: \n"
                      " SEGMENT 1 \n"
                      "  NET REMIT METHOD   - A\n"
                      "  TKT FARE INDICATOR - B       OWRT INDICATOR     - C\n"
                      "  SEASON TYPE        - D       DOW INDICATOR      - E\n"
                      "  GLOBAL DIRECTION   - XX      CARRIER CODE       - AA\n"
                      "  RULE NUMBER        -   R1    RULE TARIFF        -    1\n"
                      "  FARE TYPE          -   FT    FARE CLASS CODE    -       FC\n"
                      "  BTW CITY CODE      - AAA     AND CITY CODE      - BBB\n"
                      " SEGMENT 2 \n"
                      "  TKT FARE INDICATOR - F       OWRT INDICATOR     - G\n"
                      "  SEASON TYPE        - H       DOW INDICATOR      - I\n"
                      "  GLOBAL DIRECTION   - XX      CARRIER CODE       - BB\n"
                      "  RULE NUMBER        -   R2    RULE TARIFF        -    3\n"
                      "  FARE TYPE          -  FT2    FARE CLASS CODE    -      FC2\n"
                      "  BTW CITY CODE      - CCC     AND CITY CODE      - DDD\n"
                      " COMMISSIONS DATA:\n"
                      "  COMM PERCENT       -        2\n"
                      "  COMMISSIONS 1      -     NONE\n"
                      "  COMMISSIONS 2      -     NONE\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDoNetRemitTktRecurringAxess()
  {
    NegFareRest nfRest;
    nfRest.netRemitMethod() = 'A';
    nfRest.tktFareDataInd1() = 'B';
    nfRest.owrt1() = 'C';
    nfRest.seasonType1() = 'D';
    nfRest.dowType1() = 'E';
    nfRest.globalDir1() = GlobalDirection::XX;
    nfRest.carrier11() = "AA";
    nfRest.rule1() = "R1";
    nfRest.ruleTariff1() = 1;
    nfRest.fareType1() = "FT";
    nfRest.fareClass1() = "FC";
    nfRest.betwCity1() = "AAA";
    nfRest.andCity1() = "BBB";
    nfRest.commPercent() = 2;
    nfRest.cur1() = "PLN";
    nfRest.commAmt1() = 4;
    nfRest.cur2() = "USD";
    nfRest.commAmt2() = 5;

    nfRest.tktFareDataInd2() = 'F';
    nfRest.owrt2() = 'G';
    nfRest.seasonType2() = 'H';
    nfRest.dowType2() = 'I';
    nfRest.carrier21() = "BB";
    nfRest.rule2() = "R2";
    nfRest.ruleTariff2() = 3;
    nfRest.fareType2() = "FT2";
    nfRest.fareClass2() = "FC2";
    nfRest.betwCity2() = "CCC";
    nfRest.andCity2() = "DDD";

    _collector->doNetRemitTkt(nfRest, true, true);
    string response = " TICKETING FARE DATA: \n"
                      " SEGMENT 1 \n"
                      "  NET REMIT METHOD   - A\n"
                      "  TKT FARE INDICATOR - B       OWRT INDICATOR     - C\n"
                      "  SEASON TYPE      *X- D       DOW INDICATOR    *X- E\n"
                      "  GLOBAL DIRECTION   - XX      CARRIER CODE       - AA\n"
                      "  RULE NUMBER        -   R1    RULE TARIFF        -    1\n"
                      "  FARE TYPE        *X-   FT    FARE CLASS CODE  *X-       FC\n"
                      "  BTW CITY CODE    *X- AAA     AND CITY CODE    *X- BBB\n"
                      " SEGMENT 2 \n"
                      "  TKT FARE INDICATOR - F       OWRT INDICATOR     - G\n"
                      "  SEASON TYPE      *X- H       DOW INDICATOR    *X- I\n"
                      "  GLOBAL DIRECTION   - XX      CARRIER CODE       - BB\n"
                      "  RULE NUMBER        -   R2    RULE TARIFF        -    3\n"
                      "  FARE TYPE        *X-  FT2    FARE CLASS CODE  *X-      FC2\n"
                      "  BTW CITY CODE    *X- CCC     AND CITY CODE    *X- DDD\n"
                      " COMMISSIONS DATA:\n"
                      "  COMM PERCENT       -        2\n"
                      "  COMMISSIONS 1    *X-     4.00 PLN\n"
                      "  COMMISSIONS 2    *X-     5.00 USD\n"
                      "          ** NOTE: *X- IS IGNORED FOR AXESS USER\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDoNetRemitTktNotActive()
  {
    NegFareRest nfRest;
    _collector->disable(Diagnostic335);

    _collector->doNetRemitTkt(nfRest, false, false);
    _collector->doNetRemitTkt(nfRest, false, true);
    _collector->doNetRemitTkt(nfRest, true, false);
    _collector->doNetRemitTkt(nfRest, true, true);

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDisplayFailCodePass()
  {
    NegFareRest nfRest;
    nfRest.itemNo() = 1;
    nfRest.psgType() = "ADT";
    nfRest.overrideDateTblItemNo() = 2;
    nfRest.negFareSecurityTblItemNo() = 3;
    nfRest.negFareCalcTblItemNo() = 4;

    _collector->displayFailCode(nfRest, Diag335Collector::R3_PASS);
    string response = "\nR3:1       PTC-ADT  T994-2       T983-3       T979-4       \n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayFailCodeNotActive()
  {
    NegFareRest nfRest;
    _collector->disable(Diagnostic335);

    _collector->displayFailCode(nfRest, Diag335Collector::R3_PASS);

    CPPUNIT_ASSERT_EQUAL(string(""), _collector->_itemString);
  }

  void testDisplayFailCodeWithoutCxr()
  {
    NegFareRest nfRest;
    nfRest.itemNo() = 1;
    nfRest.psgType() = "ADT";
    nfRest.overrideDateTblItemNo() = 2;
    nfRest.carrier() = "";

    _collector->displayFailCode(nfRest, Diag335Collector::FAIL_PTC);
    string response = "R3:       1PTC-  ADTT994-       2\n"
                      "     *FAIL* PAXTYPE\n\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayFailCodeWithCxr()
  {
    NegFareRest nfRest;
    nfRest.itemNo() = 1;
    nfRest.psgType() = "ADT";
    nfRest.overrideDateTblItemNo() = 2;
    nfRest.carrier() = "AA";
    nfRest.tktAppl() = ' ';

    _collector->displayFailCode(nfRest, Diag335Collector::FAIL_PTC);
    string response = "R3:       1PTC-  ADTT994-       2CXR - AA\n"
                      "     *FAIL* PAXTYPE\n\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }

  void testDisplayFailCodeWithCxrNotAppl()
  {
    NegFareRest nfRest;
    nfRest.itemNo() = 1;
    nfRest.psgType() = "ADT";
    nfRest.overrideDateTblItemNo() = 2;
    nfRest.carrier() = "AA";
    nfRest.tktAppl() = 'A';

    _collector->displayFailCode(nfRest, Diag335Collector::FAIL_PTC);
    string response = "R3:       1PTC-  ADTT994-       2CXR NOT - AA\n"
                      "     *FAIL* PAXTYPE\n\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }
  void testDisplayMessageMoveInItemString()
  {
    const string& msg = "MOVE MESSAGE TO ITEM STRING";
    bool showDiag = false;

    _collector->displayMessage(msg, showDiag);
    string response = "MOVE MESSAGE TO ITEM STRING";

    CPPUNIT_ASSERT_EQUAL(response, _collector->_itemString);
  }
  void testDisplayMessageAndMovetoCollector()
  {
    _collector->printHeader();
    _collector->_itemString = "PREVIOUS CONTENT OF THE ITEM HERE\n";
    const string& msg = "MOVING IT TO COLLECTOR";
    bool showDiag = true;

    _collector->displayMessage(msg, showDiag);
    string response = "***************  NET FARE MARK-UP DIAGNOSTICS  ****************\n"
                      "PREVIOUS CONTENT OF THE ITEM HERE\n"
                      "MOVING IT TO COLLECTOR\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag335CollectorTest);
}
