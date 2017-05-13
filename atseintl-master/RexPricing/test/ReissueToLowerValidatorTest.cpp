#include "test/include/CppUnitHelperMacros.h"
#include "RexPricing/ReissueToLowerValidator.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareCompInfo.h"
#include "Rules/FareMarketRuleController.h"
#include "Pricing/Combinations.h"
#include "DBAccess/FareCalcConfig.h"
#include "Rules/PricingUnitRuleController.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

class ReissueToLowerValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ReissueToLowerValidatorTest);

  CPPUNIT_TEST(testHasNonHistoricalFarePass);
  CPPUNIT_TEST(testHasNonHistoricalFareFail);

  CPPUNIT_TEST(testCheckFCLevelValidationFail);
  CPPUNIT_TEST(testCheckFCLevelValidationPassByRule);
  CPPUNIT_TEST(testCheckFCLevelValidationPassByRouting);

  CPPUNIT_TEST(testCheckPULevelValidationFail);
  CPPUNIT_TEST(testCheckPULevelValidationPass1);
  CPPUNIT_TEST(testCheckPULevelValidationPass2);

  CPPUNIT_TEST(testCheckCombinationsFail);
  CPPUNIT_TEST(testCheckCombinationsPass1);
  CPPUNIT_TEST(testCheckCombinationsPass2);

  CPPUNIT_TEST(testRecalculateTotalAmount);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testHasNonHistoricalFarePass();
  void testHasNonHistoricalFareFail();

  void testCheckFCLevelValidationFail();
  void testCheckFCLevelValidationPassByRule();
  void testCheckFCLevelValidationPassByRouting();

  void testCheckPULevelValidationFail();
  void testCheckPULevelValidationPass1();
  void testCheckPULevelValidationPass2();

  void testCheckCombinationsFail();
  void testCheckCombinationsPass1();
  void testCheckCombinationsPass2();

  void testRecalculateTotalAmount();

protected:
  template <typename T>
  T* mem();
  FareMarket* FM(TravelSeg* p1, TravelSeg* p2 = 0, TravelSeg* p3 = 0);
  FareCompInfo* FC(FareMarket* fm, uint16_t fareCompNumber);
  Fare* FA(const FareClassCode& fc,
           const CurrencyCode& cc,
           const MoneyAmount& ma,
           const VendorCode& vc,
           const CarrierCode& cr,
           const TariffNumber& ft,
           const RuleNumber& rn,
           const Directionality& dr);
  FareMarket::RetrievalInfo* RI(const DateTime& date, const FareMarket::FareRetrievalFlags& flag);
  PaxTypeFare* PTF(FareMarket* fm, Fare* fa, FareMarket::RetrievalInfo* ri);
  FarePath* FP(PaxTypeFare** begin, PaxTypeFare** end);

private:
  RexPricingTrx* _trx;
  PaxType _pt;
  FareCalcConfig* _fareCalcConfig;
  std::map<FareMarket*, FareMarket*>* _FM2PrevReissueFMCache;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ReissueToLowerValidatorTest);

namespace mock
{

class MockCombinations : public Combinations
{

public:
  MockCombinations(bool r1 = true, bool r2 = true) : _result1(r1), _result2(r2) {}

  virtual CombinabilityValidationResult process(PricingUnit& pu,
                                                FareUsage*& failedSourceFareUsage,
                                                FareUsage*& failedTargetFareUsage,
                                                DiagCollector& diag,
                                                Itin* itin)
  {
    return _result1 ? CVR_PASSED : CVR_UNSPECIFIED_FAILURE;
  }

  virtual CombinabilityValidationResult process(FarePath& farePath,
                                                uint32_t farePathIndex,
                                                FareUsage*& failedSourceFareUsage,
                                                FareUsage*& failedEOETargetFareUsage,
                                                DiagCollector& diag)
  {
    return _result2 ? CVR_PASSED : CVR_UNSPECIFIED_FAILURE;
  }

private:
  bool _result1, _result2;
};

class MockPricingUnitRuleController : public PricingUnitRuleController
{
public:
  MockPricingUnitRuleController(bool r1 = true, bool r2 = true)
    : PricingUnitRuleController(1), _result1(r1), _result2(r2)
  {
  }
  virtual bool validate(PricingTrx& trx, FarePath& farePath, PricingUnit& pricingUnit)
  {
    return _categoryPhase == PURuleValidation ? _result1 : _result2;
  }
  virtual bool setPhase(CategoryPhase categoryPhase)
  {
    _categoryPhase = categoryPhase;
    return true;
  }

private:
  bool _result1, _result2;
};
}

