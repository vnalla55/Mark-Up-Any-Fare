#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag942Collector.h"
#include "Pricing/Shopping/Diversity/DmcNSPerCarrierRequirement.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"
#include "Pricing/Shopping/PQ/test/TestPQItem.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

namespace shpq
{
class CarrierTestPQItem;
typedef CarrierTestPQItem* CarrierTestPQItemPtr;
}

// Mocks
class ItinStatisticMock : public ItinStatistic
{
public:
  ItinStatisticMock(ShoppingTrx& trx) : ItinStatistic(trx) {}

  void setNumOfNonStopItinsPerCarrier(const std::map<CarrierCode, size_t>& nonStopCarriers)
  {
    _nonStopCarriers = nonStopCarriers;
  }

  void setNumOfNonStopItinsForCarrier(CarrierCode cxr, size_t number)
  {
    _nonStopCarriers[cxr] = number;
  }
};

class ContextMock : public DmcRequirementsSharedContext
{
public:
  ContextMock(Diversity& diversity, ItinStatistic& stats, DiagCollector* dc, ShoppingTrx& trx)
    : DmcRequirementsSharedContext(diversity, stats, dc, trx)
  {
  }

  void printRequirements(bool bucketsOnly) {};
  void printCarriersRequirements(bool directOnly = false) {};
};

namespace shpq
{

class CarrierTestPQItem : public test::TestPQItem
{
public:
  CarrierTestPQItem(const MoneyAmount score,
                    const SoloPQItemLevel level,
                    const SolutionPattern* sp = 0)
    : TestPQItem(score, level, sp)
  {
  }

  static CarrierTestPQItemPtr
  create(const MoneyAmount score, const SoloPQItemLevel level, const SolutionPattern* sp = 0)
  {
    return CarrierTestPQItemPtr(new CarrierTestPQItem(score, level, sp));
  }
};

} // shpq

namespace
{

// =================================
// LEGS DATA
// =================================

DateTime obDate = DateTime(2013, 06, 01);
DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "LH", DT(obDate, 6), DT(obDate, 7) }, // 1h
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 5), DT(obDate, 7) }, // 2h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }, // 1h
  { 1, 1, "AA", "DFW", "JFK", "LH", DT(ibDate, 11), DT(ibDate, 13) } // 2h
};
#undef DT

} // anonymous namespace

class DmcNSPerCarrierRequirementTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DmcNSPerCarrierRequirementTest);

  CPPUNIT_TEST(testIsOptNeededForNSCarrierGeneral);
  CPPUNIT_TEST(testIsOptNeededForNSCarrierSpecific);

  CPPUNIT_TEST(testGetStatusNotEnabled);
  CPPUNIT_TEST(testGetStatusNeedOptions);
  CPPUNIT_TEST(testGetStatusAllSatisfied);

  CPPUNIT_TEST(testGetCombinationCouldSatisfyNotEnabledOnline);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyNotEnabledInterline);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyAllSatisfiedOnline);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyAllSatisfiedInterline);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyNeedOptionsOnlineLH);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyNeedOptionsOnlineAA);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyNeedOptionsInterline);

  CPPUNIT_TEST(testGetPQItemCouldSatisfyCouldNotDetect);
  CPPUNIT_TEST(testGetPQItemCouldSatisfyNotOnline);
  CPPUNIT_TEST(testGetPQItemCouldSatisfyOnline);

  CPPUNIT_TEST(testIsEffectiveEnabledCapable);
  CPPUNIT_TEST(testIsEffectiveEnabledNotCapable);
  CPPUNIT_TEST(testIsEffectiveNotEnabledCapable);
  CPPUNIT_TEST(testIsEffectiveNotEnabledNotCapable);

  CPPUNIT_TEST(testPrintNotEffective);
  CPPUNIT_TEST(testPrintOK);
  CPPUNIT_TEST(testPrintOKWithHdm);

  CPPUNIT_TEST_SUITE_END();

