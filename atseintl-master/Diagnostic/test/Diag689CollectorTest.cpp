#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/FareTypeTable.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/Diag689Collector.h"
#include "RexPricing/PenaltyAdjuster.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class FakeRexTrx : public RefundPricingTrx
{
  std::vector<FareTypeTable*>* _fttv;

public:
  FakeRexTrx()
  {
    _fttv = new std::vector<FareTypeTable*>;
    _fttv->push_back(new FareTypeTable);
  }

  virtual const std::vector<FareTypeTable*>&
  getFareTypeTables(const VendorCode& vendor, uint32_t tblItemNo, const DateTime& applicationDate)
      const
  {
    return *_fttv;
  }

  virtual Money
  convertCurrency(const Money& source, const CurrencyCode& targetCurr, bool rounding) const
  {
    if ((source.code() == NUC && targetCurr == USD) || (source.code() == USD && targetCurr == NUC))
      return Money(source.value(), targetCurr);

    return Money(source.value() * 2, targetCurr);
  }

  virtual ~FakeRexTrx()
  {
    delete _fttv->front();
    delete _fttv;
  }
};

class Diag689CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag689CollectorTest);

  CPPUNIT_TEST(testPrintRefundProcessInfo);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintMapping);
  CPPUNIT_TEST(testPrintPermutation);
  CPPUNIT_TEST(testPrintRecord3PenaltyPart);
  CPPUNIT_TEST(testPrintFareComponenet);

  CPPUNIT_TEST(testPrintPricinUnit);
  CPPUNIT_TEST(testPrintPricinUnit_noPenalty);
  CPPUNIT_TEST(testPrintPricinUnit_noRecord3);

  CPPUNIT_TEST(printFee_discount);
  CPPUNIT_TEST(printFee_noDiscount);
  CPPUNIT_TEST(printFee_100percent);

  CPPUNIT_TEST(printPricingUnitPenaltys_fcLevel);
  CPPUNIT_TEST(printPricingUnitPenaltys_puLevel);
  CPPUNIT_TEST(printPricingUnitPenaltys_noFees);

  CPPUNIT_TEST(testFilterByFarePathRebook);
  CPPUNIT_TEST(testFilterByFarePathAsbook);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _memH.create<RootLoggerGetOff>();

    _diag = _memH.create<Diag689Collector>();
    _diag->activate();
    _diag->_fpFilterPassed = true;
    _diag->_permFilterPassed = true;
  }

  void tearDown() { _memH.clear(); }

  std::string getDiagString()
  {
    _diag->flushMsg();
    return _diag->str();
  }

  VoluntaryRefundsInfo* createRecord3()
  {
    VoluntaryRefundsInfo* vri = _memH.create<VoluntaryRefundsInfo>();
    vri->itemNo() = 12345;
    vri->fareBreakpoints() = 'A';
    vri->repriceInd() = 'A';
    vri->ruleTariff() = 43;
    vri->ruleTariffInd() = 'A';
    vri->rule() = 'R';
    vri->fareClassInd() = 'A';
    vri->fareClass() = "SDFFF";
    vri->fareTypeTblItemNo() = 12345;
    vri->sameFareInd() = 'A';
    vri->nmlSpecialInd() = 'A';
    vri->owrt() = 'A';
    vri->fareAmountInd() = 'A';
    vri->bookingCodeInd() = 'A';
    vri->formOfRefund() = 'A';
    vri->cancellationInd() = 'A';
    vri->taxNonrefundableInd() = 'A';
    vri->penalty1Amt() = 33.33;
    vri->penalty1Cur() = "NUC";
    vri->penalty2Amt() = 34.33;
    vri->penalty2Cur() = "NUC";
    vri->penaltyPercent() = 55.55;
    vri->highLowInd() = 'A';
    vri->minimumAmt() = 66.66;
    vri->minimumAmtCur() = "NUC";
    vri->reissueFeeInd() = 'A';
    vri->calcOption() = 'A';
    vri->discountTag1() = 'A';
    vri->discountTag2() = 'A';
    vri->discountTag3() = 'A';
    vri->discountTag4() = 'A';
    vri->fareTypeTblItemNo() = 0;
    return vri;
  }

  RefundProcessInfo* createRefundProcessInfo()
  {
    PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
    FareMarket* fm = _memH.create<FareMarket>();
    ptf->fareMarket() = fm;
    fm->fareCompInfo() = _memH.create<FareCompInfo>();
    fm->fareCompInfo()->fareMarket() = fm;

    fm->boardMultiCity() = "KRK";
    fm->offMultiCity() = "DFW";
    fm->governingCarrier() = "CO";

    fm->origin() = _memH.create<Loc>();
    VoluntaryRefundsInfo* r3 = createRecord3();
    return _memH.create<RefundProcessInfo>(r3, ptf, fm->fareCompInfo());
  }

  RefundPermutation* createRefundPermutation(int number)
  {
    RefundPermutation* rp = _memH.create<RefundPermutation>();
    rp->number() = number;
    return rp;
  }

  void testPrintRefundProcessInfo()
  {
    setupTransaction();
    RefundProcessInfo* info = createRefundProcessInfo();
    _diag->print(*info);
    CPPUNIT_ASSERT_EQUAL(std::string("FARE BREAKS: A\n"
                                     "REPRICE IND: A\n"
                                     "TARIFF NUMBER: 43    EXCLUDE PRIVATE/PUBLIC: A\n"
                                     "RULE NUMBER: R\n"
                                     "FARE CLASS/FAMILY IND: A   FARE CLASS: SDFFF\n"
                                     "FARE TYPE TBL: 0    FARE TYPE IND:    FARE TYPES: \n"
                                     "SAME: A     NORMAL/SPECIAL: A     OWRT: A\n"
                                     "FARE AMOUNT: A          RBD IND: A\n"
                                     "FORM OF REFUND: A\n"),
                         getDiagString());
  }

  void testPrintHeader()
  {
    RefundProcessInfo* info = createRefundProcessInfo();
    _diag->printHeader(*info);
    CPPUNIT_ASSERT_EQUAL(std::string("   0: KRK-CO-DFW R3 ITEM 12345\n"), getDiagString());
  }

  void testPrintMapping()
  {
    RefundProcessInfo* info = createRefundProcessInfo();
    _diag->printMapping(info->paxTypeFare());
    CPPUNIT_ASSERT_EQUAL(std::string("MATCHING TO REPRICE FC KRK-DFW\n"), getDiagString());
  }

  void testPrintPermutation()
  {
    RefundPermutation* perm = createRefundPermutation(3);
    _diag->_minRange = 0;
    _diag->_maxRange = 0;
    _diag->_permutationIndex = 0;
    _diag->print(*perm);
    CPPUNIT_ASSERT_EQUAL(
        std::string("--------------------------------------------------------------\n"
                    "PERMUTATION 3 \n"),
        getDiagString());
  }

  void testPrintRecord3PenaltyPart()
  {
    _diag->printRecord3PenaltyPart(*createRecord3());
    CPPUNIT_ASSERT_EQUAL(std::string(" 100% PENALTY- A  TAX NON REF- A\n"
                                     " PENALTY1- 33.33 NUC         PENALTY2- 34.33 NUC        \n"
                                     " PERCENT- 55.55  H/L- A\n"
                                     " PU/FC- A  CALC OPTION: A\n"
                                     " MINAMT- 66.66 NUC         DISCOUNT: A A A A\n"),
                         getDiagString());
  }

  FareUsage* createFareUsage(int fcNumber, const PaxTypeFare* ptf)
  {
    FareUsage* fu = _memH.insert(new FareUsage);
    fu->paxTypeFare() = const_cast<PaxTypeFare*>(ptf);

    Fare* f = _memH.insert(new Fare);
    f->nucFareAmount() = 100.00;
    fu->paxTypeFare()->setFare(f);
    fu->paxTypeFare()->fareMarket()->fareCompInfo()->fareCompNumber() = fcNumber;

    return fu;
  }

  FareUsage* createFareUsage()
  {
    FareUsage* fu = _memH.create<FareUsage>();
    fu->paxTypeFare() = _memH.create<PaxTypeFare>();

    FareMarket* fm = _memH.create<FareMarket>();
    fu->paxTypeFare()->fareMarket() = fm;
    fm->fareCompInfo() = _memH.create<FareCompInfo>();
    fm->fareCompInfo()->fareCompNumber() = 1;
    fm->boardMultiCity() = "NYC";
    fm->governingCarrier() = "AA";
    fm->offMultiCity() = "LAX";

    fm->origin() = _memH.create<Loc>();

    Fare* f = _memH.create<Fare>();
    f->nucFareAmount() = 100.00;
    fu->paxTypeFare()->setFare(f);
    return fu;
  }

  void setupTransaction()
  {
    FakeRexTrx* trx = _memH.create<FakeRexTrx>();

    trx->itin().push_back(_memH.create<Itin>());
    trx->itin().front()->originationCurrency() = "CAD";

    trx->exchangeItin().push_back(_memH.create<ExcItin>());
    trx->exchangeItin().front()->calculationCurrency() = NUC;
    trx->exchangeItin().front()->farePath().push_back(_memH.create<FarePath>());

    RexPricingOptions* op = _memH.create<RexPricingOptions>();
    trx->setOptions(op);

    _diag->trx() = trx;
  }

  void testPrintFareComponenet()
  {
    setupTransaction();
    FareUsage* fu = createFareUsage();
    PricingUnit* pu = _memH.insert(new PricingUnit);
    pu->fareUsage().push_back(fu);
    _diag->printFareComponent(
        *fu, *_memH.insert(new PenaltyAdjuster(*pu, PenaltyAdjuster::SUMARIZE_FU)));
    CPPUNIT_ASSERT_EQUAL(std::string("   1: NYC-AA-LAX O   100.00 NUC       \n"), getDiagString());
  }

  PricingUnit* createPricingUnit(const RefundPermutation& perm)
  {
    PricingUnit* pu = _memH.insert(new PricingUnit);
    for (unsigned i = 0; i < perm.processInfos().size(); ++i)
      pu->fareUsage().push_back(createFareUsage(i + 1, &perm.processInfos()[i]->paxTypeFare()));
    return pu;
  }

  void testPrintPricinUnit_noPenalty()
  {
    setupTransaction();
    RefundPermutation& perm = *createRefundPermutation(2);
    perm.processInfos().push_back(createRefundProcessInfo());

    PricingUnit& pu = *createPricingUnit(perm);

    _diag->printPricingUnit(pu, perm);
    CPPUNIT_ASSERT_EQUAL(std::string("   1: KRK-CO-DFW O   100.00 NUC       \n"
                                     " 100% PENALTY- A  TAX NON REF- A\n"
                                     " PENALTY1- 33.33 NUC         PENALTY2- 34.33 NUC        \n"
                                     " PERCENT- 55.55  H/L- A\n"
                                     " PU/FC- A  CALC OPTION: A\n"
                                     " MINAMT- 66.66 NUC         DISCOUNT: A A A A\n"),
                         getDiagString());
  }

  RefundPenalty* createPenalty(RefundPenalty::Scope scope)
  {
    RefundPenalty* p = _memH.insert(new RefundPenalty);
    std::vector<RefundPenalty::Fee> fee(scope == RefundPenalty::FC ? 2 : 1);

    for (unsigned i = 0; i < fee.size(); ++i)
      fee[i] = RefundPenalty::Fee(Money((i + 1) * 20 + 2 * i, NUC), i);

    p->assign(fee, scope);
    return p;
  }

  void testPrintPricinUnit()
  {
    setupTransaction();
    RefundPermutation& perm = *createRefundPermutation(2);
    perm.processInfos().push_back(createRefundProcessInfo());

    PricingUnit& pu = *createPricingUnit(perm);

    perm.penaltyFees()[&pu] = createPenalty(RefundPenalty::PU);

    _diag->printPricingUnit(pu, perm);
    CPPUNIT_ASSERT_EQUAL(
        std::string("   1: KRK-CO-DFW O   100.00 NUC       \n"
                    " 100% PENALTY- A  TAX NON REF- A\n"
                    " PENALTY1- 33.33 NUC         PENALTY2- 34.33 NUC        \n"
                    " PERCENT- 55.55  H/L- A\n"
                    " PU/FC- A  CALC OPTION: A\n"
                    " MINAMT- 66.66 NUC         DISCOUNT: A A A A\n"
                    "PENALTY FEE CHARGED ON PU LEVEL\n"
                    "PENALTY FEE 1: 20.00 NUC         20.00 NUC         DISCOUNT: N\n"),
        getDiagString());
  }

  void testPrintPricinUnit_noRecord3()
  {
    setupTransaction();
    RefundPermutation& perm = *createRefundPermutation(2);
    perm.processInfos().push_back(createRefundProcessInfo());

    PricingUnit& pu = *createPricingUnit(perm);

    perm.processInfos().clear();

    _diag->printPricingUnit(pu, perm);
    CPPUNIT_ASSERT_EQUAL(std::string("   1: KRK-CO-DFW O   100.00 NUC       \n"
                                     "ERROR: NO RECORD3 FOR THIS FARE COMPONENT!\n"),
                         getDiagString());
  }

  void printPricingUnitPenaltys_noFees()
  {
    setupTransaction();
    RefundPermutation& perm = *createRefundPermutation(2);
    perm.processInfos().push_back(createRefundProcessInfo());

    PricingUnit& pu = *createPricingUnit(perm);

    RefundPenalty pen;
    perm.penaltyFees()[&pu] = &pen;

    _diag->printPricingUnit(pu, perm);
    CPPUNIT_ASSERT_EQUAL(std::string("   1: KRK-CO-DFW O   100.00 NUC       \n"
                                     " 100% PENALTY- A  TAX NON REF- A\n"
                                     " PENALTY1- 33.33 NUC         PENALTY2- 34.33 NUC        \n"
                                     " PERCENT- 55.55  H/L- A\n"
                                     " PU/FC- A  CALC OPTION: A\n"
                                     " MINAMT- 66.66 NUC         DISCOUNT: A A A A\n"
                                     "PENALTY FEE CHARGED ON FC LEVEL\n"),
                         getDiagString());
  }

  enum
  {
    noRefundable = 1,
    Refundable = 0
  };

  enum
  {
    noDiscounted = 0,
    Discounted = 1
  };

  void printFee_discount()
  {
    setupTransaction();
    RefundPenalty::Fee fee(Money(10.0, NUC), Discounted, Refundable);
    _diag->printFee(fee, 1);
    CPPUNIT_ASSERT_EQUAL(
        std::string("PENALTY FEE 1: 10.00 NUC         10.00 NUC         DISCOUNT: Y\n"),
        getDiagString());
  }

  void printFee_noDiscount()
  {
    setupTransaction();
    RefundPenalty::Fee fee(Money(10.0, NUC), noDiscounted, Refundable);
    _diag->printFee(fee, 1);
    CPPUNIT_ASSERT_EQUAL(
        std::string("PENALTY FEE 1: 10.00 NUC         10.00 NUC         DISCOUNT: N\n"),
        getDiagString());
  }

  void printFee_100percent()
  {
    setupTransaction();
    RefundPenalty::Fee fee(Money(10.0, NUC), noDiscounted, noRefundable);
    _diag->printFee(fee, 1);
    CPPUNIT_ASSERT_EQUAL(std::string("PENALTY FEE 1: 100%\n"), getDiagString());
  }

  void printPricingUnitPenaltys_fcLevel()
  {
    setupTransaction();
    _diag->printPricingUnitPenaltys(*createPenalty(RefundPenalty::FC));
    CPPUNIT_ASSERT_EQUAL(
        std::string("PENALTY FEE CHARGED ON FC LEVEL\n"
                    "PENALTY FEE 1: 20.00 NUC         20.00 NUC         DISCOUNT: N\n"
                    "PENALTY FEE 2: 42.00 NUC         42.00 NUC         DISCOUNT: Y\n"),
        getDiagString());
  }

  void printPricingUnitPenaltys_puLevel()
  {
    setupTransaction();
    _diag->printPricingUnitPenaltys(*createPenalty(RefundPenalty::PU));
    CPPUNIT_ASSERT_EQUAL(
        std::string("PENALTY FEE CHARGED ON PU LEVEL\n"
                    "PENALTY FEE 1: 20.00 NUC         20.00 NUC         DISCOUNT: N\n"),
        getDiagString());
  }

  void testFilterByFarePathRebook()
  {
    FarePath fp;
    fp.rebookClassesExists() = _diag->_filterRebook = true;
    fp.pricingUnit().push_back(0);
    CPPUNIT_ASSERT(_diag->_fpFilterPassed);
    CPPUNIT_ASSERT_NO_THROW(_diag->filterByFarePath(fp));
    CPPUNIT_ASSERT(!_diag->_fpFilterPassed);
  }

  void testFilterByFarePathAsbook()
  {
    FarePath fp;
    fp.rebookClassesExists() = false;
    _diag->_filterAsbook = true;
    fp.pricingUnit().push_back(0);
    CPPUNIT_ASSERT(_diag->_fpFilterPassed);
    CPPUNIT_ASSERT_NO_THROW(_diag->filterByFarePath(fp));
    CPPUNIT_ASSERT(!_diag->_fpFilterPassed);
  }

protected:
  Diag689Collector* _diag;
  TestMemHandle _memH;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag689CollectorTest);

} // tse
