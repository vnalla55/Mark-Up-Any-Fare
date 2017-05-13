#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag535Collector.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diagnostic.h"
#include "Rules/RuleConst.h"
#include "DBAccess/NegFareRest.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/ValueCodeAlgorithm.h"
#include "Rules/TicketingEndorsement.h"
#include "test/include/TestConfigInitializer.h"

#include "Common/Config/ConfigMan.h"

namespace tse
{
class Diag535CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag535CollectorTest);
  CPPUNIT_TEST(testShouldDisplayTFDSPCIndReturnFalseWhenByte101Exists);
  CPPUNIT_TEST(testShouldDisplayTFDSPCIndReturnFalseWhenNoTFDSPCExt);
  CPPUNIT_TEST(testShouldDisplayTFDSPCIndReturnFalseWhenNoIndAndNoDataSeg);
  CPPUNIT_TEST(testShouldDisplayTFDSPCIndReturnTrueWhenFareBasisAmtIndExists);
  CPPUNIT_TEST(testShouldDisplayTFDSPCIndReturnTrueWhenDataSegExists);
  CPPUNIT_TEST(testTktFareDataSegExistsWhenNoTFDSPCExt);
  CPPUNIT_TEST(testTktFareDataSegExistsWhenNoDataSeg);
  CPPUNIT_TEST(testTktFareDataSegExistsWhenPass);
  CPPUNIT_TEST(testDisplayFareBasisAmountInd);
  CPPUNIT_TEST(testDisplayValueCodeAlgorithmWithSpace);
  CPPUNIT_TEST(testDisplayValueCodeAlgorithmWithoutSpace);
  CPPUNIT_TEST(testGetCat18ValueCode_Pass);
  CPPUNIT_TEST(testGetCat18ValueCode_Fail);
  CPPUNIT_TEST(testDisplayStaticValueCode_None_Blank);
  CPPUNIT_TEST(testDisplayStaticValueCode_C18_1st);
  CPPUNIT_TEST(testDisplayStaticValueCode_C35_1st);
  CPPUNIT_TEST(testDisplayStaticValueCode_C35_2nd);
  CPPUNIT_TEST(testDisplayStaticValueCode_Both_All);
  CPPUNIT_TEST(testDisplayTourCodeDisplayOption_1st);
  CPPUNIT_TEST(testDisplayTourCodeDisplayOption_2nd);
  CPPUNIT_TEST(testDisplayTourCodeDisplayOption_All);
  CPPUNIT_TEST(testDisplayMatchedSequencesResultWhenNoExtSeq);
  CPPUNIT_TEST(testDisplayMatchedSequencesResultWhenNoUnfbc);
  CPPUNIT_TEST(testDisplayMatchedSequencesResultWhenUnfbcExists);
  CPPUNIT_TEST(testDiag535Request);
  CPPUNIT_TEST(testDiag535Request_WithCurrencyOverride);
  CPPUNIT_TEST(testDisplayCoupons);
  CPPUNIT_TEST(testDisplayCoupons_WithAuditCoupon);
  CPPUNIT_TEST(testDisplayCoupons_WithPsgCoupon);
  CPPUNIT_TEST(testDisplayCoupons_WithNRRMethod2);
  CPPUNIT_TEST(testDisplayCoupons_WithNRRMethod3);
  CPPUNIT_TEST(testDisplayCoupons_WithFareAmt1Set);
  CPPUNIT_TEST(testDisplayCoupons_WithFareAmt1Set_NoCurrency);
  CPPUNIT_TEST(testDisplayCoupons_WithFareAmt1Zero);
  CPPUNIT_TEST(testDisplayCoupons_With2Segs);
  CPPUNIT_TEST(testDisplayCoupons_With2SegsAndWithAuditCoupon);
  CPPUNIT_TEST(testDisplayCoupons_With2SegsAndPsgCoupon);
  CPPUNIT_TEST(testDisplayCoupons_With2SegsAndNRRMethod2);
  CPPUNIT_TEST(testDisplayCoupons_With2SegsAndFareAmt2Set);
  CPPUNIT_TEST(testDisplayCoupons_With2SegsAndFareAmt2Set_NoCurrency);
  CPPUNIT_TEST(testDisplayCoupons_With2SegsAndFareAmt2Zero);
  CPPUNIT_TEST(testDisplayHeader_PUScope);
  CPPUNIT_TEST(testDisplayHeader_FPScope);
  CPPUNIT_TEST(testDiag535Collector_NotNegotiated);
  CPPUNIT_TEST(testDiag535Collector_WrongCat35Data);
  CPPUNIT_TEST(testDiag535Collector_WrongRec3Data);
  CPPUNIT_TEST(testDiag535Collector_Negotiated);
  CPPUNIT_TEST(testDiag535Collector_PtfAsParameter);

  CPPUNIT_TEST(testDiag535Message_MultipleTourCode);
  CPPUNIT_TEST(testDiag535Message_TourCodeNotValid);
  CPPUNIT_TEST(testDiag535Message_MultipleValueCode);
  CPPUNIT_TEST(testDiag535Message_MultiplePrintOption);
  CPPUNIT_TEST(testDiag535Message_InvalidItbt);
  CPPUNIT_TEST(testDiag535Message_EmptyItbt);
  CPPUNIT_TEST(testDiag535Message_ItbtNotBlank);
  CPPUNIT_TEST(testDiag535Message_NotItbt);
  CPPUNIT_TEST(testDiag535Message_MultipleFareBox);
  CPPUNIT_TEST(testDiag535Message_MultipleBsp);
  CPPUNIT_TEST(testDiag535Message_MultipleNetGross);
  CPPUNIT_TEST(testDiag535Message_MixFares);
  CPPUNIT_TEST(testDiag535Message_FaresNotCombinable);
  CPPUNIT_TEST(testDiag535Message_NetSellingConflict);
  CPPUNIT_TEST(testDiag535Message_MultipleCommision);
  CPPUNIT_TEST(testDiag535Message_MixCommision);
  CPPUNIT_TEST(testDiag535Message_IssueSeparateTkt);
  CPPUNIT_TEST(testDiag535Message_InvalidNetRemit);
  CPPUNIT_TEST(testDiag535Message_NoNetAmount);
  CPPUNIT_TEST(testDiag535Message_NotCombinableNoNet);
  CPPUNIT_TEST(testDiag535Message_InvalidNetRemitComm);
  CPPUNIT_TEST(testDiag535Message_AutoTktNotPermitted);
  CPPUNIT_TEST(testDiag535Message_NetAmountExceedsFareAmount);
  CPPUNIT_TEST(testDiag535Message_MixedFareCombination);
  CPPUNIT_TEST(testDiag535Message_ConflictingTfdByte101);
  CPPUNIT_TEST(testDiag535Message_Tfd);
  CPPUNIT_TEST(testDiag535Message_MixedFareBoxAmount);
  CPPUNIT_TEST(testDiag535Message_Other);

  CPPUNIT_TEST(testDisplayPaxTypeFare_NotNegotiated);
  CPPUNIT_TEST(testDisplayPaxTypeFare_Negotiated);

  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx* _trx;
  Itin* _itin;
  Diag535Collector* _collector;
  Diagnostic* _diagRoot;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _itin = _memHandle.create<Itin>();
    _diagRoot = _memHandle.insert(new Diagnostic(Diagnostic535));
    _diagRoot->activate();
    _collector = _memHandle.insert(new Diag535Collector(*_diagRoot));
    _collector->enable(Diagnostic535);
    _memHandle.create<TestConfigInitializer>();
  }
  void tearDown()
  {
    _memHandle.clear();
  }
  NegPaxTypeFareRuleData* prepareTFDSPCData(Indicator tktFareDataSegExistInd = 'Y',
                                            Indicator fareBasisAmtInd = RuleConst::BLANK)
  {
    NegPaxTypeFareRuleData* rd = _memHandle.create<NegPaxTypeFareRuleData>();
    NegFareRestExt* negFareRestExt = _memHandle.create<NegFareRestExt>();
    rd->negFareRestExt() = negFareRestExt;
    negFareRestExt->tktFareDataSegExistInd() = tktFareDataSegExistInd;
    negFareRestExt->fareBasisAmtInd() = fareBasisAmtInd;
    return rd;
  }
  void testShouldDisplayTFDSPCIndReturnFalseWhenByte101Exists()
  {
    const NegPaxTypeFareRuleData* negPaxTypeFare = prepareTFDSPCData();
    Indicator tktFareDataInd1 = RuleConst::NR_VALUE_B;
    CPPUNIT_ASSERT(!_collector->shouldDisplayTFDSPCInd(negPaxTypeFare, tktFareDataInd1));
  }
  void testShouldDisplayTFDSPCIndReturnFalseWhenNoTFDSPCExt()
  {
    NegPaxTypeFareRuleData* negPaxTypeFare = prepareTFDSPCData();
    Indicator tktFareDataInd1 = RuleConst::BLANK;
    negPaxTypeFare->negFareRestExt() = 0;
    const NegPaxTypeFareRuleData* ruleData = negPaxTypeFare;
    CPPUNIT_ASSERT(!_collector->shouldDisplayTFDSPCInd(ruleData, tktFareDataInd1));
  }
  void testShouldDisplayTFDSPCIndReturnFalseWhenNoIndAndNoDataSeg()
  {
    NegPaxTypeFareRuleData* negPaxTypeFare = prepareTFDSPCData('N', RuleConst::BLANK);
    Indicator tktFareDataInd1 = RuleConst::BLANK;
    const NegPaxTypeFareRuleData* ruleData = negPaxTypeFare;
    CPPUNIT_ASSERT(!_collector->shouldDisplayTFDSPCInd(ruleData, tktFareDataInd1));
  }
  void testShouldDisplayTFDSPCIndReturnTrueWhenFareBasisAmtIndExists()
  {
    const NegPaxTypeFareRuleData* negPaxTypeFare = prepareTFDSPCData('N', RuleConst::NR_VALUE_B);
    Indicator tktFareDataInd1 = RuleConst::BLANK;
    CPPUNIT_ASSERT(_collector->shouldDisplayTFDSPCInd(negPaxTypeFare, tktFareDataInd1));
  }
  void testShouldDisplayTFDSPCIndReturnTrueWhenDataSegExists()
  {
    const NegPaxTypeFareRuleData* negPaxTypeFare = prepareTFDSPCData('Y', RuleConst::BLANK);
    Indicator tktFareDataInd1 = RuleConst::BLANK;
    CPPUNIT_ASSERT(_collector->shouldDisplayTFDSPCInd(negPaxTypeFare, tktFareDataInd1));
  }
  void testTktFareDataSegExistsWhenNoTFDSPCExt()
  {
    NegPaxTypeFareRuleData* negPaxTypeFare = prepareTFDSPCData();
    negPaxTypeFare->negFareRestExt() = 0;
    const NegPaxTypeFareRuleData* ruleData = negPaxTypeFare;
    CPPUNIT_ASSERT(!_collector->tktFareDataSegExists(ruleData));
  }
  void testTktFareDataSegExistsWhenNoDataSeg()
  {
    NegPaxTypeFareRuleData* negPaxTypeFare = prepareTFDSPCData('N');
    negPaxTypeFare->negFareRestExt() = 0;
    const NegPaxTypeFareRuleData* ruleData = negPaxTypeFare;
    CPPUNIT_ASSERT(!_collector->tktFareDataSegExists(ruleData));
  }
  void testTktFareDataSegExistsWhenPass()
  {
    NegPaxTypeFareRuleData* negPaxTypeFare = prepareTFDSPCData();
    const NegPaxTypeFareRuleData* ruleData = negPaxTypeFare;
    CPPUNIT_ASSERT(_collector->tktFareDataSegExists(ruleData));
  }
  void testDisplayFareBasisAmountInd()
  {
    NegFareRestExt negFareRestExt;
    negFareRestExt.fareBasisAmtInd() = RuleConst::NR_VALUE_B;
    _collector->displayFareBasisAmountInd(&negFareRestExt);
    CPPUNIT_ASSERT_EQUAL(std::string("  FARE BASIS/AMOUNT IND : B\n"), _collector->str());
  }
  void prepareDisplayValueCodeAlgorithmCase(ValueCodeAlgorithm& valCodeAlg)
  {
    valCodeAlg.prefix() = "QWE";
    valCodeAlg.suffix() = "RTY";
    valCodeAlg.decimalChar() = 'S';
    valCodeAlg.digitToChar()[0] = 'D';
    valCodeAlg.digitToChar()[1] = 'C';
    valCodeAlg.digitToChar()[2] = 'U';
    valCodeAlg.digitToChar()[3] = 'M';
    valCodeAlg.digitToChar()[4] = 'B';
    valCodeAlg.digitToChar()[5] = 'E';
    valCodeAlg.digitToChar()[6] = 'R';
    valCodeAlg.digitToChar()[7] = 'L';
    valCodeAlg.digitToChar()[8] = 'A';
    valCodeAlg.digitToChar()[9] = 'N';
    valCodeAlg.algorithmName() = "VALUECODE1";
  }
  void testDisplayValueCodeAlgorithmWithSpace()
  {
    ValueCodeAlgorithm valCodeAlg;
    prepareDisplayValueCodeAlgorithmCase(valCodeAlg);
    _collector->displayValueCodeAlgorithm(valCodeAlg, true);
    CPPUNIT_ASSERT_EQUAL(std::string("  VALUE CODE ALGORITHM NAME : VALUECODE1\n"
                                     "  PREFIX  SUFFIX  1  2  3  4  5  6  7  8  9  0  DECIMAL \n"
                                     "  QWE     RTY     C  U  M  B  E  R  L  A  N  D  S  \n"),
                         _collector->str());
  }
  void testDisplayValueCodeAlgorithmWithoutSpace()
  {
    ValueCodeAlgorithm valCodeAlg;
    prepareDisplayValueCodeAlgorithmCase(valCodeAlg);
    _collector->displayValueCodeAlgorithm(valCodeAlg, false);
    CPPUNIT_ASSERT_EQUAL(std::string(" VALUE CODE ALGORITHM NAME : VALUECODE1\n"
                                     " PREFIX  SUFFIX  1  2  3  4  5  6  7  8  9  0  DECIMAL \n"
                                     " QWE     RTY     C  U  M  B  E  R  L  A  N  D  S  \n"),
                         _collector->str());
  }
  FarePath* createFarePath()
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FareUsage* fu = _memHandle.create<FareUsage>();
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    FarePath* fp = _memHandle.create<FarePath>();
    fu->paxTypeFare() = ptf;
    pu->fareUsage().push_back(fu);
    fp->pricingUnit().push_back(pu);
    fp->itin() = _itin;
    return fp;
  }
  void testGetCat18ValueCode_Pass()
  {
    FarePath* fp = createFarePath();
    FareUsage* fu = fp->pricingUnit().front()->fareUsage().front();
    TicketEndorseItem te;
    te.tktLocInd = '1';
    te.endorsementTxt = "AAA";
    fu->tktEndorsement().push_back(te);
    CPPUNIT_ASSERT_EQUAL(std::string("AAA"),
                         Diag535Collector::getCat18ValueCode(fp, *fu->paxTypeFare()));
  }
  void testGetCat18ValueCode_Fail()
  {
    PaxTypeFare ptf;
    CPPUNIT_ASSERT_EQUAL(std::string(""), Diag535Collector::getCat18ValueCode(NULL, ptf));
  }
  void testDisplayStaticValueCode_None_Blank()
  {
    NegFareRestExt negFareRestExt;
    _collector->displayStaticValueCode(&negFareRestExt, "", true);
    CPPUNIT_ASSERT_EQUAL(std::string("  STATIC VALUE CODE :\n"
                                     "  STATIC VALUE CODE DISPLAY OPTION : \n"),
                         _collector->str());
  }
  void testDisplayStaticValueCode_C18_1st()
  {
    NegFareRestExt negFareRestExt;
    negFareRestExt.staticValueCodeCombInd() = '1';
    _collector->displayStaticValueCode(&negFareRestExt, "C18", true);
    CPPUNIT_ASSERT_EQUAL(std::string("  STATIC VALUE CODE : *C18* C18\n"
                                     "  STATIC VALUE CODE DISPLAY OPTION : 1ST\n"),
                         _collector->str());
  }
  void testDisplayStaticValueCode_C35_1st()
  {
    NegFareRestExt negFareRestExt;
    negFareRestExt.staticValueCode() = "C35";
    negFareRestExt.staticValueCodeCombInd() = '1';
    _collector->displayStaticValueCode(&negFareRestExt, "", true);
    CPPUNIT_ASSERT_EQUAL(std::string("  STATIC VALUE CODE : *C35* C35\n"
                                     "  STATIC VALUE CODE DISPLAY OPTION : 1ST\n"),
                         _collector->str());
  }
  void testDisplayStaticValueCode_C35_2nd()
  {
    NegFareRestExt negFareRestExt;
    negFareRestExt.staticValueCode() = "C35";
    negFareRestExt.staticValueCodeCombInd() = '2';
    _collector->displayStaticValueCode(&negFareRestExt, "", true);
    CPPUNIT_ASSERT_EQUAL(std::string("  STATIC VALUE CODE : *C35* C35\n"
                                     "  STATIC VALUE CODE DISPLAY OPTION : 2ND\n"),
                         _collector->str());
  }
  void testDisplayStaticValueCode_Both_All()
  {
    NegFareRestExt negFareRestExt;
    negFareRestExt.staticValueCode() = "C35";
    negFareRestExt.staticValueCodeCombInd() = 'B';
    _collector->displayStaticValueCode(&negFareRestExt, "C18", true);
    CPPUNIT_ASSERT_EQUAL(std::string("  STATIC VALUE CODE : *C35* C35 / *C18* C18\n"
                                     "  STATIC VALUE CODE DISPLAY OPTION : ALL\n"),
                         _collector->str());
  }
  void testDisplayTourCodeDisplayOption_1st()
  {
    NegFareRestExt negFareRestExt;
    negFareRestExt.tourCodeCombInd() = '1';
    _collector->displayTourCodeCombInd(&negFareRestExt, true);
    CPPUNIT_ASSERT_EQUAL(std::string("  TOUR CODE DISPLAY OPTION : 1ST\n"), _collector->str());
  }

  void testDisplayTourCodeDisplayOption_2nd()
  {
    NegFareRestExt negFareRestExt;
    negFareRestExt.tourCodeCombInd() = '2';
    _collector->displayTourCodeCombInd(&negFareRestExt, true);
    CPPUNIT_ASSERT_EQUAL(std::string("  TOUR CODE DISPLAY OPTION : 2ND\n"), _collector->str());
  }

  void testDisplayTourCodeDisplayOption_All()
  {
    NegFareRestExt negFareRestExt;
    negFareRestExt.tourCodeCombInd() = 'B';
    _collector->displayTourCodeCombInd(&negFareRestExt, true);
    CPPUNIT_ASSERT_EQUAL(std::string("  TOUR CODE DISPLAY OPTION : ALL\n"), _collector->str());
  }

  void testDisplayMatchedSequencesResultWhenNoExtSeq()
  {
    FareUsage fu;
    _collector->displayMatchedSequencesResult(fu);
    CPPUNIT_ASSERT_EQUAL(std::string("  **NO MATCHED SEQUENCES FOR TFDPSC/UNFBC**\n"),
                         _collector->str());
  }

  void testDisplayMatchedSequencesResultWhenNoUnfbc()
  {
    FareUsage* fu = createNetRemitPscResults(false);
    _collector->displayMatchedSequencesResult(*fu);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _collector->str());
  }

  void testDisplayMatchedSequencesResultWhenUnfbcExists()
  {
    FareUsage* fu = createNetRemitPscResults(true);
    _collector->displayMatchedSequencesResult(*fu);
    CPPUNIT_ASSERT_EQUAL(std::string("  MATCHED SEQUENCES FOR UNFBC: \n"
                                     "  SEQNO       1 \n"),
                         _collector->str());
  }

  FareUsage* createNetRemitPscResults(bool uniqueFareBasis)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    FareUsage::TktNetRemitPscResult* netRemitResult =
        _memHandle.create<FareUsage::TktNetRemitPscResult>();
    NegFareRestExtSeq* restExtSeq = _memHandle.create<NegFareRestExtSeq>();
    if (uniqueFareBasis)
    {
      restExtSeq->seqNo() = 1;
      restExtSeq->uniqueFareBasis() = "UNFBC";
    }
    netRemitResult->_tfdpscSeqNumber = restExtSeq;
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->segmentOrder() = 1;
    netRemitResult->_endTravelSeg = seg;
    netRemitResult->_startTravelSeg = seg;
    fu->netRemitPscResults().push_back(*netRemitResult);
    return fu;
  }

  void testDiag535Request()
  {
    _collector->diag535Request(*_trx, *createFarePath());
    CPPUNIT_ASSERT_EQUAL(
        std::string("************************************************************\n"
                    "*     ATSE REQUEST DATA FOR NEGOTIATED FARE RULE PROCESS   *\n"
                    "************************************************************\n"
                    " USER TYPE                       : \n"
                    " TICKETING ENTRY                 : N\n"
                    " CREDIT CARD FOP                 : N\n"
                    " PRINT SELLING CATEGORY 35 FARE  : N\n"
                    " PRINT NET CATEGORY 35 FARE      : N\n"
                    " PAYMENT CURRENCY                : \n"
                    " NET REMIT PROCESS               : N\n"),
        _collector->str());
  }

  void testDiag535Request_WithCurrencyOverride()
  {
    _trx->getOptions()->currencyOverride() = "CUR";
    _collector->diag535Request(*_trx, *createFarePath());
    CPPUNIT_ASSERT_EQUAL(
        std::string("************************************************************\n"
                    "*     ATSE REQUEST DATA FOR NEGOTIATED FARE RULE PROCESS   *\n"
                    "************************************************************\n"
                    " USER TYPE                       : \n"
                    " TICKETING ENTRY                 : N\n"
                    " CREDIT CARD FOP                 : N\n"
                    " PRINT SELLING CATEGORY 35 FARE  : N\n"
                    " PRINT NET CATEGORY 35 FARE      : N\n"
                    " PAYMENT CURRENCY                : CUR\n"
                    " NET REMIT PROCESS               : N\n"),
        _collector->str());
  }

  void testDisplayCoupons()
  {
    NegFareRest nfr;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT_EQUAL(std::string("     TOUR BOX TYPE    -  \n"
                                     "     TOUR BOX CODE    - \n"
                                     "     TKT DESIGNATOR   - \n"
                                     "     FARE BOX TEXT    - \n"
                                     "     FARE BOX CUR/AMT - \n"),
                         _collector->str());
  }

  void testDisplayCoupons_WithAuditCoupon()
  {
    NegFareRest nfr;
    nfr.couponInd1() = NegotiatedFareRuleUtil::AUDIT_COUPON;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("AUDIT COUPON"));
  }

  void testDisplayCoupons_WithPsgCoupon()
  {
    NegFareRest nfr;
    nfr.couponInd1() = NegotiatedFareRuleUtil::PSG_COUPON;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("PASSENGER COUPON"));
  }

  void testDisplayCoupons_WithNRRMethod2()
  {
    NegFareRest nfr;
    nfr.netRemitMethod() = RuleConst::NRR_METHOD_2;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("TICKETED FARE DATA"));
  }

  void testDisplayCoupons_WithNRRMethod3()
  {
    NegFareRest nfr;
    nfr.netRemitMethod() = RuleConst::NRR_METHOD_3;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("TICKETED FARE DATA"));
  }

  void testDisplayCoupons_WithFareAmt1Set()
  {
    NegFareRest nfr;
    nfr.fareAmt1() = 123.45;
    nfr.cur11() = "USD";
    nfr.noDec11() = 2;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("FARE BOX CUR/AMT - USD123.45"));
  }

  void testDisplayCoupons_WithFareAmt1Set_NoCurrency()
  {
    NegFareRest nfr;
    nfr.fareAmt1() = 123.45;
    nfr.cur11() = "";
    nfr.noDec11() = 2;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("FARE BOX CUR/AMT - \n"));
  }

  void testDisplayCoupons_WithFareAmt1Zero()
  {
    NegFareRest nfr;
    nfr.fareAmt1() = 0;
    nfr.cur11() = "USD";
    nfr.noDec11() = 2;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("FARE BOX CUR/AMT - USD0.00"));
  }

  void testDisplayCoupons_With2Segs()
  {
    NegFareRest nfr;
    nfr.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT_EQUAL(std::string("     TOUR BOX TYPE    -  \n"
                                     "     TOUR BOX CODE    - \n"
                                     "     TKT DESIGNATOR   - \n"
                                     "     FARE BOX TEXT    - \n"
                                     "     FARE BOX CUR/AMT - \n"
                                     "     TOUR BOX TYPE    -  \n"
                                     "     TOUR BOX CODE    - \n"
                                     "     TKT DESIGNATOR   - \n"
                                     "     FARE BOX TEXT    - \n"
                                     "     FARE BOX CUR/AMT - \n"),
                         _collector->str());
  }

  void testDisplayCoupons_With2SegsAndWithAuditCoupon()
  {
    NegFareRest nfr;
    nfr.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    nfr.couponInd2() = NegotiatedFareRuleUtil::AUDIT_COUPON;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("AUDIT COUPON"));
  }

  void testDisplayCoupons_With2SegsAndPsgCoupon()
  {
    NegFareRest nfr;
    nfr.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    nfr.couponInd2() = NegotiatedFareRuleUtil::PSG_COUPON;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("PASSENGER COUPON"));
  }

  void testDisplayCoupons_With2SegsAndNRRMethod2()
  {
    NegFareRest nfr;
    nfr.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    nfr.netRemitMethod() = RuleConst::NRR_METHOD_2;
    _collector->displayCoupons(*_trx, nfr);
    size_t firstInstance = _collector->str().find("TICKETED FARE DATA");
    CPPUNIT_ASSERT(std::string::npos != firstInstance);
    CPPUNIT_ASSERT(std::string::npos !=
                   _collector->str().find("TICKETED FARE DATA", firstInstance + 1));
  }

  void testDisplayCoupons_With2SegsAndFareAmt2Set()
  {
    NegFareRest nfr;
    nfr.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    nfr.fareAmt2() = 12.345;
    nfr.cur21() = "USD";
    nfr.noDec21() = 3;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("FARE BOX CUR/AMT - USD12.345"));
  }

  void testDisplayCoupons_With2SegsAndFareAmt2Set_NoCurrency()
  {
    NegFareRest nfr;
    nfr.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    nfr.fareAmt2() = 12.345;
    nfr.cur21() = "";
    nfr.noDec21() = 3;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("FARE BOX CUR/AMT - \n"));
  }

  void testDisplayCoupons_With2SegsAndFareAmt2Zero()
  {
    NegFareRest nfr;
    nfr.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    nfr.fareAmt2() = 0;
    nfr.cur21() = "USD";
    nfr.noDec21() = 3;
    _collector->displayCoupons(*_trx, nfr);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("FARE BOX CUR/AMT - USD0.000"));
  }

  void testDisplayHeader_PUScope()
  {
    _collector->displayHeader(false);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("PRICING UNIT SCOPE"));
  }

  void testDisplayHeader_FPScope()
  {
    _collector->displayHeader(true);
    CPPUNIT_ASSERT(std::string::npos != _collector->str().find("FARE PATH SCOPE"));
  }

  FareUsage* createFareUsage(bool negotiated)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Loc* orig = _memHandle.create<Loc>();
    Loc* dest = _memHandle.create<Loc>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fInfo = _memHandle.create<FareInfo>();
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    fu->paxTypeFare() = ptf;
    ptf->fareMarket() = fm;
    ptf->setFare(fare);
    ptf->fareClassAppInfo() = appInfo;
    fare->setFareInfo(fInfo);
    fm->origin() = orig;
    fm->destination() = dest;
    orig->loc() = "ORI";
    dest->loc() = "DES";
    fInfo->carrier() = "CR";
    fInfo->fareClass() = "CLASS";
    fInfo->fareAmount() = 3.14;
    fInfo->currency() = "CAD";

    if (negotiated)
    {
      ptf->status().set(PaxTypeFare::PTF_Negotiated);
      NegPaxTypeFareRuleData* ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
      PaxTypeFare::PaxTypeFareAllRuleData* allRules =
          _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
      allRules->chkedRuleData = true;
      allRules->chkedGfrData = false;
      allRules->fareRuleData = ruleData;
      allRules->gfrRuleData = 0;
      ruleData->ruleItemInfo() = _memHandle.create<NegFareRest>();
      (*ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allRules;
    }
    return fu;
  }

  void testDiag535Collector_NotNegotiated()
  {
    _collector->diag535Collector(*_trx, *_itin, *createFareUsage(false));
    CPPUNIT_ASSERT_EQUAL(
        std::string("************************************************************\n"
                    "*         ATSE NEGOTIATED FARE RULE PROCESS                *\n"
                    "************************************************************\n"
                    "ORI-DES   CR   CLASS          3.14 CAD  DISP TYPE -     \n"
                    " NOT NEGOTIATED FARE \n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Collector_WrongCat35Data()
  {
    FareUsage* fu = createFareUsage(true);
    for(auto& a: *fu->paxTypeFare()->paxTypeFareRuleDataMap())
      a = nullptr;
    _collector->diag535Collector(*_trx, *_itin, *fu);
    CPPUNIT_ASSERT_EQUAL(
        std::string("************************************************************\n"
                    "*         ATSE NEGOTIATED FARE RULE PROCESS                *\n"
                    "************************************************************\n"
                    "ORI-DES   CR   CLASS          3.14 CAD  DISP TYPE -     C35\n"
                    " NOT NEGOTIATED FARE -WRONG PAX TYPE FARE RULE DATA\n"
                    "************************************************************\n"),
        _collector->str());
  }

  void testDiag535Collector_WrongRec3Data()
  {
    FareUsage* fu = createFareUsage(true);
    (*fu->paxTypeFare()->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE].load(std::memory_order_relaxed)
        ->fareRuleData->ruleItemInfo() = NULL;
    _collector->diag535Collector(*_trx, *_itin, *fu);
    CPPUNIT_ASSERT_EQUAL(
        std::string("************************************************************\n"
                    "*         ATSE NEGOTIATED FARE RULE PROCESS                *\n"
                    "************************************************************\n"
                    "ORI-DES   CR   CLASS          3.14 CAD  DISP TYPE -     C35\n"
                    " NOT NEGOTIATED FARE - WRONG NEGOTIATED FARE RULE DATA \n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Collector_Negotiated()
  {
    _collector->diag535Collector(*_trx, *_itin, *createFareUsage(true));
    CPPUNIT_ASSERT_EQUAL(
        std::string("************************************************************\n"
                    "*         ATSE NEGOTIATED FARE RULE PROCESS                *\n"
                    "************************************************************\n"
                    "ORI-DES   CR   CLASS          3.14 CAD  DISP TYPE -     C35\n"
                    " NEGOTIATED FARE RULE DATA: RECORD3 - 0\n"
                    "\n"
                    " NEG FARE SECURITY TABLE NUMBER  : 0\n"
                    " NEG FARE CALCULATE TABLE NUMBER : 0\n"
                    " COMMISSION PERCENT              : 0\n"
                    " NET REMIT METHOD                :  \n"
                    " NET GROSS INDICATOR             :  \n"
                    " BAGGAGE TYPE                    :  \n"
                    " BAGGAGE NUMBER                  : \n"
                    " NUMBER OF SEGMENTS              : 0\n"
                    " TOUR CODE DISPLAY OPTION : \n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Collector_PtfAsParameter()
  {
    _collector->diag535Collector(*createFareUsage(false)->paxTypeFare());
    CPPUNIT_ASSERT_EQUAL(
        std::string(" NET REMIT PUBLISHED FARE:\n"
                    " ORI-DES   CR   CLASS          3.14 CAD\n"
                    " RULE-     RULE TARIFF-0   OWRT-   GI-   FT-    ST-   DT-   \n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MultipleTourCode()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MULTIPLE_TOUR_CODE);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MULTIPLE TOUR CODES\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_TourCodeNotValid()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::TOUR_CODE_NOT_FOUND);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: TOUR CODE NOT FOUND\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MultipleValueCode()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MULTIPLE_VALUE_CODE);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MULTIPLE VALUE CODES\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MultiplePrintOption()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MULTIPLE_PRINT_OPTION);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MULTIPLE PRINT OPTIONS\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_InvalidItbt()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: INVALID ITBT PSG COUPON\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_EmptyItbt()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::EMPTY_ITBT_PSG_COUPON);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: EMPTY ITBT PSG COUPON\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_ItbtNotBlank()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::ITBT_BSP_NOT_BLANK);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: ITBT BSP NOT BLANK\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_NotItbt()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::NOT_ITBT);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: NOT ITBT\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MultipleFareBox()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MULTIPLE_FARE_BOX);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MULTIPLE FARE BOX\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MultipleBsp()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MULTIPLE_BSP);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MULTIPLE BSP\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MultipleNetGross()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MULTIPLE_NET_GROSS);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MULTIPLE NET GROSS\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MixFares()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MIX_FARES);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MIX FARES\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_FaresNotCombinable()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::FARES_NOT_COMBINABLE);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: FARES NOT COMBINABLE\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_NetSellingConflict()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::NET_SELLING_CONFLICT);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: NET SELLING CONFLICT\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MultipleCommision()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MULTIPLE_COMMISSION);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MULTIPLE COMMISSION\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MixCommision()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MIX_COMMISSION);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MIX COMMISSION\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_IssueSeparateTkt()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::ISSUE_SEPARATE_TKT);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: ISSUE SEPARATE TKT\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_InvalidNetRemit()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::INVALID_NET_REMIT_FARE);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: INVALID NET REMIT FARE\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_NoNetAmount()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::NO_NET_FARE_AMOUNT);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: NO NET FARE AMOUNT\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_NotCombinableNoNet()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::FARES_NOT_COMBINABLE_NO_NET);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: UNABLE TO AUTO TICKET - INVALID COMMISSION AMOUNT\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_InvalidNetRemitComm()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::INVALID_NET_REMIT_COMM);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: INVALID NET REMIT COMMISSION\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_AutoTktNotPermitted()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::AUTO_TKT_NOT_PERMITTED);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: AUTO TICKETING NOT PERMITTED\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_NetAmountExceedsFareAmount()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::NET_FARE_AMOUNT_EXCEEDS_FARE);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: NET AMOUNT EXCEEDS FARE\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MixedFareCombination()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MIXED_FARES_COMBINATION);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MIXED FARES INCL NET REMIT\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_ConflictingTfdByte101()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::CONFLICTING_TFD_BYTE101);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE:NET REMIT/CONFLICT IN FARE IND BYTE 101\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_Tfd()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::TFD_RETRIEVE_FAIL);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE:NET REMIT/UNA TO VAL TKT FARE DATA\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_MixedFareBoxAmount()
  {
    _collector->diag535Message(*_trx, NegotiatedFareRuleUtil::MIXED_FARE_BOX_AMT);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: MIXED FARE BOX CUR/AMT\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDiag535Message_Other()
  {
    _collector->diag535Message(*_trx, (tse::NegotiatedFareRuleUtil::WARNING_MSG)9999);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" WARNING MESSAGE CODE: SYSTEM ERROR\n"
                    "************************************************************\n"),
        _diagRoot->toString());
  }

  void testDisplayPaxTypeFare_NotNegotiated()
  {
    NegFareRest negFareRest;
    _collector->displayPaxTypeFare(
        *createFareUsage(false)->paxTypeFare(), &negFareRest, true, true, *_trx, createFarePath());

    CPPUNIT_ASSERT_EQUAL(std::string("ORI-DES   CR   CLASS          3.14 CAD  DISP TYPE -     \n"
                                     "  NOT NEGOTIATED FARE\n \n"),
                         _collector->str());
  }

  void testDisplayPaxTypeFare_Negotiated()
  {
    NegFareRest negFareRest;
    _collector->displayPaxTypeFare(
        *createFareUsage(true)->paxTypeFare(), &negFareRest, true, false, *_trx, NULL);

    CPPUNIT_ASSERT_EQUAL(std::string("ORI-DES   CR   CLASS          3.14 CAD  DISP TYPE -     C35\n"
                                     "  NEGOTIATED FARE RULE DATA: RECORD3 - 0\n"
                                     "  NET REMIT METHOD :  \n"
                                     "  COMMISSION : 0PCT\n"
                                     "  TOUR CODE DISPLAY OPTION : \n"
                                     "  FARE IND BYTE 101 :  \n \n"),
                         _collector->str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag535CollectorTest);
}
