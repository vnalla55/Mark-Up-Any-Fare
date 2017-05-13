// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcConsts.h"
#include "Xform/XformUtil.h"

namespace tse
{

class XformUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XformUtilTest);
  CPPUNIT_TEST(testFormatFc_remove);
  CPPUNIT_TEST(testFormatFc_Add);
  CPPUNIT_TEST(testFormatFc_AddLast);
  CPPUNIT_TEST(testFormatFc_AddFailed);
  CPPUNIT_TEST(testFormatFc_NoBaggage);

  CPPUNIT_TEST(splitNewLine_onlySplit);
  CPPUNIT_TEST(splitNewLine_trimToLeft);
  CPPUNIT_TEST(splitBaggageText_removeEmptyLine);
  CPPUNIT_TEST(splitBaggageText_notRemoveEmptyLine);

  CPPUNIT_TEST(splitLineLen_noSplit);
  CPPUNIT_TEST(splitLineLen_splitFirst);
  CPPUNIT_TEST(splitLineLen_splitLast);
  CPPUNIT_TEST(splitLineLen_splitMiddle);
  CPPUNIT_TEST(splitLineLen_oneCharToLong);

  CPPUNIT_TEST(baggageResponseBuilder_initialize_sat_true);
  CPPUNIT_TEST(baggageResponseBuilder_initialize_sat_false);

  CPPUNIT_TEST(baggageResponseBuilder_findLastBaggageId_2pax);
  CPPUNIT_TEST(baggageResponseBuilder_findLastBaggageId_1pax);
  CPPUNIT_TEST(baggageResponseBuilder_findLastBaggageId_failed);

  CPPUNIT_TEST(baggageResponseBuilder_findCalcTotals_found);
  CPPUNIT_TEST(baggageResponseBuilder_findCalcTotals_notFound);
  CPPUNIT_TEST(baggageResponseBuilder_findCalcTotals_empty);

  CPPUNIT_TEST(baggageResponseBuilder_altPricing);
  CPPUNIT_TEST(baggageResponseBuilder_altPricing_empty);
  CPPUNIT_TEST(baggageResponseBuilder_altPricing_attn);
  CPPUNIT_TEST(baggageResponseBuilder_ticketing);
  CPPUNIT_TEST(baggageResponseBuilder_ticketing_empty);
  CPPUNIT_TEST(baggageResponseBuilder_ticketingADDonly_short);
  CPPUNIT_TEST(baggageResponseBuilder_ticketingADDonly_long);
  CPPUNIT_TEST(baggageResponseBuilder_greenScreenADT);
  CPPUNIT_TEST(baggageResponseBuilder_greenScreenATTN);
  CPPUNIT_TEST(baggageResponseBuilder_greenScreenMIL);
  CPPUNIT_TEST(baggageResponseBuilder_greenScreenNotFound);
  CPPUNIT_TEST(baggageResponseBuilder_greenScreenIncorrect);

  CPPUNIT_TEST(splitGreenScreenText_max204);
  CPPUNIT_TEST(splitGreenScreenText_toLongLine);
  CPPUNIT_TEST(testSetIsUsDotForMultiTkt_true_whenSingleTktIsUsDot);
  CPPUNIT_TEST(testSetIsUsDotForMultiTkt_false_whenSingleTktIsNotUsDot);
  CPPUNIT_TEST(testSetIsUsDotForMultiTkt_true_whenMultiTktIsUsDot);
  CPPUNIT_TEST(testSetIsUsDotForMultiTkt_false_whenMultiTktIsNotUsDot);

  CPPUNIT_TEST(testCheckElementONVwithPDORequested);
  CPPUNIT_TEST(testCheckElementONVwithPDORequested2);
  CPPUNIT_TEST(testCheckElementONVwithNoPDORequested);
  CPPUNIT_TEST(testCheckElementONVwithNoPDORequested2);
  CPPUNIT_TEST(testConstructElementONV);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  CalcTotals* _calcTotals;
  PricingTrx* _trx;
  Itin* _itin;
  FareCalcCollector::CalcTotalsMap* _calcTotalsMap;
  FareCalcCollector::BaggageTagMap* _baggageTagMap;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _calcTotals = _memHandle.create<CalcTotals>();
    _calcTotals->farePath = _memHandle.create<FarePath>();
    _trx = _memHandle.create<PricingTrx>();
    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->billing() = _memHandle.create<Billing>();
    _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();
    _trx->fareCalcConfig()->truePsgrTypeInd() = FareCalcConsts::FC_YES;
    _trx->itin().front()->setBaggageTripType(BaggageTripType::OTHER);

    _calcTotalsMap = _memHandle.create<FareCalcCollector::CalcTotalsMap>();
    _baggageTagMap = _memHandle.create<FareCalcCollector::BaggageTagMap>();

  }

  void tearDown() { _memHandle.clear(); }

  void baggageResponseBuilder_initialize_sat_true()
  {
    _trx->getRequest()->setSpecificAgencyText(true);

    BaggageResponseBuilder builder(*_trx, *_calcTotals);

    CPPUNIT_ASSERT_EQUAL(true, builder._useShortFooterMsg);
  }

  void baggageResponseBuilder_initialize_sat_false()
  {
    _trx->getRequest()->setSpecificAgencyText(false);

    BaggageResponseBuilder builder(*_trx, *_calcTotals);

    CPPUNIT_ASSERT_EQUAL(false, builder._useShortFooterMsg);
  }

  void testFormatFc_remove()
  {
    std::string S84 = "BASE FARE      EQUIV AMT      TAXES             TOTAL\nBAGGAGETEXTMIL\n 1-  "
                 "PLN41119.00    USD12870.00     356.90XT     USD13226.90ADT\n";
    std::string expected = "BASE FARE      EQUIV AMT      TAXES             TOTAL\\n 1-  PLN41119.00    "
                      "USD12870.00     356.90XT     USD13226.90ADT\\n";

    XformMsgFormatter::formatFc(
        FreeBaggageUtil::BaggageTagHead, FreeBaggageUtil::BaggageTagTotalSize + 1, "", S84);
    CPPUNIT_ASSERT_EQUAL(expected, S84);
  }

  void testFormatFc_Add()
  {
    std::string S84 = "BASE FARE      EQUIV AMT      TAXES             TOTAL\nBAGGAGETEXTMIL\n 1-  "
                 "PLN41119.00    USD12870.00     356.90XT     USD13226.90ADT\n";
    std::string expected = "BASE FARE      EQUIV AMT      TAXES             TOTAL\\nEMBARGOES-APPLY TO "
                      "EACH PASSENGER\\nFRANRT-LH\\nTHIRD BAG NOT PERMITTED\\n 1-  PLN41119.00    "
                      "USD12870.00     356.90XT     USD13226.90ADT\\n";
    std::string baggage = "EMBARGOES-APPLY TO EACH PASSENGER\nFRANRT-LH\nTHIRD BAG NOT PERMITTED\n";

    XformMsgFormatter::formatFc(
        FreeBaggageUtil::BaggageTagHead, FreeBaggageUtil::BaggageTagTotalSize + 1, baggage, S84);
    CPPUNIT_ASSERT_EQUAL(expected, S84);
  }

  void testFormatFc_AddLast()
  {
    std::string S84 = "BASE FARE      EQUIV AMT      TAXES             TOTAL\nBAGGAGETEXTMIL\n";
    std::string expected = "BASE FARE      EQUIV AMT      TAXES             TOTAL\\nEMBARGOES-APPLY TO "
                      "EACH PASSENGER\\nFRANRT-LH\\nTHIRD BAG NOT PERMITTED\\n";
    std::string baggage = "EMBARGOES-APPLY TO EACH PASSENGER\nFRANRT-LH\nTHIRD BAG NOT PERMITTED\n";

    XformMsgFormatter::formatFc(
        FreeBaggageUtil::BaggageTagHead, FreeBaggageUtil::BaggageTagTotalSize + 1, baggage, S84);
    CPPUNIT_ASSERT_EQUAL(expected, S84);
  }

  void testFormatFc_AddFailed()
  {
    std::string S84 = "BASE FARE      EQUIV AMT      TAXES             TOTAL\nBAGGAGETEXTMIL";
    std::string expected = "BASE FARE      EQUIV AMT      TAXES             TOTAL\\nBAGGAGETEXTMIL";

    XformMsgFormatter::formatFc(
        FreeBaggageUtil::BaggageTagHead, FreeBaggageUtil::BaggageTagTotalSize + 1, "", S84);
    CPPUNIT_ASSERT_EQUAL(expected, S84);
  }

  void testFormatFc_NoBaggage()
  {
    std::string S84 = "BASE FARE      EQUIV AMT      TAXES             TOTAL\n 1-  PLN41119.00    "
                 "USD12870.00     356.90XT     USD13226.90ADT\n";
    std::string expected = "BASE FARE      EQUIV AMT      TAXES             TOTAL\\n 1-  PLN41119.00    "
                      "USD12870.00     356.90XT     USD13226.90ADT\\n";

    XformMsgFormatter::formatFc(
        FreeBaggageUtil::BaggageTagHead, FreeBaggageUtil::BaggageTagTotalSize + 1, "", S84);
    CPPUNIT_ASSERT_EQUAL(expected, S84);
  }

  void splitNewLine_onlySplit()
  {
    std::vector<std::string> lines;
    std::string baggage = "EMBARGOES-APPLY TO EACH PASSENGER\n FRANRT-LH\nTHIRD BAG NOT PERMITTED\n";
    std::string line1 = "EMBARGOES-APPLY TO EACH PASSENGER";
    std::string line2 = " FRANRT-LH";
    std::string line3 = "THIRD BAG NOT PERMITTED";

    XformMsgFormatter::splitNewLine(baggage, lines, false);
    CPPUNIT_ASSERT_EQUAL(3, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
  }

  void splitNewLine_trimToLeft()
  {
    std::vector<std::string> lines;
    std::string baggage = "EMBARGOES-APPLY TO EACH PASSENGER\n FRANRT-LH\nTHIRD BAG NOT PERMITTED\n \n";
    std::string line1 = "EMBARGOES-APPLY TO EACH PASSENGER";
    std::string line2 = "FRANRT-LH";
    std::string line3 = "THIRD BAG NOT PERMITTED";
    std::string line4 = " ";

    XformMsgFormatter::splitNewLine(baggage, lines, true);
    CPPUNIT_ASSERT_EQUAL(4, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
  }

  void splitBaggageText_removeEmptyLine()
  {
    std::vector<std::string> lines;
    std::string baggage = "EMBARGOES-APPLY TO EACH PASSENGER\n FRANRT-LH\nTHIRD BAG NOT PERMITTED\n \n";
    std::string line1 = "EMBARGOES-APPLY TO EACH PASSENGER";
    std::string line2 = "FRANRT-LH";
    std::string line3 = "THIRD BAG NOT PERMITTED";

    XformMsgFormatter::splitBaggageText(baggage, 63, lines);
    CPPUNIT_ASSERT_EQUAL(3, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
  }

  void splitBaggageText_notRemoveEmptyLine()
  {
    std::vector<std::string> lines;
    std::string baggage = "EMBARGOES-APPLY TO EACH PASSENGER\n FRANRT-LH\nTHIRD BAG NOT PERMITTED\n \n";
    std::string line1 = "EMBARGOES-APPLY TO EACH PASSENGER";
    std::string line2 = "FRANRT-LH";
    std::string line3 = "THIRD BAG NOT PERMITTED";
    std::string line4 = " ";

    XformMsgFormatter::splitBaggageText(baggage, 63, lines, true);
    CPPUNIT_ASSERT_EQUAL(4, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
  }

  void splitLineLen_noSplit()
  {
    std::vector<std::string> lines;
    lines.push_back("FRANRT-LH");
    lines.push_back("THIRD BAG NOT PERMITTED");

    std::string line1 = "FRANRT-LH";
    std::string line2 = "THIRD BAG NOT PERMITTED";

    XformMsgFormatter::splitLineLen(23, lines);
    CPPUNIT_ASSERT_EQUAL(2, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
  }

  void splitLineLen_splitFirst()
  {
    std::vector<std::string> lines;
    lines.push_back("EMBARGOES-APPLY TO EACH PASSENGER");
    lines.push_back("FRANRT-LH");
    lines.push_back("THIRD BAG NOT PERMITTED");

    std::string line1 = "EMBARGOES-APPLY TO EACH";
    std::string line2 = "PASSENGER";
    std::string line3 = "FRANRT-LH";
    std::string line4 = "THIRD BAG NOT PERMITTED";

    XformMsgFormatter::splitLineLen(23, lines);
    CPPUNIT_ASSERT_EQUAL(4, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
  }

  void splitLineLen_splitLast()
  {
    std::vector<std::string> lines;
    lines.push_back("FRANRT-LH");
    lines.push_back("THIRD BAG NOT PERMITTED");
    lines.push_back("EMBARGOES-APPLY TO EACH PASSENGER");

    std::string line1 = "FRANRT-LH";
    std::string line2 = "THIRD BAG NOT PERMITTED";
    std::string line3 = "EMBARGOES-APPLY TO EACH";
    std::string line4 = "PASSENGER";

    XformMsgFormatter::splitLineLen(23, lines);
    CPPUNIT_ASSERT_EQUAL(4, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
  }

  void splitLineLen_splitMiddle()
  {
    std::vector<std::string> lines;
    lines.push_back("FRANRT-LH");
    lines.push_back("EMBARGOES-APPLY TO EACH PASSENGER");
    lines.push_back("THIRD BAG NOT PERMITTED");

    std::string line1 = "FRANRT-LH";
    std::string line2 = "EMBARGOES-APPLY TO EACH";
    std::string line3 = "PASSENGER";
    std::string line4 = "THIRD BAG NOT PERMITTED";

    XformMsgFormatter::splitLineLen(23, lines);
    CPPUNIT_ASSERT_EQUAL(4, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
  }

  void splitLineLen_oneCharToLong()
  {
    std::vector<std::string> lines;
    lines.push_back("EMBARGOES-APPLY TO EACH ");

    std::string line1 = "EMBARGOES-APPLY TO EACH";
    std::string line2 = "";

    XformMsgFormatter::splitLineLen(23, lines);
    CPPUNIT_ASSERT_EQUAL(2, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
  }

  void splitGreenScreenText_max204()
  {
    std::string response =
        "ENDOS*VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14\n FRANRT-LH\n ";
    std::string line1 = "ENDOS*VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14";
    std::string line2 = " FRANRT-LH";
    std::string line3 = " ";

    std::vector<std::string> lines;
    XformMsgFormatter::splitGreenScreeText(response, 204, lines);
    CPPUNIT_ASSERT_EQUAL(3, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
  }

  void splitGreenScreenText_toLongLine()
  {
    std::string response =
        "ENDOS*VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14\n FRANRT-LH";
    std::string line1 = " FRANRT-LH";

    std::vector<std::string> lines;
    XformMsgFormatter::splitGreenScreeText(response, 63, lines);
    CPPUNIT_ASSERT_EQUAL(1, (int)lines.size());
    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
  }

  void baggageResponseBuilder_findLastBaggageId_2pax()
  {
    BaggageResponseBuilder builder(*_trx, *_calcTotals);
    std::string line = "BASE FARE  \nBAGGAGETEXTMIL\n 1-  PLN41119.00    USD12870.00     356.90XT     "
                  "USD13226.90ADT\nBAGGAGETEXTADT";
    std::string expected = "BAGGAGETEXTADT";

    std::string pax = builder.findLastBaggageId(line);
    CPPUNIT_ASSERT_EQUAL(expected, pax);
  }

  void baggageResponseBuilder_findLastBaggageId_1pax()
  {
    BaggageResponseBuilder builder(*_trx, *_calcTotals);
    std::string line = "BASE FARE  \nBAGGAGETEXTMIL\n 1-  PLN41119.00    USD12870.00     356.90XT     "
                  "USD13226.90ADT\n";
    std::string expected = "BAGGAGETEXTMIL";

    std::string pax = builder.findLastBaggageId(line);
    CPPUNIT_ASSERT_EQUAL(expected, pax);
  }

  void baggageResponseBuilder_findLastBaggageId_failed()
  {
    BaggageResponseBuilder builder(*_trx, *_calcTotals);
    std::string line = "BASE FARE  \nBAGGAGETEXTMIL\n 1-  PLN41119.00    USD12870.00     356.90XT     "
                  "USD13226.90ADT\nBAGGAGETEXTAD";
    std::string expected = "";

    std::string pax = builder.findLastBaggageId(line);
    CPPUNIT_ASSERT_EQUAL(expected, pax);
  }

  CalcTotals* buildCalcTotals(PaxTypeCode paxType,
                              std::string baggageResponse = "",
                              std::string baggageEmbargoesResponse = "")
  {
    CalcTotals* calcTotals = _memHandle.create<CalcTotals>();
    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->paxType() = _memHandle.create<PaxType>();
    farePath->paxType()->paxType() = paxType;
    farePath->paxType()->number() = 1;

    calcTotals->truePaxType = paxType;
    calcTotals->requestedPaxType = "INF";

    if (!baggageResponse.empty())
      farePath->baggageResponse() = baggageResponse;

    if (!baggageEmbargoesResponse.empty())
      farePath->baggageEmbargoesResponse() = baggageEmbargoesResponse;

    calcTotals->farePath = farePath;

    return calcTotals;
  }

  void baggageResponseBuilder_findCalcTotals_found()
  {
    CalcTotals* calcTotalsADT = buildCalcTotals("ADT");
    CalcTotals* calcTotalsMIL = buildCalcTotals("MIL");

    _baggageTagMap->insert(std::make_pair("BAGGAGETEXT001", calcTotalsADT));
    _baggageTagMap->insert(std::make_pair("BAGGAGETEXT002", calcTotalsMIL));

    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, "");
    const CalcTotals* foundCalcTotals = builder.findCalcTotals("BAGGAGETEXT001");

    CPPUNIT_ASSERT_EQUAL((const CalcTotals*)calcTotalsADT, (const CalcTotals*)foundCalcTotals);
  }

  void baggageResponseBuilder_findCalcTotals_notFound()
  {
    CalcTotals* calcTotalsADT = buildCalcTotals("ADT");
    CalcTotals* calcTotalsMIL = buildCalcTotals("MIL");

    _baggageTagMap->insert(std::make_pair("BAGGAGETEXT001", calcTotalsADT));
    _baggageTagMap->insert(std::make_pair("BAGGAGETEXT002", calcTotalsMIL));

    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, "");
    const CalcTotals* foundCalcTotals = builder.findCalcTotals("BAGGAGETEXT003");

    CPPUNIT_ASSERT((0 == foundCalcTotals));
  }

  void baggageResponseBuilder_findCalcTotals_empty()
  {
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, "");
    const CalcTotals* foundCalcTotals = builder.findCalcTotals("BAGGAGETEXT001");

    CPPUNIT_ASSERT((0 == foundCalcTotals));
  }

  void baggageResponseBuilder_ticketing()
  {
    std::vector<std::string> lines;
    std::string baggage = "CARRY ON ALLOWANCE\n \nMIADCA-02P\n01/UP TO 45 LINEAR INCHES/115 LINEAR "
                     "CENTIMETERS\n<ADD>";

    CalcTotals* calcTotalsADT = buildCalcTotals("ADT", baggage);
    BaggageResponseBuilder builder(*_trx, *calcTotalsADT);
    builder.getPqResponse(lines);

    CPPUNIT_ASSERT_EQUAL(6, (int)lines.size());

    std::string line1 = "CARRY ON ALLOWANCE";
    std::string line2 = "MIADCA-02P";
    std::string line3 = "01/UP TO 45 LINEAR INCHES/115 LINEAR CENTIMETERS";
    std::string line4 = "ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY DEPENDING ON";
    std::string line5 = "FLYER-SPECIFIC FACTORS /E.G. FREQUENT FLYER STATUS/MILITARY/";
    std::string line6 = "CREDIT CARD FORM OF PAYMENT/EARLY PURCHASE OVER INTERNET,ETC./";

    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
    CPPUNIT_ASSERT_EQUAL(line5, lines[4]);
    CPPUNIT_ASSERT_EQUAL(line6, lines[5]);
  }

  void baggageResponseBuilder_ticketingADDonly_short()
  {
    std::vector<std::string> lines;
    std::string baggage = "<ADD>";

    _trx->getRequest()->setSpecificAgencyText(true);

    CalcTotals* calcTotalsADT = buildCalcTotals("ADT", baggage);
    BaggageResponseBuilder builder(*_trx, *calcTotalsADT);
    builder.getPqResponse(lines);

    CPPUNIT_ASSERT_EQUAL(1, (int)lines.size());

    std::string line1 = "ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY";

    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
  }

  void baggageResponseBuilder_ticketingADDonly_long()
  {
    std::vector<std::string> lines;
    std::string baggage = "<ADD>";

    _trx->getRequest()->setSpecificAgencyText(false);

    CalcTotals* calcTotalsADT = buildCalcTotals("ADT", baggage);
    BaggageResponseBuilder builder(*_trx, *calcTotalsADT);
    builder.getPqResponse(lines);

    CPPUNIT_ASSERT_EQUAL(3, (int)lines.size());

    std::string line1 = "ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY DEPENDING ON";
    std::string line2 = "FLYER-SPECIFIC FACTORS /E.G. FREQUENT FLYER STATUS/MILITARY/";
    std::string line3 = "CREDIT CARD FORM OF PAYMENT/EARLY PURCHASE OVER INTERNET,ETC./";

    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
  }

  void baggageResponseBuilder_ticketing_empty()
  {
    std::vector<std::string> lines;
    CalcTotals* calcTotalsADT = buildCalcTotals("ADT");
    BaggageResponseBuilder builder(*_trx, *calcTotalsADT);
    builder.getPqResponse(lines);

    CPPUNIT_ASSERT_EQUAL(0, (int)lines.size());
  }

  std::string greenScreenSetup()
  {
    std::string baggageMIL =
        "CARRY ON ALLOWANCE\n \nMIADCA-02P\n01/UP TO 45 LINEAR INCHES/115 LINEAR "
        "CENTIMETERS\n<ADD>";
    std::string baggageADT = "CARRY ON ALLOWANCE\nMIADCA-02P\n";
    std::string embargoes = "EMBARGOES-APPLY TO EACH PASSENGER\n FRANRT-LH\n";

    CalcTotals* calcTotalsADT = buildCalcTotals("ADT", baggageADT, embargoes);
    CalcTotals* calcTotalsMIL = buildCalcTotals("MIL", baggageMIL, embargoes);

    _baggageTagMap->insert(std::make_pair("BAGGAGETEXT001", calcTotalsADT));
    _baggageTagMap->insert(std::make_pair("BAGGAGETEXT002", calcTotalsMIL));

    return "BASE FARE  \nBAGGAGETEXT002\n 1-  PLN41119.00    USD12870.00     356.90XT     "
           "USD13226.90ADT\nBAGGAGETEXT001";
  }

  void baggageResponseBuilder_greenScreenMIL()
  {
    std::string fareCalcResponse = greenScreenSetup();

    std::vector<std::string> lines;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, fareCalcResponse);
    builder.setBaggageTag("BAGGAGETEXT002");
    builder.getGsResponse(lines);

    // Passanger info + 5 line
    CPPUNIT_ASSERT_EQUAL(6, (int)lines.size());

    std::string line1 = "MIL-01";
    std::string line2 = "CARRY ON ALLOWANCE";
    std::string line3 = " ";
    std::string line4 = "MIADCA-02P";
    std::string line5 = "01/UP TO 45 LINEAR INCHES/115 LINEAR CENTIMETERS";
    std::string line6 = "ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY";

    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
    CPPUNIT_ASSERT_EQUAL(line5, lines[4]);
    CPPUNIT_ASSERT_EQUAL(line6, lines[5]);
  }

  void baggageResponseBuilder_greenScreenADT()
  {
    std::string fareCalcResponse = greenScreenSetup();

    std::vector<std::string> lines;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, fareCalcResponse);
    builder.setBaggageTag("BAGGAGETEXT001");
    builder.getGsResponse(lines);

    // Passanger info + 2 line baggage + 2 line embargoes
    CPPUNIT_ASSERT_EQUAL(5, (int)lines.size());

    std::string line1 = "ADT-01";
    std::string line2 = "CARRY ON ALLOWANCE";
    std::string line3 = "MIADCA-02P";
    std::string line4 = "EMBARGOES-APPLY TO EACH PASSENGER";
    std::string line5 = "FRANRT-LH";

    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
    CPPUNIT_ASSERT_EQUAL(line5, lines[4]);
  }

  void baggageResponseBuilder_greenScreenATTN()
  {
    std::string fareCalcResponse = greenScreenSetup();
    _trx->fareCalcConfig()->warningMessages() = FareCalcConsts::FC_YES;

    std::vector<std::string> lines;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, fareCalcResponse);
    builder.setBaggageTag("BAGGAGETEXT001");
    builder.getGsResponse(lines);

    // Passanger info + 2 line baggage + 2 line embargoes
    CPPUNIT_ASSERT_EQUAL(5, (int)lines.size());

    std::string line1 = "ATTN*ADT-01";
    std::string line2 = "ATTN*CARRY ON ALLOWANCE";
    std::string line3 = "ATTN*MIADCA-02P";
    std::string line4 = "ATTN*EMBARGOES-APPLY TO EACH PASSENGER";
    std::string line5 = "ATTN*FRANRT-LH";

    CPPUNIT_ASSERT_EQUAL(line1, lines[0]);
    CPPUNIT_ASSERT_EQUAL(line2, lines[1]);
    CPPUNIT_ASSERT_EQUAL(line3, lines[2]);
    CPPUNIT_ASSERT_EQUAL(line4, lines[3]);
    CPPUNIT_ASSERT_EQUAL(line5, lines[4]);
  }

  void baggageResponseBuilder_greenScreenNotFound()
  {
    std::string fareCalcResponse = greenScreenSetup();

    std::vector<std::string> lines;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, fareCalcResponse);
    builder.setBaggageTag("BAGGAGETEXT005");
    CPPUNIT_ASSERT(!builder.getGsResponse(lines));
    CPPUNIT_ASSERT_EQUAL(0, (int)lines.size());
  }

  void baggageResponseBuilder_greenScreenIncorrect()
  {
    std::string fareCalcResponse = greenScreenSetup();

    std::vector<std::string> lines;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, fareCalcResponse);
    builder.setBaggageTag("BAGGAGETEXT00");
    CPPUNIT_ASSERT(!builder.getGsResponse(lines));
    CPPUNIT_ASSERT_EQUAL(0, (int)lines.size());
  }

  void baggageResponseBuilder_altPricing()
  {
    std::string baggage = "CARRY ON ALLOWANCE\n \nMIADCA-02P\n<ADD>";
    std::string baggageEmbargoes = "EMBARGOES-APPLY TO EACH PASSENGER\nFRANRT-LH\n";

    std::string baggageResponse;
    CalcTotals* calcTotalsADT = buildCalcTotals("ADT", baggage, baggageEmbargoes);
    BaggageResponseBuilder builder(*_trx, *calcTotalsADT);
    builder.getWpaGsResponse(baggageResponse);

    std::string expected = "CARRY ON ALLOWANCE\\n \\nMIADCA-02P\\nADDITIONAL ALLOWANCES AND/OR "
                      "DISCOUNTS MAY APPLY\\nEMBARGOES-APPLY TO EACH PASSENGER\\nFRANRT-LH\\n";

    CPPUNIT_ASSERT_EQUAL(expected, baggageResponse);
  }

  void baggageResponseBuilder_altPricing_empty()
  {
    std::string baggageResponse;
    CalcTotals* calcTotalsADT = buildCalcTotals("ADT");
    BaggageResponseBuilder builder(*_trx, *calcTotalsADT);
    builder.getWpaGsResponse(baggageResponse);

    std::string expected;
    CPPUNIT_ASSERT_EQUAL(expected, baggageResponse);
  }

  void baggageResponseBuilder_altPricing_attn()
  {
    _trx->fareCalcConfig()->warningMessages() = FareCalcConsts::FC_YES;

    std::string baggage = "CARRY ON ALLOWANCE\n \nMIADCA-02P\n<ADD>";
    std::string baggageEmbargoes = "EMBARGOES-APPLY TO EACH PASSENGER\nFRANRT-LH\n";

    std::string baggageResponse;
    CalcTotals* calcTotalsADT = buildCalcTotals("ADT", baggage, baggageEmbargoes);
    BaggageResponseBuilder builder(*_trx, *calcTotalsADT);
    builder.getWpaGsResponse(baggageResponse);

    std::string expected = "ATTN*CARRY ON ALLOWANCE\\n \\nATTN*MIADCA-02P\\nATTN*ADDITIONAL ALLOWANCES "
                      "AND/OR DISCOUNTS MAY APPLY\\nATTN*EMBARGOES-APPLY TO EACH "
                      "PASSENGER\\nATTN*FRANRT-LH\\n";
    CPPUNIT_ASSERT_EQUAL(expected, baggageResponse);
  }

  void testSetIsUsDotForMultiTkt_true_whenSingleTktIsUsDot()
  {
    _trx->itin().front()->setBaggageTripType(BaggageTripType::TO_FROM_US); //singleTkt is UsDot
    _trx->itin().front()->ticketSolution() = MultiTicketUtil::SINGLETKT_LOWER_THAN_MULTITKT;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, "");
    builder.setIsUsDotForMultiTkt(*_trx, _trx->itin().front());
    CPPUNIT_ASSERT(builder._isUsDot);
  }

  void testSetIsUsDotForMultiTkt_false_whenSingleTktIsNotUsDot()
  {
    _trx->itin().front()->ticketSolution() = MultiTicketUtil::SINGLETKT_LOWER_THAN_MULTITKT;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, "");
    builder.setIsUsDotForMultiTkt(*_trx, _trx->itin().front());
    CPPUNIT_ASSERT(!builder._isUsDot);
  }

  void testSetIsUsDotForMultiTkt_true_whenMultiTktIsUsDot()
  {
    _trx->itin().front()->ticketSolution() = MultiTicketUtil::SINGLETKT_NOT_APPLICABLE;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, "");
    CPPUNIT_ASSERT(!builder._isUsDot); //singleTkt is nonUsDot
    _itin->setBaggageTripType(BaggageTripType::TO_FROM_US); //multiTkt is UsDot
    builder.setIsUsDotForMultiTkt(*_trx, _itin);
    CPPUNIT_ASSERT(builder._isUsDot);
  }

  void testSetIsUsDotForMultiTkt_false_whenMultiTktIsNotUsDot()
  {
    _trx->itin().front()->setBaggageTripType(BaggageTripType::TO_FROM_US); //singleTkt is UsDot
    _trx->itin().front()->ticketSolution() = MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT;
    BaggageResponseBuilder builder(*_trx, *_baggageTagMap, "");
    CPPUNIT_ASSERT(builder._isUsDot); //singleTkt is UsDot
    _itin->setBaggageTripType(BaggageTripType::OTHER); //multiTkt is nonUsDot
    builder.setIsUsDotForMultiTkt(*_trx, _itin);
    CPPUNIT_ASSERT(!builder._isUsDot);
  }

  void testCheckElementONVwithPDORequested()
  {
    FareCalcCollector* fareCalcCollector = _memHandle.create<FareCalcCollector>();
    FarePath farePath;
    _calcTotals->farePath = &farePath;
    CalcTotals calcTotalsADJ;
    _calcTotals->adjustedCalcTotal = &calcTotalsADJ;
    fareCalcCollector->passengerCalcTotals().push_back(_calcTotals);
    PricingOptions options;
    options.setPDOForFRRule(true);
    _trx->setOptions(&options);
    XMLConstruct xml;
    MAFUtil::checkElementONV(*_trx, *fareCalcCollector, xml);
    CPPUNIT_ASSERT(xml.getXMLData().find("ONV") == std::string::npos);
  }

  void testCheckElementONVwithNoPDORequested()
  {
    FareCalcCollector* fareCalcCollector = _memHandle.create<FareCalcCollector>();
    FarePath farePath;
    _calcTotals->farePath = &farePath;
    CalcTotals calcTotalsADJ;
    _calcTotals->adjustedCalcTotal = &calcTotalsADJ;
    fareCalcCollector->passengerCalcTotals().push_back(_calcTotals);
    PricingOptions options;
    options.setPDOForFRRule(false);
    _trx->setOptions(&options);
    XMLConstruct xml;
    MAFUtil::checkElementONV(*_trx, *fareCalcCollector, xml);
    CPPUNIT_ASSERT(xml.getXMLData().find("ONV") != std::string::npos);
  }

  void testCheckElementONVwithPDORequested2()
  {
    FarePath farePath;
    _calcTotals->farePath = &farePath;
    CalcTotals calcTotalsADJ;
    _calcTotals->adjustedCalcTotal = &calcTotalsADJ;
    PricingOptions options;
    options.setPDOForFRRule(true);
    _trx->setOptions(&options);
    XMLConstruct xml;
    MAFUtil::checkElementONV(*_trx, _calcTotals, xml);
    CPPUNIT_ASSERT(xml.getXMLData().find("ONV") == std::string::npos);
  }

  void testCheckElementONVwithNoPDORequested2()
  {
    FarePath farePath;
    _calcTotals->farePath = &farePath;
    CalcTotals calcTotalsADJ;
    _calcTotals->adjustedCalcTotal = &calcTotalsADJ;
    PricingOptions options;
    options.setPDOForFRRule(false);
    _trx->setOptions(&options);
    XMLConstruct xml;
    MAFUtil::checkElementONV(*_trx, _calcTotals, xml);
    CPPUNIT_ASSERT(xml.getXMLData().find("ONV") != std::string::npos);
  }

  void testConstructElementONV()
  {
    XMLConstruct xml;
    MAFUtil::constructElementONV(xml);
    CPPUNIT_ASSERT(xml.getXMLData().find("ONV ADJ=\"T\"") != std::string::npos);
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(XformUtilTest);
}