public:
  DmcNSPerCarrierRequirementTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    TSE_ASSERT(_trx);
    initTrx();

    _dc = new DiagCollector();
    _dc->activate();

    _diversityTestUtil = _memHandle.create<shpq::DiversityTestUtil>(_trx->diversity());
    _diversityTestUtil->setNonStopOptionsPerCarrierEnabled(true);

    _stats = _memHandle.create<ItinStatisticMock>(*_trx);
    _context = _memHandle.create<ContextMock>(_trx->diversity(), *_stats, _dc, *_trx);

    _nsPerCarrier.clear();
    _sopVec.clear();
  }

  void tearDown()
  {
    _memHandle.clear();
    delete _dc;
  }

  void testGetStatusNotEnabled()
  {
    initNotEnabled();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getStatus());
  }

  void testGetStatusNeedOptions()
  {
    initNeedOptions();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_NONSTOPS_CARRIERS), _requirement->getStatus());
  }

  void testGetStatusAllSatisfied()
  {
    initAllSatisfied();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getStatus());
  }

  void testGetCombinationCouldSatisfyNotEnabledOnline()
  {
    initNotEnabled();
    initOnline();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(_sopVec, 0.0));
  }

  void testGetCombinationCouldSatisfyAllSatisfiedOnline()
  {
    initAllSatisfied();
    initOnline();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(_sopVec, 0.0));
  }

  void testGetCombinationCouldSatisfyNeedOptionsOnlineLH()
  {
    initNeedOptions();
    initOnline();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_NONSTOPS_CARRIERS),
                         _requirement->getCombinationCouldSatisfy(_sopVec, 0.0));
  }

  void testGetCombinationCouldSatisfyNeedOptionsOnlineAA()
  {
    initNeedOptions();
    initOnline();
    _sopVec[0] = 0;
    _sopVec[1] = 1;
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(_sopVec, 0.0));
  }

  void testGetCombinationCouldSatisfyNotEnabledInterline()
  {
    initNotEnabled();
    initInterline();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(_sopVec, 0.0));
  }

  void testGetCombinationCouldSatisfyAllSatisfiedInterline()
  {
    initAllSatisfied();
    initInterline();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(_sopVec, 0.0));
  }

  void testGetCombinationCouldSatisfyNeedOptionsInterline()
  {
    initNeedOptions();
    initInterline();
    initRequirement();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(_sopVec, 0.0));
  }

  void testIsOptNeededForNSCarrierGeneral()
  {
    initNeedOptions();
    initRequirement();

    CPPUNIT_ASSERT(_requirement->isOptNeededForNSCarrier());
  }

  void testIsOptNeededForNSCarrierSpecific()
  {
    initNeedOptions();
    initRequirement();

    CPPUNIT_ASSERT(!_requirement->isOptNeededForNSCarrier("AA"));
    CPPUNIT_ASSERT(_requirement->isOptNeededForNSCarrier("LH"));
    CPPUNIT_ASSERT(!_requirement->isOptNeededForNSCarrier("GG"));
  }

  void testGetPQItemCouldSatisfyCouldNotDetect()
  {
    using namespace shpq;

    std::map<ItinIndex::Key, CarrierCode> carrierMap;
    carrierMap[0] = "LH";
    _trx->diversity().setCarrierMap(carrierMap);
    initRequirement();

    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::SP_LEVEL, SoloPQItem::CR_LEVEL,
                                             SoloPQItem::FMP_LEVEL };
    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    for (const SoloPQItem::SoloPQItemLevel level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<CarrierTestPQItem>(score, level, pattern);

      initNeedOptions();
      CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_NONSTOPS_CARRIERS),
                           _requirement->getPQItemCouldSatisfy(pqItem));

      initAllSatisfied();
      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetPQItemCouldSatisfyNotOnline()
  {
    using namespace shpq;
    initRequirement();

    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::CRC_LEVEL, SoloPQItem::FPF_LEVEL };
    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    for (const SoloPQItem::SoloPQItemLevel level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<CarrierTestPQItem>(score, level, pattern);

      initNeedOptions();
      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));

      initAllSatisfied();
      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetPQItemCouldSatisfyOnline()
  {
    using namespace shpq;

    initRequirement();

    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::CRC_LEVEL, SoloPQItem::FPF_LEVEL };
    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    for (const SoloPQItem::SoloPQItemLevel level : levels)
    {
      CarrierTestPQItem* pqItem = _memHandle.create<CarrierTestPQItem>(score, level, pattern);
      pqItem->addApplicableCxr("LH");

      initNeedOptions();
      CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_NONSTOPS_CARRIERS),
                           _requirement->getPQItemCouldSatisfy(pqItem));

      initAllSatisfied();
      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testIsEffectiveEnabledCapable()
  {
    _diversityTestUtil->addDirectCarrier("LH");
    initRequirement();

    CPPUNIT_ASSERT(_requirement->isEffective());
  }

  void testIsEffectiveEnabledNotCapable()
  {
    initRequirement();

    CPPUNIT_ASSERT(!_requirement->isEffective());
  }

  void testIsEffectiveNotEnabledCapable()
  {
    _diversityTestUtil->addDirectCarrier("LH");
    initNotEnabled();
    initRequirement();

    CPPUNIT_ASSERT(!_requirement->isEffective());
  }

  void testIsEffectiveNotEnabledNotCapable()
  {
    initNotEnabled();
    initRequirement();

    CPPUNIT_ASSERT(!_requirement->isEffective());
  }

  void testPrintNotEffective()
  {
    initNotEnabled();
    initRequirement();
    _requirement->print();

    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());
  }

  void testPrintOK()
  {
    _diversityTestUtil->addDirectCarrier("AA");
    _diversityTestUtil->addDirectCarrier("LH");
    _diversityTestUtil->setHighDensityMarket(false);
    initNeedOptions();

    initRequirement();
    _requirement->print();

    std::string expected = "\tNS Carriers: AA[1/1] LH[0/1]\n";

    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
  }

  void testPrintOKWithHdm()
  {
    _diversityTestUtil->addDirectCarrier("AA");
    _diversityTestUtil->addDirectCarrier("LH");
    _diversityTestUtil->setHighDensityMarket(true);
    _diversityTestUtil->setMaxNonStopCountForCarrier("AA", 7);
    _diversityTestUtil->setMaxNonStopCountForCarrier("LH", 3);
    initNeedOptions();

    initRequirement();
    _requirement->print();

    std::string expected = "\tNS Carriers: AA[1/7] LH[0/3]\n";

    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
  }

