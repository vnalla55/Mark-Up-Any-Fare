#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DataModel/RexPricingOptions.h"

namespace tse
{

class RexExchangeTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexExchangeTrxTest);

  CPPUNIT_TEST(test_getMultiNewItinData);
  CPPUNIT_TEST(test_reissueOptionsMap);
  CPPUNIT_TEST(test_itinStatus);
  CPPUNIT_TEST(test_processTagPermutations);
  CPPUNIT_TEST(test_fareRetrievalFlags);
  CPPUNIT_TEST(test_repriceWithDiffDates);
  CPPUNIT_TEST(test_newItinKeepFares);
  CPPUNIT_TEST(test_newToExcItinFareMarketMapForKeep);
  CPPUNIT_TEST(test_needRetrieveKeepFareAnyItin_KeepNotNeeded);
  CPPUNIT_TEST(test_needRetrieveKeepFareAnyItin_1stItinNeedKeep);
  CPPUNIT_TEST(test_needRetrieveKeepFareAnyItin_2ndItinNeedKeep);
  CPPUNIT_TEST(test_setPnrSegmentCollocation_empty);
  CPPUNIT_TEST(test_setPnrSegmentCollocation_set);
  CPPUNIT_TEST(test_checkPnrSegmentCollocation_empty);
  CPPUNIT_TEST(test_checkPnrSegmentCollocation_notFound);
  CPPUNIT_TEST(test_checkPnrSegmentCollocation_found);

  CPPUNIT_TEST_SUITE_END();

