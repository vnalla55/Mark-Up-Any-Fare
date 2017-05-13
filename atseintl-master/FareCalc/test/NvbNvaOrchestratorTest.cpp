#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "FareCalc/NvbNvaOrchestrator.h"

#include "Common/TseConsts.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "FareCalc/CalcTotals.h"
#include "DataModel/PricingUnit.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/NvbNvaInfo.h"
#include "DBAccess/NvbNvaSeg.h"
#include "Rules/RuleConst.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/EndOnEnd.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "test/testdata/TestLocFactory.h"

namespace
{
typedef std::map<int16_t, tse::DateTime> DatesMap;
inline std::ostream& operator<<(std::ostream& os, const DatesMap& m)
{
  os << "\n";
  for (DatesMap::const_iterator it = m.begin(), itEnd = m.end(); it != itEnd; ++it)
    os << it->first << " " << it->second << "\n";

  return os;
}
}

namespace tse
{

class NvbNvaOrchestratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NvbNvaOrchestratorTest);
  CPPUNIT_TEST(testProcess_ActiveNetRemitNvbNvaNotAbacus);
  CPPUNIT_TEST(testProcess_ActiveNetRemitNvbNvaAbacus);

  CPPUNIT_TEST(testProcess_SpecificRuleMatch);
  CPPUNIT_TEST(testProcess_SpecificRuleNoMatch);
  CPPUNIT_TEST(testProcess_SpecificRuleEmpty_AnyRuleMatch);
  CPPUNIT_TEST(testProcess_SpecificRuleEmpty_AnyRuleEmpty);
  CPPUNIT_TEST(testProcess_SpecificRuleEmpty_AnyRuleNoMatch);

  CPPUNIT_TEST(testAreAllFaresWithoutPenalties_FailCat16);
  CPPUNIT_TEST(testAreAllFaresWithoutPenalties_Pass);
  CPPUNIT_TEST(testAreAllFaresSmf_Fail);
  CPPUNIT_TEST(testAreAllFaresSmf_Pass);

  CPPUNIT_TEST(testSuppressNvbNva_Fu1);
  CPPUNIT_TEST(testSuppressNvbNva_Fu2);

  CPPUNIT_TEST(testProcessRecord_Nvb1stSector);
  CPPUNIT_TEST(testProcessRecord_Nvb1stIntlSector);
  CPPUNIT_TEST(testProcessRecord_NvbEntireJourney);
  CPPUNIT_TEST(testProcessRecord_NvbEntireOutbound);
  CPPUNIT_TEST(testProcessRecord_Nva1stSectorEarliest);
  CPPUNIT_TEST(testProcessRecord_Nva1stIntlSectorEarliest);
  CPPUNIT_TEST(testProcessRecord_NvaEntireOutboundEarliest);

  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEmpty_NvbEmpty);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEmpty_Nvb1stSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEmpty_Nvb1stIntlSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEmpty_NvbEntireJourney);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEmpty_NvbEntireOutbound);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stSector_NvbEmpty);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stIntlSector_NvbEmpty);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireJourney_NvbEmpty);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireOutbound_NvbEmpty);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stSector_Nvb1stSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stSector_Nvb1stIntlSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stSector_NvbEntireJourney);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stSector_NvbEntireOutbound);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stIntlSector_Nvb1stSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireJourney_Nvb1stSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireOutbound_Nvb1stSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stIntlSector_Nvb1stIntlSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stIntlSector_NvbEntireJourney);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nvb1stIntlSector_NvbEntireOutbound);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireJourney_Nvb1stIntlSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireOutbound_Nvb1stIntlSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireJourney_NvbEntireJourney);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireJourney_NvbEntireOutbound);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireOutbound_NvbEntireJourney);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvbEntireOutbound_NvbEntireOutbound);

  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvaEmpty_NvaEmpty);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvaEmpty_Nva1stSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvaEmpty_Nva1stIntlSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvaEmpty_NvaEntireOutbound);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nva1stSector_Nva1stSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nva1stSector_Nva1stIntlSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nva1stSector_NvaEntireOutbound);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nva1stIntlSector_Nva1stIntlSector);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_Nva1stIntlSector_NvaEntireOutbound);
  CPPUNIT_TEST(testSetWinningNvbNvaFlag_NvaEntireOutbound_NvaEntireOutbound);

  CPPUNIT_TEST(testGetEndOnEndRequiredFareUsages_Pass);
  CPPUNIT_TEST(testGetEndOnEndRequiredFareUsages_Fail_EoeNotRequired);
  CPPUNIT_TEST(testGetEndOnEndRequiredFareUsages_Fail_NotMatchedCombination);

  CPPUNIT_TEST(testBuildVirtualPu_noEndOnEnd);
  CPPUNIT_TEST(testBuildVirtualPu_withEndOnEnd);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _dataHandleMock = _memHandle.create<NvbNvaDataHandleMock>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    PricingRequest* req = _memHandle.create<PricingRequest>();
    _trx->setRequest(req);
    Agent* agent = req->ticketingAgent() = _memHandle.create<Agent>();
    agent->agentCity() = "SIN";
    agent->agentTJR() = _memHandle.create<Customer>();

    _farePath = _memHandle.create<FarePath>();

    Itin* itin = _memHandle.create<Itin>();
    _farePath->itin() = itin;

    _calcTotals = _memHandle.create<CalcTotals>();

    setMapDays(_calcTotals->tvlSegNVB, 1, 1, 1, 1);
    setMapDays(_calcTotals->tvlSegNVA, 11, 12, 13, 14);

    _nvb = _memHandle.insert(new NvbNvaOrchestrator(*_trx, *_farePath, *_calcTotals));

    TrxUtil::enableAbacus();
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";

    createPricingUnit();
  }

  void tearDown() { _memHandle.clear(); }

  // TESTS

  void testProcess_ActiveNetRemitNvbNvaNotAbacus()
  {
    createTfdpsc();
    TrxUtil::disableAbacus();
    setMapDays(_calcTotals->tvlSegNVA, 23, 25, 27, 29);

    _nvb->process();

    CPPUNIT_ASSERT_EQUAL(createMapDays(23, 23, 23, 23), _calcTotals->tvlSegNVB);
    CPPUNIT_ASSERT_EQUAL(createMapDays(23, 25, 27, 29), _calcTotals->tvlSegNVA);
  }

  void testProcess_ActiveNetRemitNvbNvaAbacus()
  {
    createTfdpsc();
    setMapDays(_calcTotals->tvlSegNVA, 23, 25, 27, 29);

    _nvb->process();

    CPPUNIT_ASSERT_EQUAL(createMapDays(-1, -1, 23, 23), _calcTotals->tvlSegNVB);
    CPPUNIT_ASSERT_EQUAL(createMapDays(-1, -1, 27, 29), _calcTotals->tvlSegNVA);
  }

  void testProcess_SpecificRuleMatch()
  {
    _nvb->process();

    CPPUNIT_ASSERT_EQUAL(createMapDays(23, 23, 23, 23), _calcTotals->tvlSegNVB);
  }

  void testProcess_SpecificRuleNoMatch()
  {
    _dataHandleMock->infos().front()->segs().front()->fareBasis() = "XX";

    _nvb->process();

    CPPUNIT_ASSERT_EQUAL(createMapDays(1, 1, 1, 1), _calcTotals->tvlSegNVB);
  }

  void testProcess_SpecificRuleEmpty_AnyRuleMatch()
  {
    _dataHandleMock->infos().clear();

    _nvb->process();

    CPPUNIT_ASSERT_EQUAL(createMapDays(1, 23, 1, 1), _calcTotals->tvlSegNVB);
  }

  void testProcess_SpecificRuleEmpty_AnyRuleEmpty()
  {
    _dataHandleMock->infos().clear();
    _dataHandleMock->infosForAnyRule().clear();

    _nvb->process();

    CPPUNIT_ASSERT_EQUAL(createMapDays(1, 1, 1, 1), _calcTotals->tvlSegNVB);
  }

  void testProcess_SpecificRuleEmpty_AnyRuleNoMatch()
  {
    _dataHandleMock->infos().clear();
    _dataHandleMock->infosForAnyRule().front()->segs().front()->fareBasis() = "XX";

    _nvb->process();

    CPPUNIT_ASSERT_EQUAL(createMapDays(1, 1, 1, 1), _calcTotals->tvlSegNVB);
  }

  void testAreAllFaresWithoutPenalties_FailCat16()
  {
    _pu->fareUsage().front()->changePenaltyApply() = true;
    _pu->fareUsage().back()->changePenaltyApply() = true;

    CPPUNIT_ASSERT(!_nvb->areAnyFaresWithoutPenalties(_pu->fareUsage()));
  }

  void testAreAllFaresWithoutPenalties_Pass()
  {
    CPPUNIT_ASSERT(_nvb->areAnyFaresWithoutPenalties(_pu->fareUsage()));
  }

  void testAreAllFaresSmf_Fail()
  {
    _pu->fareUsage().push_back(createFareUsage(false));

    CPPUNIT_ASSERT(!_nvb->areAllFaresSmf(_pu->fareUsage()));
  }

  void testAreAllFaresSmf_Pass() { CPPUNIT_ASSERT(_nvb->areAllFaresSmf(_pu->fareUsage())); }

  void testSuppressNvbNva_Fu1()
  {
    _nvb->suppressNvbNva(1, 2);
    CPPUNIT_ASSERT_EQUAL(createMapDays(-1, -1, 1, 1), _calcTotals->tvlSegNVB);
    CPPUNIT_ASSERT_EQUAL(createMapDays(-1, -1, 13, 14), _calcTotals->tvlSegNVA);
  }

  void testSuppressNvbNva_Fu2()
  {
    _nvb->suppressNvbNva(3, 4);
    CPPUNIT_ASSERT_EQUAL(createMapDays(1, 1, -1, -1), _calcTotals->tvlSegNVB);
    CPPUNIT_ASSERT_EQUAL(createMapDays(11, 12, -1, -1), _calcTotals->tvlSegNVA);
  }

  void testProcessRecord_Nvb1stSector()
  {
    _nvb->processNvb(_pu->fareUsage(), NVB_1ST_SECTOR);

    CPPUNIT_ASSERT_EQUAL(createMapDays(23, 1, 1, 1), _calcTotals->tvlSegNVB);
  }

  void testProcessRecord_Nvb1stIntlSector()
  {
    _nvb->processNvb(_pu->fareUsage(), NVB_1ST_INTL_SECTOR);

    CPPUNIT_ASSERT_EQUAL(createMapDays(1, 23, 1, 1), _calcTotals->tvlSegNVB);
  }

  void testProcessRecord_NvbEntireOutbound()
  {
    _nvb->processNvb(_pu->fareUsage(), NVB_ENTIRE_OUTBOUND);

    CPPUNIT_ASSERT_EQUAL(createMapDays(23, 23, 1, 1), _calcTotals->tvlSegNVB);
  }

  void testProcessRecord_NvbEntireJourney()
  {
    _nvb->processNvb(_pu->fareUsage(), NVB_ENTIRE_JOURNEY);

    CPPUNIT_ASSERT_EQUAL(createMapDays(23, 23, 23, 23), _calcTotals->tvlSegNVB);
  }

  void testProcessRecord_Nva1stSectorEarliest()
  {
    _nvb->processNva(_pu->fareUsage(), NVA_1ST_SECTOR_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(createMapDays(11, 12, 13, 14), _calcTotals->tvlSegNVA);
  }

  void testProcessRecord_Nva1stIntlSectorEarliest()
  {
    _nvb->processNva(_pu->fareUsage(), NVA_1ST_INTL_SECTOR_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(createMapDays(11, 11, 13, 14), _calcTotals->tvlSegNVA);
  }

  void testProcessRecord_NvaEntireOutboundEarliest()
  {
    _nvb->processNva(_pu->fareUsage(), NVA_ENTIRE_OUTBOUND_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(createMapDays(11, 11, 13, 14), _calcTotals->tvlSegNVA);
  }

  void testSetWinningNvbNvaFlag_NvbEmpty_NvbEmpty()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_EMPTY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_EMPTY);
  }

  void testSetWinningNvbNvaFlag_NvbEmpty_Nvb1stSector()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_1ST_SECTOR);
  }

  void testSetWinningNvbNvaFlag_NvbEmpty_Nvb1stIntlSector()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_INTL_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_1ST_INTL_SECTOR);
  }

  void testSetWinningNvbNvaFlag_NvbEmpty_NvbEntireJourney()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_JOURNEY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_NvbEmpty_NvbEntireOutbound()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_OUTBOUND);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_OUTBOUND);
  }

  void testSetWinningNvbNvaFlag_Nvb1stSector_NvbEmpty()
  {
    Indicator ind = NVB_1ST_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_EMPTY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_1ST_SECTOR);
  }

  void testSetWinningNvbNvaFlag_Nvb1stIntlSector_NvbEmpty()
  {
    Indicator ind = NVB_1ST_INTL_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_EMPTY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_1ST_INTL_SECTOR);
  }

  void testSetWinningNvbNvaFlag_NvbEntireJourney_NvbEmpty()
  {
    Indicator ind = NVB_ENTIRE_JOURNEY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_EMPTY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_NvbEntireOutbound_NvbEmpty()
  {
    Indicator ind = NVB_ENTIRE_OUTBOUND;
    _nvb->setWinningNvbNvaFlag(ind, NVB_EMPTY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_OUTBOUND);
  }

  void testSetWinningNvbNvaFlag_Nvb1stSector_Nvb1stSector()
  {
    Indicator ind = NVB_1ST_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_1ST_SECTOR);
  }

  void testSetWinningNvbNvaFlag_Nvb1stSector_Nvb1stIntlSector()
  {
    Indicator ind = NVB_1ST_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_INTL_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_1ST_INTL_SECTOR);
  }

  void testSetWinningNvbNvaFlag_Nvb1stSector_NvbEntireJourney()
  {
    Indicator ind = NVB_1ST_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_JOURNEY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_Nvb1stSector_NvbEntireOutbound()
  {
    Indicator ind = NVB_1ST_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_OUTBOUND);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_OUTBOUND);
  }

  void testSetWinningNvbNvaFlag_Nvb1stIntlSector_Nvb1stSector()
  {
    Indicator ind = NVB_1ST_INTL_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_1ST_INTL_SECTOR);
  }

  void testSetWinningNvbNvaFlag_NvbEntireJourney_Nvb1stSector()
  {
    Indicator ind = NVB_ENTIRE_JOURNEY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_NvbEntireOutbound_Nvb1stSector()
  {
    Indicator ind = NVB_ENTIRE_OUTBOUND;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_OUTBOUND);
  }

  void testSetWinningNvbNvaFlag_Nvb1stIntlSector_Nvb1stIntlSector()
  {
    Indicator ind = NVB_1ST_INTL_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_INTL_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_1ST_INTL_SECTOR);
  }

  void testSetWinningNvbNvaFlag_Nvb1stIntlSector_NvbEntireJourney()
  {
    Indicator ind = NVB_1ST_INTL_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_JOURNEY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_Nvb1stIntlSector_NvbEntireOutbound()
  {
    Indicator ind = NVB_1ST_INTL_SECTOR;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_OUTBOUND);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_OUTBOUND);
  }

  void testSetWinningNvbNvaFlag_NvbEntireJourney_Nvb1stIntlSector()
  {
    Indicator ind = NVB_ENTIRE_JOURNEY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_INTL_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_NvbEntireOutbound_Nvb1stIntlSector()
  {
    Indicator ind = NVB_ENTIRE_OUTBOUND;
    _nvb->setWinningNvbNvaFlag(ind, NVB_1ST_INTL_SECTOR);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_OUTBOUND);
  }

  void testSetWinningNvbNvaFlag_NvbEntireJourney_NvbEntireJourney()
  {
    Indicator ind = NVB_ENTIRE_JOURNEY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_JOURNEY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_NvbEntireJourney_NvbEntireOutbound()
  {
    Indicator ind = NVB_ENTIRE_JOURNEY;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_OUTBOUND);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_NvbEntireOutbound_NvbEntireJourney()
  {
    Indicator ind = NVB_ENTIRE_OUTBOUND;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_JOURNEY);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_JOURNEY);
  }

  void testSetWinningNvbNvaFlag_NvbEntireOutbound_NvbEntireOutbound()
  {
    Indicator ind = NVB_ENTIRE_OUTBOUND;
    _nvb->setWinningNvbNvaFlag(ind, NVB_ENTIRE_OUTBOUND);

    CPPUNIT_ASSERT_EQUAL(ind, NVB_ENTIRE_OUTBOUND);
  }

  void testSetWinningNvbNvaFlag_NvaEmpty_NvaEmpty()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVA_EMPTY);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_EMPTY);
  }

  void testSetWinningNvbNvaFlag_NvaEmpty_Nva1stSector()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVA_1ST_SECTOR_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_1ST_SECTOR_EARLIEST);
  }

  void testSetWinningNvbNvaFlag_NvaEmpty_Nva1stIntlSector()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVA_1ST_INTL_SECTOR_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_1ST_INTL_SECTOR_EARLIEST);
  }

  void testSetWinningNvbNvaFlag_NvaEmpty_NvaEntireOutbound()
  {
    Indicator ind = NVB_EMPTY;
    _nvb->setWinningNvbNvaFlag(ind, NVA_ENTIRE_OUTBOUND_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_ENTIRE_OUTBOUND_EARLIEST);
  }

  void testSetWinningNvbNvaFlag_Nva1stSector_Nva1stSector()
  {
    Indicator ind = NVA_1ST_SECTOR_EARLIEST;
    _nvb->setWinningNvbNvaFlag(ind, NVA_1ST_SECTOR_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_1ST_SECTOR_EARLIEST);
  }

  void testSetWinningNvbNvaFlag_Nva1stSector_Nva1stIntlSector()
  {
    Indicator ind = NVA_1ST_SECTOR_EARLIEST;
    _nvb->setWinningNvbNvaFlag(ind, NVA_1ST_INTL_SECTOR_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_1ST_INTL_SECTOR_EARLIEST);
  }

  void testSetWinningNvbNvaFlag_Nva1stSector_NvaEntireOutbound()
  {
    Indicator ind = NVA_1ST_SECTOR_EARLIEST;
    _nvb->setWinningNvbNvaFlag(ind, NVA_ENTIRE_OUTBOUND_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_ENTIRE_OUTBOUND_EARLIEST);
  }

  void testSetWinningNvbNvaFlag_Nva1stIntlSector_Nva1stIntlSector()
  {
    Indicator ind = NVA_1ST_INTL_SECTOR_EARLIEST;
    _nvb->setWinningNvbNvaFlag(ind, NVA_1ST_INTL_SECTOR_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_1ST_INTL_SECTOR_EARLIEST);
  }

  void testSetWinningNvbNvaFlag_Nva1stIntlSector_NvaEntireOutbound()
  {
    Indicator ind = NVA_1ST_INTL_SECTOR_EARLIEST;
    _nvb->setWinningNvbNvaFlag(ind, NVA_ENTIRE_OUTBOUND_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_ENTIRE_OUTBOUND_EARLIEST);
  }

  void testSetWinningNvbNvaFlag_NvaEntireOutbound_NvaEntireOutbound()
  {
    Indicator ind = NVA_ENTIRE_OUTBOUND_EARLIEST;
    _nvb->setWinningNvbNvaFlag(ind, NVA_ENTIRE_OUTBOUND_EARLIEST);

    CPPUNIT_ASSERT_EQUAL(ind, NVA_ENTIRE_OUTBOUND_EARLIEST);
  }

  void testGetEndOnEndRequiredFareUsages_Pass()
  {
    std::vector<const FareUsage*> eoeFUVec;

    _pu->fareUsage().front()->endOnEndRequired() = true;
    CPPUNIT_ASSERT(_nvb->getEndOnEndRequiredFareUsages(
        _trx->dataHandle(), _pu->fareUsage().begin(), _pu->fareUsage(), eoeFUVec));
  }

  void testGetEndOnEndRequiredFareUsages_Fail_EoeNotRequired()
  {
    std::vector<const FareUsage*> eoeFUVec;

    CPPUNIT_ASSERT(!_nvb->getEndOnEndRequiredFareUsages(
        _trx->dataHandle(), _pu->fareUsage().begin(), _pu->fareUsage(), eoeFUVec));
  }

  void testGetEndOnEndRequiredFareUsages_Fail_NotMatchedCombination()
  {
    std::vector<const FareUsage*> eoeFUVec;
    _pu->fareUsage().front()->eoeRules().clear();

    CPPUNIT_ASSERT(!_nvb->getEndOnEndRequiredFareUsages(
        _trx->dataHandle(), _pu->fareUsage().begin(), _pu->fareUsage(), eoeFUVec));
  }

  void testBuildVirtualPu_noEndOnEnd()
  {
    commonForBuildVirtualPuTests(false);
    std::vector<FareUsage*> fus;
    _nvb->buildVirtualPu(fus);

    CPPUNIT_ASSERT(fus.empty());
  }

  void testBuildVirtualPu_withEndOnEnd()
  {
    commonForBuildVirtualPuTests(true);
    std::vector<FareUsage*> fus;
    _nvb->buildVirtualPu(fus);

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fus.size()), 4);
  }