private:
  TestMemHandle _memHandle;

  ShoppingTrx* _trx;
  DiagCollector* _dc;
  ItinStatisticMock* _stats;
  DmcRequirementsSharedContext* _context;
  DmcNSPerCarrierRequirement* _requirement;

  shpq::DiversityTestUtil* _diversityTestUtil;

  std::map<CarrierCode, size_t> _nsPerCarrier;
  std::vector<int> _sopVec;

  void initTrx()
  {
    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();
  }

  void initRequirement()
  {
    _requirement = _memHandle.create<DmcNSPerCarrierRequirement>(*_context);
  }

  void initNotEnabled() { _diversityTestUtil->setNonStopOptionsPerCarrierEnabled(false); }

  void initAllSatisfied()
  {
    _nsPerCarrier["AA"] = 1;
    _nsPerCarrier["LH"] = 1;
    _stats->setNumOfNonStopItinsPerCarrier(_nsPerCarrier);
  }

  void initNeedOptions()
  {
    _nsPerCarrier["AA"] = 1;
    _nsPerCarrier["LH"] = 0;
    _stats->setNumOfNonStopItinsPerCarrier(_nsPerCarrier);
  }

  void initOnline()
  {
    _sopVec.resize(2);
    _sopVec[0] = 1;
    _sopVec[1] = 0;
  }

  void initInterline()
  {
    _sopVec.resize(2);
    _sopVec[0] = 0;
    _sopVec[1] = 0;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DmcNSPerCarrierRequirementTest);

} // tse
