#include "Diagnostic/Diag316Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include <iostream>
#include "test/include/TestMemHandle.h"
#include "DBAccess/Loc.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/PenaltyInfo.h"
#include "DataModel/PaxTypeFare.h"

using namespace std;
namespace tse
{
class Diag316CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag316CollectorTest);
  CPPUNIT_TEST(testWriteHeadera_llInd_Blank);
  CPPUNIT_TEST(testWriteHeader_IndB);
  CPPUNIT_TEST(testWriteHeader_IndN);
  CPPUNIT_TEST(testWriteHeader_IndX);
  CPPUNIT_TEST_SUITE_END();

private:
  Diag316Collector* _diag;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    try
    {
      _diag = _memHandle.create<Diag316Collector>();
      _diag->enable(Diagnostic316);
      _diag->activate();
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  void initDiag316Data(PaxTypeFare& paxTypeFare,
                       CategoryRuleInfo& cri,
                       PenaltyInfo& penaltyInfo,
                       Indicator noRefundInd,
                       Indicator penaltyReissue,
                       Indicator volAppl,
                       Indicator penaltyNoReissue,
                       Indicator cancelRefundAppl)
  {
    penaltyInfo.itemNo() = 151;
    FareMarket* fareMarket = _memHandle.create<FareMarket>();

    Loc* org = _memHandle.create<Loc>();
    org->loc() = "KRK";
    Loc* dest = _memHandle.create<Loc>();
    dest->loc() = "LON";

    fareMarket->origin() = org;
    fareMarket->destination() = dest;
    paxTypeFare.fareMarket() = fareMarket;

    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fare->setFareInfo(fareInfo);
    paxTypeFare.setFare(fare);

    // CategoryRuleInfo
    cri.vendorCode() = "5KAD";
    cri.tariffNumber() = 456;
    cri.carrierCode() = "AA";
    cri.ruleNumber() = "123";

    penaltyInfo.noRefundInd() = noRefundInd;
    penaltyInfo.penaltyNoDec1() = 2;
    penaltyInfo.penaltyReissue() = penaltyReissue;
    penaltyInfo.penaltyNoReissue() = penaltyNoReissue;
    penaltyInfo.volAppl() = volAppl;
    penaltyInfo.cancelRefundAppl() = cancelRefundAppl;
    penaltyInfo.penaltyAmt1() = 10;
    penaltyInfo.penaltyCur1() = "USD";
    penaltyInfo.penaltyAmt2() = 20;
    penaltyInfo.penaltyCur2() = "USD";
    penaltyInfo.penaltyPercent() = 50;
  }

  std::string prepareExpString(Indicator noRefundInd,
                               Indicator penaltyReissue = 'X',
                               Indicator volAppl = 'X',
                               Indicator penaltyNoReissue = 'X',
                               Indicator cancelRefundAppl = 'X')
  {
    std::string expect;
    expect += "******************************************************\n";
    expect += "*   CATEGORY 16 PENALTIES APPLICATION DIAGNOSTICS    *\n";
    expect += "******************************************************\n";
    expect += "PHASE: PRICING UNIT     R3 ITEM NUMBER: 151\n";
    expect += "KRK LON      R2:FARERULE    :  5KAD 456 AA 123\n";
    expect += "EXCLUDE PENALTIES: NO\n";
    expect += "CATEGORY 16 RULE DATA\n";

    if (noRefundInd == Diag316Collector::NO_APPLICATION)
      expect += "TICKET/RESERVATIONS RESTRICTIONS - N/A\n";
    else if (noRefundInd == 'B')
      expect += "TICKET/RESERVATIONS RESTRICTIONS - NO-REFUND/NO CHANGE\n";
    else if (noRefundInd == 'N')
      expect += "TICKET/RESERVATIONS RESTRICTIONS - NO CHANGE\n";
    else if (noRefundInd == 'X')
      expect += "TICKET/RESERVATIONS RESTRICTIONS - NO-REFUND\n";

    expect += "PENALTIES: \n";
    if ('X' == penaltyReissue)
      expect += "CHANGE OF ITINERARY REQUIRING REISSUE OF TICKET: - APPLIES\n";
    else
      expect += "CHANGE OF ITINERARY REQUIRING REISSUE OF TICKET: - N/A\n";

    if ('X' == penaltyNoReissue)
      expect += "CHANGE NOT REQUIRING REISSUE OF TICKET: - APPLIES\n";
    else
      expect += "CHANGE NOT REQUIRING REISSUE OF TICKET: - N/A\n";

    if ('X' == penaltyNoReissue)
      expect += "PENALTY VOLUNTARY APPL: - APPLIES\n";
    else
      expect += "PENALTY VOLUNTARY APPL: - N/A\n";

    if ('X' == cancelRefundAppl)
      expect += "PENALTY CANCEL REFUND APPL: - APPLIES\n";
    else
      expect += "PENALTY CANCEL REFUND APPL: - N/A\n";

    expect += "PENALTY AMT1 - 10.00\n";
    expect += "PENALTY CUR1 - USD\n";
    expect += "PENALTY AMT2 - 20\n";
    expect += "PENALTY CUR2 - USD\n";
    expect += "PENALTY PERCENT - 50\n";

    return expect;
  }

  void testWriteHeadera_llInd_Blank()
  {
    PaxTypeFare paxTypeFare;
    CategoryRuleInfo cri;
    PenaltyInfo penaltyInfo;
    PricingTrx trx;

    bool farePhase = false;
    trx.setOptions(_memHandle.create<PricingOptions>());
    initDiag316Data(paxTypeFare, cri, penaltyInfo, ' ', ' ', ' ', ' ', ' ');
    std::string expect = prepareExpString(' ', ' ', ' ', ' ', ' ');

    _diag->writeHeader(paxTypeFare, cri, penaltyInfo, trx, farePhase);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testWriteHeader_IndB()
  {
    PaxTypeFare paxTypeFare;
    CategoryRuleInfo cri;
    PenaltyInfo penaltyInfo;
    PricingTrx trx;

    bool farePhase = false;
    trx.setOptions(_memHandle.create<PricingOptions>());
    initDiag316Data(paxTypeFare, cri, penaltyInfo, 'B', 'X', 'X', 'X', 'X');

    std::string expect = prepareExpString('B');
    _diag->writeHeader(paxTypeFare, cri, penaltyInfo, trx, farePhase);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testWriteHeader_IndN()
  {
    PaxTypeFare paxTypeFare;
    CategoryRuleInfo cri;
    PenaltyInfo penaltyInfo;
    PricingTrx trx;

    bool farePhase = false;
    trx.setOptions(_memHandle.create<PricingOptions>());
    initDiag316Data(paxTypeFare, cri, penaltyInfo, 'N', 'X', 'X', 'X', 'X');

    std::string expect = prepareExpString('N');
    _diag->writeHeader(paxTypeFare, cri, penaltyInfo, trx, farePhase);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testWriteHeader_IndX()
  {
    PaxTypeFare paxTypeFare;
    CategoryRuleInfo cri;
    PenaltyInfo penaltyInfo;
    PricingTrx trx;

    bool farePhase = false;
    trx.setOptions(_memHandle.create<PricingOptions>());
    initDiag316Data(paxTypeFare, cri, penaltyInfo, 'X', 'X', 'X', 'X', 'X');

    std::string expect = prepareExpString('X');
    _diag->writeHeader(paxTypeFare, cri, penaltyInfo, trx, farePhase);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag316CollectorTest);
} // tse
