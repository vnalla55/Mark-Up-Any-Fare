#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingRequest.h"
#include "Diagnostic/Diag601Collector.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PUPQItem.h"
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace boost::assign;

class PricingUnitFactory_buildFareUsageTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingUnitFactory_buildFareUsageTest);

  CPPUNIT_TEST(testSetUpNewFUforCat31ExpndKeep);
  CPPUNIT_TEST(testSetUpNewFUforCat31Keep);

  CPPUNIT_TEST(testBuildFareUsage_PassCmdPricing);
  CPPUNIT_TEST(testBuildFareUsage_FailNotCheapest);
  CPPUNIT_TEST(testBuildFareUsage_FailNotCmdPricing);

  CPPUNIT_TEST_SUITE_END();

public:
  PricingUnitFactory* _factory;
  TestMemHandle _memH;
  DiagCollector* _diag;

  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _factory = _memH.create<MockPricingUnitFactory>();
    _factory->_paxType = createPaxType(ADULT);
    _factory->_trx = _memH.create<PricingTrx>();
    PricingRequest* req = _memH.create<PricingRequest>();
    req->ticketingAgent() = _memH.create<Agent>();
    _factory->_trx->setRequest(req);
    _factory->_trx->setOptions(_memH.create<PricingOptions>());
    _factory->_trx->ticketingDate() = DateTime(2010, 10, 10);
    _factory->_itin = _memH.create<Itin>();
    _diag = _memH.create<Diag601Collector>();
    _diag->activate();
  }

  void tearDown() { _memH.clear(); }

  FareMarket* createFareMarket()
  {
    FareMarket* fm = _memH.create<FareMarket>();
    fm->paxTypeCortege().resize(1);
    return fm;
  }

  PaxType* createPaxType(PaxTypeCode code)
  {
    PaxType* pt = _memH.create<PaxType>();
    pt->paxType() = code;
    pt->paxTypeInfo() = _memH.create<PaxTypeInfo>();
    return pt;
  }

  PaxTypeFare& createPaxTypeFare(CarrierCode cc = "CX")
  {
    FareInfo* fi = _memH.create<FareInfo>();
    fi->carrier() = cc;
    fi->currency() = NUC;
    fi->fareClass() = "FBC";
    Fare* f = _memH.create<Fare>();
    f->setFareInfo(fi);
    PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
    ptf->setFare(f);
    ptf->fareMarket() = createFareMarket();
    FareClassAppInfo* fca = _memH.create<FareClassAppInfo>();
    fca->_fareType = "ABC";
    ptf->fareClassAppInfo() = fca;
    ptf->paxType() = createPaxType(ADULT);
    ptf->nucFareAmount() = 100;
    return *ptf;
  }

  PaxTypeFare& createDummyPaxTypeFare()
  {
    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->fareBasisCode() = "ABC";
    ptf.fareMarket()->fareCalcFareAmt() = NUC;
    return ptf;
  }

  AirSeg* createAirSeg() { return _memH.create<AirSeg>(); }

  void populateRexPricingTrx()
  {
    RexPricingTrx* t = _memH.create<RexPricingTrx>();
    t->trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    _factory->_trx = t;
  }

  PaxTypeFare& createKeepPaxTypeFare()
  {
    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.retrievalInfo() = _memH.create<FareMarket::RetrievalInfo>();
    ptf.retrievalInfo()->_flag = FareMarket::RetrievKeep;
    return ptf;
  }

  PaxTypeFare& createExpndKeepPTF()
  {
    PaxTypeFare& ptf = *_memH.create<PaxTypeFare>();
    ptf.retrievalInfo() = _memH.create<FareMarket::RetrievalInfo>();
    ptf.retrievalInfo()->_flag = FareMarket::RetrievExpndKeep;
    return ptf;
  }

  PaxTypeFare* createCmdPricingPtf()
  {
    PU* pu = _memH.create<PU>();
    MergedFareMarket* mfm = _memH.create<MergedFareMarket>();
    _factory->pu() = pu;
    pu->fareMarket().push_back(mfm);
    pu->fareDirectionality().push_back(TO);
    PaxType* pt = _memH.create<PaxType>();
    pt->paxType() = ADULT;
    _factory->paxType() = pt;
    PaxTypeBucket* ptc = _memH.create<PaxTypeBucket>();
    ptc->requestedPaxType() = pt;
    PaxTypeFare& ptf = createPaxTypeFare();
    ptc->paxTypeFare().push_back(&ptf);
    FareMarket* fm = _memH.create<FareMarket>();
    ptf.fareMarket() = fm;
    mfm->mergedFareMarket().push_back(fm);
    fm->paxTypeCortege().push_back(*ptc);
    fm->fareBasisCode() = "FBC";
    fm->fbcUsage() = COMMAND_PRICE_FBC;

    return &ptf;
  }

  void testSetUpNewFUforCat31ExpndKeep() { assertSetUpNewFUforCat31(createExpndKeepPTF()); }

  void testSetUpNewFUforCat31Keep() { assertSetUpNewFUforCat31(createKeepPaxTypeFare()); }

  void assertSetUpNewFUforCat31(const PaxTypeFare& ptf)
  {
    populateRexPricingTrx();
    PricingUnit pu;
    FareUsage fu;
    CPPUNIT_ASSERT(!fu.isKeepFare());
    CPPUNIT_ASSERT(!pu.hasKeepFare());
    CPPUNIT_ASSERT(!fu.ruleFailed());
    CPPUNIT_ASSERT(!pu.ruleFailedButSoftPassForKeepFare());
    _factory->setUpNewFUforCat31(fu, pu, ptf);
    CPPUNIT_ASSERT(fu.isKeepFare());
    CPPUNIT_ASSERT(pu.hasKeepFare());
    CPPUNIT_ASSERT(fu.ruleFailed());
    CPPUNIT_ASSERT(pu.ruleFailedButSoftPassForKeepFare());
  }

  void testBuildFareUsage_PassCmdPricing()
  {
    bool found = false;
    createCmdPricingPtf();

    CPPUNIT_ASSERT(_factory->buildFareUsage(
        0, 0, 0, found, *_factory->constructPUPQItem(), 0, false, "FT", *_diag, false, false));
  }

  void testBuildFareUsage_FailNotCheapest()
  {
    bool found = false;

    CPPUNIT_ASSERT(!_factory->buildFareUsage(
        createCmdPricingPtf(), 0, 0, found, *_factory->constructPUPQItem(), 0, false, "FT", *_diag, false, false));
  }

  void testBuildFareUsage_FailNotCmdPricing()
  {
    bool found = false;
    PaxTypeFare* ptf = createCmdPricingPtf();
    ptf->fareMarket()->fbcUsage() = ' ';

    CPPUNIT_ASSERT(!_factory->buildFareUsage(
        0, 0, 0, found, *_factory->constructPUPQItem(), 0, false, "FT", *_diag, false, false));
  }

  class MockPricingUnitFactory : public PricingUnitFactory
  {
  protected:
    virtual bool isFareValidForBuildFareUsage(PaxTypeFare* paxTypeFare,
                                              const PaxTypeFare* primaryPaxTypeFare,
                                              PaxTypeFare*& lowestPaxTypeFareForCP,
                                              const uint16_t mktIdx,
                                              bool& fareFound,
                                              PUPQItem& pupqItem,
                                              bool seekCxrFare,
                                              const FareType& fareType,
                                              bool fxCnException,
                                              bool isCmdPricingFM,
                                              PaxTypeBucket* paxTypeCortege,
                                              FareMarket* fm,
                                              DiagCollector& diag) const
    {
      return fareFound = (paxTypeFare->carrier() == "AA");
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingUnitFactory_buildFareUsageTest);
}