void
ReissueToLowerValidatorTest::setUp()
{

  _pt.paxType() = "ADT";
  _trx = new RexPricingTrx;
  _trx->paxType().push_back(&_pt);

  _fareCalcConfig = new FareCalcConfig;
  _trx->fareCalcConfig() = _fareCalcConfig;
  _FM2PrevReissueFMCache = new std::map<FareMarket*, FareMarket*>;
}

void
ReissueToLowerValidatorTest::tearDown()
{
  delete _fareCalcConfig;
  delete _trx;
  delete _FM2PrevReissueFMCache;
}

template <typename T>
T*
ReissueToLowerValidatorTest::mem()
{
  T* ptr;
  _trx->dataHandle().get(ptr);
  return ptr;
}

FareMarket*
ReissueToLowerValidatorTest::FM(TravelSeg* p1, TravelSeg* p2, TravelSeg* p3)
{
  FareMarket* fm = mem<FareMarket>();
  if (p1)
    fm->travelSeg().push_back(p1);
  if (p2)
    fm->travelSeg().push_back(p2);
  if (p3)
    fm->travelSeg().push_back(p3);
  fm->setFCChangeStatus(-1);

  PaxTypeBucket ptc;
  ptc.requestedPaxType() = &_pt;
  fm->paxTypeCortege().push_back(ptc);

  return fm;
}

FareCompInfo*
ReissueToLowerValidatorTest::FC(FareMarket* fm, uint16_t fareCompNumber)
{
  FareCompInfo* fc = mem<FareCompInfo>();
  fc->fareCompNumber() = fareCompNumber;
  fc->fareMarket() = fm;

  return fc;
}

Fare*
ReissueToLowerValidatorTest::FA(const FareClassCode& fc,
                                const CurrencyCode& cc,
                                const MoneyAmount& ma,
                                const VendorCode& vc,
                                const CarrierCode& cr,
                                const TariffNumber& ft,
                                const RuleNumber& rn,
                                const Directionality& dr)
{
  FareInfo* fi = mem<FareInfo>();
  fi->fareClass() = fc;
  fi->currency() = cc;
  fi->originalFareAmount() = ma;
  fi->vendor() = vc;
  fi->carrier() = cr;
  fi->fareTariff() = ft;
  fi->ruleNumber() = rn;
  fi->directionality() = dr;

  Fare* f = mem<Fare>();
  f->setFareInfo(fi);
  return f;
}

FareMarket::RetrievalInfo*
ReissueToLowerValidatorTest::RI(const DateTime& date, const FareMarket::FareRetrievalFlags& flag)
{
  FareMarket::RetrievalInfo* ri = mem<FareMarket::RetrievalInfo>();
  ri->_date = date;
  ri->_flag = flag;
  return ri;
}

PaxTypeFare*
ReissueToLowerValidatorTest::PTF(FareMarket* fm, Fare* fa, FareMarket::RetrievalInfo* ri)
{
  PaxTypeFare* ptf = mem<PaxTypeFare>();
  ptf->fareMarket() = fm;
  ptf->setFare(fa);
  ptf->retrievalInfo() = ri;
  fm->allPaxTypeFare().push_back(ptf);
  fm->paxTypeCortege(&_pt)->paxTypeFare().push_back(ptf);
  ptf->setRoutingValid(true);
  ptf->nucFareAmount() = fa->originalFareAmount();

  return ptf;
}

FarePath*
ReissueToLowerValidatorTest::FP(PaxTypeFare** begin, PaxTypeFare** end)
{
  FarePath* fp = mem<FarePath>();
  for (; begin != end; ++begin)
  {
    FareUsage* fu = mem<FareUsage>();
    fu->paxTypeFare() = *begin;
    PricingUnit* pu = mem<PricingUnit>();
    pu->fareUsage().push_back(fu);
    fp->pricingUnit().push_back(pu);
  }

  return fp;
}

// -----=====##### TESTS #####=====------

void
ReissueToLowerValidatorTest::testHasNonHistoricalFarePass()
{
  DateTime originalDT(2008, 01, 02);
  DateTime currentDT(2008, 01, 20);

  _trx->setOriginalTktIssueDT() = originalDT;
  _trx->lastTktReIssueDT() = DateTime::emptyDate();

  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);
  FareMarket* fmH = FM(&seg);

  FareMarket::RetrievalInfo* ri[2] = { RI(currentDT, FareMarket::RetrievCurrent),
                                       RI(originalDT, FareMarket::RetrievHistorical) };

  PaxTypeFare* ptf[5] = { PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fm, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fmH, FA("OLD3", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[1]) };

  _FM2PrevReissueFMCache->insert(std::make_pair(fm, fmH));

  FarePath* fp = FP(ptf, ptf + 2);

  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);
  ReissueToLowerValidator::PaxTypeFareMap faresMap;

  CPPUNIT_ASSERT(validator.hasNonHistoricalFare(faresMap));
}