public:
  RexExchangeTrxTest() { log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff()); }

  void setUp()
  {
    _trx.exchangeItin().push_back(&_excItin);
    _trx.itin().push_back(&_itin);
    _trx.itin().push_back(&_itin);
    _trx._pnrSegmentCollocation.clear();
    _trx.setOptions(new RexPricingOptions);
    _trx.ititializeMultiItinData();
    _fm.travelSeg().clear();
    _seg.departureDT() = DateTime(2016, 5, 13, 0, 0, 0);
    _fm.travelSeg().push_back(&_seg);
    _ptf.fareMarket() = &_fm;
  }

  void tearDown() { delete _trx.getOptions(); }

  void test_getMultiNewItinData()
  {
    _trx.setItinIndex(0);
    _trx.getMultiItinData().itinStatus() = RexExchangeTrx::REISSUE_RULES_FAIL;
    CPPUNIT_ASSERT(_trx.getMultiItinData().itinStatus() == RexExchangeTrx::REISSUE_RULES_FAIL);

    _trx.setItinIndex(1);
    CPPUNIT_ASSERT(_trx.getMultiItinData().itinStatus() == RexExchangeTrx::REISSUE_RULES_PASS);
  }

  void test_reissueOptionsMap()
  {
    _trx.setItinIndex(0);
    _trx.reissueOptions().insertOption(&_ptf, &_vcRec3);
    std::vector<ReissueOptions::R3WithDateRange> r3v;
    _trx.reissueOptions().getRec3s(&_ptf, r3v);
    CPPUNIT_ASSERT(!r3v.empty());

    _trx.setItinIndex(1);
    std::vector<ReissueOptions::R3WithDateRange> r3v_2;
    _trx.reissueOptions().getRec3s(&_ptf, r3v_2);
    CPPUNIT_ASSERT(r3v_2.empty());
  }

  void test_itinStatus()
  {
    _trx.setItinIndex(0);
    _trx.itinStatus() = RexExchangeTrx::REISSUE_RULES_FAIL;
    CPPUNIT_ASSERT(_trx.itinStatus() == RexExchangeTrx::REISSUE_RULES_FAIL);

    _trx.setItinIndex(1);
    CPPUNIT_ASSERT(_trx.itinStatus() == RexExchangeTrx::REISSUE_RULES_PASS);
  }

  void test_processTagPermutations()
  {
    _trx.setItinIndex(0);
    ProcessTagPermutation p;
    _trx.processTagPermutations().push_back(&p);
    CPPUNIT_ASSERT(!_trx.processTagPermutations().empty());

    _trx.setItinIndex(1);
    CPPUNIT_ASSERT(_trx.processTagPermutations().empty());
  }

  void test_fareRetrievalFlags()
  {
    _trx.setItinIndex(0);
    _trx.markFareRetrievalMethodHistorical();
    CPPUNIT_ASSERT(_trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(_trx.fareRetrievalFlags().isSet(FareMarket::RetrievHistorical) == true);

    _trx.setItinIndex(1);
    CPPUNIT_ASSERT(_trx.fareRetrievalFlags().isNull() == true);
  }

  void test_repriceWithDiffDates()
  {
    _trx.setItinIndex(0);
    _trx.repriceWithDiffDates() = true;
    CPPUNIT_ASSERT(_trx.repriceWithDiffDates() == true);

    _trx.setItinIndex(1);
    CPPUNIT_ASSERT(_trx.repriceWithDiffDates() == false);
  }

  void test_newItinKeepFares()
  {
    _trx.setItinIndex(0);
    FareMarket fm1;
    const PaxTypeFare ptf1, ptf2;
    _trx.newItinKeepFares()[&ptf1] = &fm1;
    CPPUNIT_ASSERT(!_trx.newItinKeepFares().empty());

    _trx.setItinIndex(1);
    CPPUNIT_ASSERT(_trx.newItinKeepFares().empty());
  }

  void test_newToExcItinFareMarketMapForKeep()
  {
    _trx.setItinIndex(0);
    FareMarket fm1, fm2;
    _trx.newToExcItinFareMarketMapForKeep().insert(std::make_pair(&fm1, &fm2));
    CPPUNIT_ASSERT(!_trx.newToExcItinFareMarketMapForKeep().empty());

    _trx.setItinIndex(1);
    CPPUNIT_ASSERT(_trx.newToExcItinFareMarketMapForKeep().empty());
  }
  void test_needRetrieveKeepFareAnyItin_KeepNotNeeded()
  {
    _trx.setItinIndex(0);
    _trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);

    _trx.setItinIndex(1);
    _trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);

    CPPUNIT_ASSERT(false == _trx.needRetrieveKeepFareAnyItin());
  }
  void test_needRetrieveKeepFareAnyItin_1stItinNeedKeep()
  {
    _trx.setItinIndex(0);
    _trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);

    _trx.setItinIndex(1);
    _trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);

    CPPUNIT_ASSERT(_trx.needRetrieveKeepFareAnyItin());
  }
  void test_needRetrieveKeepFareAnyItin_2ndItinNeedKeep()
  {
    _trx.setItinIndex(0);
    _trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);

    _trx.setItinIndex(1);
    _trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);

    CPPUNIT_ASSERT(_trx.needRetrieveKeepFareAnyItin());
  }

  void test_setPnrSegmentCollocation_empty()
  {
    CPPUNIT_ASSERT(_trx.getPnrSegmentCollocation().empty());
  }

  void test_setPnrSegmentCollocation_set()
  {
    _trx.setPnrSegmentCollocation(1, 4);

    CPPUNIT_ASSERT(!_trx.getPnrSegmentCollocation().empty());
  }

  void test_checkPnrSegmentCollocation_empty()
  {
    CPPUNIT_ASSERT(!_trx.checkPnrSegmentCollocation(1, 4));
  }

  void test_checkPnrSegmentCollocation_notFound()
  {
    _trx.setPnrSegmentCollocation(1, 4);
    _trx.setPnrSegmentCollocation(1, 5);
    _trx.setPnrSegmentCollocation(7, 1);

    CPPUNIT_ASSERT(!_trx.checkPnrSegmentCollocation(1, 7));
  }

  void test_checkPnrSegmentCollocation_found()
  {
    _trx.setPnrSegmentCollocation(1, 4);
    _trx.setPnrSegmentCollocation(1, 5);
    _trx.setPnrSegmentCollocation(7, 1);

    CPPUNIT_ASSERT(_trx.checkPnrSegmentCollocation(1, 4));
  }

protected:
  ExcItin _excItin;
  Itin _itin;
  RexExchangeTrx _trx;
  VoluntaryChangesInfo _vcRec3;
  PaxTypeFare _ptf;
  FareMarket _fm;
  AirSeg _seg;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RexExchangeTrxTest);
}