protected:
  class NvbNvaDataHandleMock;
  NvbNvaDataHandleMock* _dataHandleMock;
  PricingTrx* _trx;
  FarePath* _farePath;
  CalcTotals* _calcTotals;
  PricingUnit* _pu;
  NvbNvaOrchestrator* _nvb;
  TestMemHandle _memHandle;

  NvbNvaInfo* createNvbNvaInfo(Indicator nvb = NVA_EMPTY, Indicator nva = NVA_EMPTY)
  {
    NvbNvaSeg* seg = new NvbNvaSeg();
    seg->fareBasis() = "FC";
    seg->nvb() = nvb;
    seg->nva() = nva;

    NvbNvaInfo* info = _memHandle.create<NvbNvaInfo>();
    info->segs().push_back(seg);
    return info;
  }

  void createPricingUnit()
  {
    _pu = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit().push_back(_pu);
    Itin* itin = _farePath->itin();

    AirSeg* airSeg;
    FareUsage* fu;

    // fareusage 1
    fu = createFareUsage(true, false, false); // SMF, outbound
    airSeg = _memHandle.create<AirSeg>();
    airSeg->segmentOrder() = 1;
    airSeg->departureDT() = DateTime(2012, 12, 23);
    airSeg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    airSeg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    fu->travelSeg().push_back(airSeg);
    itin->travelSeg().push_back(airSeg);

    airSeg = _memHandle.create<AirSeg>();
    airSeg->segmentOrder() = 2;
    airSeg->departureDT() = DateTime(2012, 12, 25);
    airSeg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    airSeg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
    fu->travelSeg().push_back(airSeg);
    itin->travelSeg().push_back(airSeg);
    _pu->fareUsage().push_back(fu);

    // fareusage 2
    fu = createFareUsage(true, true, false); // SMF, inbound
    airSeg = _memHandle.create<AirSeg>();
    airSeg->segmentOrder() = 3;
    airSeg->departureDT() = DateTime(2012, 12, 27);
    airSeg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
    airSeg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    fu->travelSeg().push_back(airSeg);
    itin->travelSeg().push_back(airSeg);
    airSeg = _memHandle.create<AirSeg>();
    airSeg->segmentOrder() = 4;
    airSeg->departureDT() = DateTime(2012, 12, 29);
    airSeg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    airSeg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    fu->travelSeg().push_back(airSeg);
    itin->travelSeg().push_back(airSeg);
    _pu->fareUsage().push_back(fu);
  }

  void createTfdpsc()
  {
    NegFareRestExtSeq* nfrExtSeq = _memHandle.create<NegFareRestExtSeq>();
    nfrExtSeq->suppressNvbNva() = YES;

    FareUsage* fu = _pu->fareUsage().front();
    fu->netRemitPscResults().push_back(FareUsage::TktNetRemitPscResult(
        fu->travelSeg().front(), fu->travelSeg().back(), nfrExtSeq, fu->paxTypeFare()));
  }

  FareUsage*
  createFareUsage(bool isSmf = true, bool isInbound = false, bool endOnEndRequired = true)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    fu->paxTypeFare() = ptf;
    fu->inbound() = isInbound;

    fu->endOnEndRequired() = endOnEndRequired;
    CombinabilityRuleItemInfo* c10Info = _memHandle.create<CombinabilityRuleItemInfo>();
    c10Info->setItemNo(9);
    fu->eoeRules().push_back(c10Info);

    if (endOnEndRequired)
    {
      CombinabilityRuleInfo* rec2Cat10 = _memHandle.create<CombinabilityRuleInfo>();
      rec2Cat10->eoeInd() = NvbNvaOrchestrator::EOE_IND_RESTRICTIONS;
      fu->rec2Cat10() = rec2Cat10;
    }

    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->carrier() = "CR";
    fareInfo->fareClass() = "FC";
    fareInfo->vendor() = isSmf ? SMF_ABACUS_CARRIER_VENDOR_CODE : ATPCO_VENDOR_CODE;
    fareInfo->fareTariff() = 1;
    fareInfo->ruleNumber() = "1234";
    fare->setFareInfo(fareInfo);
    ptf->setFare(fare);
    ptf->fareTypeApplication() = 'N';

    return fu;
  }

  const DateTime& firstDate()
  {
    return _pu->fareUsage().front()->travelSeg().front()->departureDT();
  }

  DatesMap& createMapDays(int day1, int day2, int day3, int day4)
  {
    DatesMap* m = _memHandle.create<DatesMap>();
    setMapDays(*m, day1, day2, day3, day4);

    return *m;
  }

  void setMapDays(DatesMap& m, int day1, int day2, int day3, int day4)
  {

    m[1] = day1 > 0 ? DateTime(2012, 12, day1) : DateTime::openDate();
    m[2] = day2 > 0 ? DateTime(2012, 12, day2) : DateTime::openDate();
    m[3] = day3 > 0 ? DateTime(2012, 12, day3) : DateTime::openDate();
    m[4] = day4 > 0 ? DateTime(2012, 12, day4) : DateTime::openDate();
  }

  void commonForBuildVirtualPuTests(bool addThirdPu)
  {
    FareUsage* fu;
    PricingUnit* pu;

    _farePath->pricingUnit().clear();

    pu = _memHandle.create<PricingUnit>();
    fu = createFareUsage(true, true, false); // SMF, inbound, no EOE
    pu->fareUsage().push_back(fu);
    fu = createFareUsage(false, true, false); // ATP, inbound, no EOE
    pu->fareUsage().push_back(fu);
    _farePath->pricingUnit().push_back(pu);

    pu = _memHandle.create<PricingUnit>();
    fu = createFareUsage(true, true, false); // SMF, inbound, no EOE
    pu->fareUsage().push_back(fu);
    _farePath->pricingUnit().push_back(pu);

    if (!addThirdPu)
      return;

    pu = _memHandle.create<PricingUnit>();
    fu = createFareUsage(true, true, false); // SMF, inbound, no EOE
    pu->fareUsage().push_back(fu);
    fu = createFareUsage(true, true, false); // SMF, inbound, no EOE
    pu->fareUsage().push_back(fu);
    fu = createFareUsage(false, true, true); // ATP, inbound, EOE
    pu->fareUsage().push_back(fu);
    _farePath->pricingUnit().push_back(pu);
  }

  class NvbNvaDataHandleMock : public DataHandleMock
  {
  public:
    NvbNvaDataHandleMock()
    {
      _infos.push_back(&_info);
      NvbNvaSeg* seg = new NvbNvaSeg();
      _info.segs().push_back(seg);
      seg->nvb() = NVB_ENTIRE_JOURNEY;
      seg->fareBasis() = "FC";
      _info.carrier() = "CR";
      _info.vendor() = SMF_ABACUS_CARRIER_VENDOR_CODE;
      _info.ruleTariff() = 1;
      _info.rule() = "1234";

      _infosForAnyRule.push_back(&_infoForAnyRule);
      seg = new NvbNvaSeg();
      _infoForAnyRule.segs().push_back(seg);
      seg->nvb() = NVB_1ST_INTL_SECTOR;
      seg->fareBasis() = "FC";
      _infoForAnyRule.carrier() = "CR";
      _infoForAnyRule.vendor() = SMF_ABACUS_CARRIER_VENDOR_CODE;
      _infoForAnyRule.ruleTariff() = 1;
      _infoForAnyRule.rule() = ANY_RULE;

      _eoe.eoeNormalInd() = 'R';
    }

    ~NvbNvaDataHandleMock() {}

    const std::vector<NvbNvaInfo*>& getNvbNvaInfo(const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const TariffNumber& tarrif,
                                                  const RuleNumber& rule)
    {
      if (rule == ANY_RULE)
        return _infosForAnyRule;
      else
        return _infos;
    }

    char getVendorType(const VendorCode& vendor)
    {
      return ('S' == vendor[0]) ? RuleConst::SMF_VENDOR : RuleConst::PUBLIC_VENDOR;
    }

    const EndOnEnd* getEndOnEnd(const VendorCode& vendor, const int itemNo) { return &_eoe; }

    std::vector<NvbNvaInfo*>& infos() { return _infos; }
    std::vector<NvbNvaInfo*>& infosForAnyRule() { return _infosForAnyRule; }

  private:
    std::vector<NvbNvaInfo*> _infos;
    NvbNvaInfo _info;
    EndOnEnd _eoe;
    std::vector<NvbNvaInfo*> _infosForAnyRule;
    NvbNvaInfo _infoForAnyRule;
    bool _calledWithAnyRule;
  }; // class NvbNvaDataHandleMock

}; // class NvbNvaOrchestratorTest

CPPUNIT_TEST_SUITE_REGISTRATION(NvbNvaOrchestratorTest);
}