void
ReissueToLowerValidatorTest::testHasNonHistoricalFareFail()
{
  DateTime originalDT(2008, 01, 02);
  DateTime currentDT(2008, 01, 20);

  _trx->setOriginalTktIssueDT() = originalDT;
  _trx->lastTktReIssueDT() = DateTime::emptyDate();

  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);
  FareMarket* fmH = FM(&seg);

  FareMarket::RetrievalInfo* ri[2] = { RI(currentDT, FareMarket::RetrievCurrent),
                                       RI(originalDT, FareMarket::RetrievHistorical) };

  PaxTypeFare* ptf[5] = { PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fm, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fmH, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[1]) };

  _FM2PrevReissueFMCache->insert(std::make_pair(fm, fmH));

  FarePath* fp = FP(ptf, ptf + 2);

  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);
  ReissueToLowerValidator::PaxTypeFareMap faresMap;

  CPPUNIT_ASSERT(!validator.hasNonHistoricalFare(faresMap));
  CPPUNIT_ASSERT_EQUAL(ptf[3], faresMap[ptf[0]]);
  CPPUNIT_ASSERT_EQUAL(ptf[2], faresMap[ptf[1]]);
}

void
ReissueToLowerValidatorTest::testCheckFCLevelValidationFail()
{
  DateTime originalDT(2008, 01, 02);
  DateTime currentDT(2008, 01, 20);

  _trx->setOriginalTktIssueDT() = originalDT;
  _trx->lastTktReIssueDT() = DateTime::emptyDate();

  AirSeg seg;
  seg.pnrSegment() = 1;
  FareMarket* fm = FM(&seg);
  FareMarket* fmH = FM(&seg);

  FareMarket::RetrievalInfo* ri[2] = { RI(currentDT, FareMarket::RetrievCurrent),
                                       RI(originalDT, FareMarket::RetrievHistorical) };

  PaxTypeFare* ptf[6] = { PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fm, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fm, FA("NEW0", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fmH, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("NEW0", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), ri[1]) };

  ReissueToLowerValidator::PaxTypeFareMap faresMap;
  for (int i = 0; i < 3; ++i)
    faresMap[ptf[i]] = ptf[i + 3];

  FarePath* fp = 0; // not used in this case
  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);
  CPPUNIT_ASSERT(!validator.checkFCLevelValidation(faresMap));
}

void
ReissueToLowerValidatorTest::testCheckFCLevelValidationPassByRule()
{
  DateTime originalDT(2008, 01, 02);
  DateTime currentDT(2008, 01, 20);

  _trx->setOriginalTktIssueDT() = originalDT;
  _trx->lastTktReIssueDT() = DateTime::emptyDate();

  AirSeg seg;
  seg.pnrSegment() = 1;
  FareMarket* fm = FM(&seg);
  FareMarket* fmH = FM(&seg);

  FareMarket::RetrievalInfo* ri[2] = { RI(currentDT, FareMarket::RetrievCurrent),
                                       RI(originalDT, FareMarket::RetrievHistorical) };

  PaxTypeFare* ptf[6] = { PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fm, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fm, FA("NEW0", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fmH, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("NEW0", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), ri[1]) };

  CPPUNIT_ASSERT(ptf[4]->areAllCategoryValid());
  ptf[4]->setCategoryValid(1, false);
  CPPUNIT_ASSERT(!ptf[4]->areAllCategoryValid());

  ReissueToLowerValidator::PaxTypeFareMap faresMap;
  for (int i = 0; i < 3; ++i)
    faresMap[ptf[i]] = ptf[i + 3];

  FarePath* fp = 0; // not used in this case
  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);
  CPPUNIT_ASSERT(validator.checkFCLevelValidation(faresMap));
}

void
ReissueToLowerValidatorTest::testCheckFCLevelValidationPassByRouting()
{
  DateTime originalDT(2008, 01, 02);
  DateTime currentDT(2008, 01, 20);

  _trx->setOriginalTktIssueDT() = originalDT;
  _trx->lastTktReIssueDT() = DateTime::emptyDate();

  AirSeg seg;
  seg.pnrSegment() = 1;
  FareMarket* fm = FM(&seg);
  FareMarket* fmH = FM(&seg);

  FareMarket::RetrievalInfo* ri[2] = { RI(currentDT, FareMarket::RetrievCurrent),
                                       RI(originalDT, FareMarket::RetrievHistorical) };

  PaxTypeFare* ptf[6] = { PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fm, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fm, FA("NEW0", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), ri[0]),
                          PTF(fmH, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("OLD2", "NUC", 120.00, "ATP", "AA", 6, "500", BOTH), ri[1]),
                          PTF(fmH, FA("NEW0", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), ri[1]) };

  CPPUNIT_ASSERT(ptf[4]->isRoutingValid());
  ptf[4]->setRoutingValid(false);
  CPPUNIT_ASSERT(!ptf[4]->isRoutingValid());

  ReissueToLowerValidator::PaxTypeFareMap faresMap;
  for (int i = 0; i < 3; ++i)
    faresMap[ptf[i]] = ptf[i + 3];

  FarePath* fp = 0; // not used in this case
  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);
  CPPUNIT_ASSERT(validator.checkFCLevelValidation(faresMap));
}

void
ReissueToLowerValidatorTest::testCheckPULevelValidationFail()
{
  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);

  PaxTypeFare* ptf[3] = { PTF(fm, FA("OLD3", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0) };

  FarePath* fp = 0;
  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);

  FarePath* fpH = FP(ptf, ptf + 3);

  CPPUNIT_ASSERT(!validator.checkPULevelValidation(*fpH));
}

void
ReissueToLowerValidatorTest::testCheckPULevelValidationPass1()
{
  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);

  PaxTypeFare* ptf[3] = { PTF(fm, FA("OLD3", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0) };

  FarePath* fp = 0;
  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl(true, false);

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);

  FarePath* fpH = FP(ptf, ptf + 3);

  CPPUNIT_ASSERT(validator.checkPULevelValidation(*fpH));
}

void
ReissueToLowerValidatorTest::testCheckPULevelValidationPass2()
{
  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);

  PaxTypeFare* ptf[3] = { PTF(fm, FA("OLD3", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0) };

  FarePath* fp = 0;
  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl(false, true);

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);

  FarePath* fpH = FP(ptf, ptf + 3);

  CPPUNIT_ASSERT(validator.checkPULevelValidation(*fpH));
}

void
ReissueToLowerValidatorTest::testCheckCombinationsFail()
{
  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);

  PaxTypeFare* ptf[3] = { PTF(fm, FA("OLD3", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0) };

  FarePath* fp = 0;
  mock::MockCombinations cCtrl;
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);

  FarePath* fpH = FP(ptf, ptf + 3);

  CPPUNIT_ASSERT(!validator.checkCombinations(*fpH));
}

void
ReissueToLowerValidatorTest::testCheckCombinationsPass1()
{
  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);

  PaxTypeFare* ptf[3] = { PTF(fm, FA("OLD3", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0) };

  FarePath* fp = 0;
  mock::MockCombinations cCtrl(true, false);
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);

  FarePath* fpH = FP(ptf, ptf + 3);

  CPPUNIT_ASSERT(validator.checkCombinations(*fpH));
}

void
ReissueToLowerValidatorTest::testCheckCombinationsPass2()
{
  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);

  PaxTypeFare* ptf[3] = { PTF(fm, FA("OLD3", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0) };

  FarePath* fp = 0;
  mock::MockCombinations cCtrl(false, true);
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);

  FarePath* fpH = FP(ptf, ptf + 3);

  CPPUNIT_ASSERT(validator.checkCombinations(*fpH));
}

void
ReissueToLowerValidatorTest::testRecalculateTotalAmount()
{
  AirSeg seg;
  seg.pnrSegment() = 1;

  FareMarket* fm = FM(&seg);

  PaxTypeFare* ptf[3] = { PTF(fm, FA("OLD3", "NUC", 140.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("OLD1", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0),
                          PTF(fm, FA("NEW0", "NUC", 100.00, "ATP", "AA", 6, "500", BOTH), 0) };

  FarePath* fp = 0;
  mock::MockCombinations cCtrl(false, true);
  mock::MockPricingUnitRuleController rCtrl;

  ReissueToLowerValidator validator(*_trx, *fp, cCtrl, rCtrl, *_FM2PrevReissueFMCache);

  FarePath* fpH = FP(ptf, ptf + 3);
  validator.recalculateTotalAmount(*fpH);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(340.00, fpH->getTotalNUCAmount(), EPSILON);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, fpH->plusUpAmount(), EPSILON);
}

} // end of tse namespace
