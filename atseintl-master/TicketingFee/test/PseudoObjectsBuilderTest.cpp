#include "test/include/CppUnitHelperMacros.h"

#include "test/include/TestMemHandle.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "TicketingFee/PseudoObjectsBuilder.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagCollector.h"

#include "test/testdata/TestAirSegFactory.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/TktFeesRequest.h"
#include "test/include/MockGlobal.h"
#include "DBAccess/DiskCache.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class PseudoObjectsBuilderStub : public PseudoObjectsBuilder
{
public:
  PseudoObjectsBuilderStub(TktFeesPricingTrx& trx) : PseudoObjectsBuilder(trx) {}

  ~PseudoObjectsBuilderStub() {}
};

class PseudoObjectsBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PseudoObjectsBuilderTest);
  CPPUNIT_TEST(testInitAccountCodes_validTtktDesignators);
  CPPUNIT_TEST(testInitAccountCodes_validCorpId);
  CPPUNIT_TEST(testInitAccountCodes_invalidCorpId);
  CPPUNIT_TEST(testInitAccountCodes_validAccountCode);
  CPPUNIT_TEST(testInitFOPBinNumber_valid);
  CPPUNIT_TEST(testDiag881_No_PUs);
  CPPUNIT_TEST(testDiag881_PU_No_FU);
  CPPUNIT_TEST(testDiag881_PU_With_FU);
  CPPUNIT_TEST(testInsertFareUsageTvlSegToItsPU);
  CPPUNIT_TEST(testDiag881_printAccountCode);
  CPPUNIT_TEST(testDiag881_printCorpId);
  CPPUNIT_TEST(testDiag881_printTktDesignator);
  CPPUNIT_TEST(testDiag881_printFopBin);

  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  PseudoObjectsBuilderTest() {}

  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    buildTktFeesPricingTrx();
  }
  void tearDown() { _memHandle.clear(); }

protected:
  TktFeesPricingTrx* _trx;
  TktFeesRequest* _request;
  TestMemHandle _memHandle;
  Itin* _itin;
  PricingOptions* _options;
  PaxType _paxType;
  AirSeg* _tvlSegLaxSfo;
  AirSeg* _tvlSegSfoLax;

  TktFeesRequest::TktFeesFareBreakInfo* _fbi;
  TktFeesRequest::TktFeesFareBreakAssociation* _fba;
  TktFeesRequest::PaxTypePayment* _ptp;
  TktFeesRequest::PassengerPaymentInfo* _ppi;
  TktFeesRequest::FormOfPayment* _fop;
  void buildTktFeesPricingTrx()
  {
    _memHandle.get(_trx);
    _memHandle.get(_request);
    _memHandle.get(_itin);
    _memHandle.get(_options);

    _trx->setRequest(_request);
    _trx->itin().push_back(_itin);
    _trx->setOptions(_options);

    _tvlSegLaxSfo =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_SFO.xml");
    _tvlSegSfoLax =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegSFO_LAX.xml");

    _paxType.paxType() = "ADT";
    _trx->paxType().push_back(&_paxType);

    _memHandle.get(_fbi);
    _fbi->fareComponentID() = 1;
    _memHandle.get(_fba);
    _fba->segmentID() = 1;

    _memHandle.get(_ptp);
    _memHandle.get(_ppi);
    _memHandle.get(_fop);
  }
  void createDiag(DiagnosticTypes diagType)
  {
    _trx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      _trx->diagnostic().activate();
    }
  }

  // TESTS
public:
  void testInitAccountCodes_validTtktDesignators()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->tktDesignatorPerItin()[_itin][0] = "TKTDSG1";
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->tktDesignatorPerItin()[_itin][1] = "TKTDSG2";

    CPPUNIT_ASSERT_NO_THROW(builder.initAccountCodes(_itin));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->getRequest()->tktDesignator().size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("TKTDSG1"), _trx->getRequest()->tktDesignator()[0]);
    CPPUNIT_ASSERT_EQUAL(TktDesignator("TKTDSG2"), _trx->getRequest()->tktDesignator()[1]);
    CPPUNIT_ASSERT(!_trx->getRequest()->isMultiAccCorpId());
  }

  void testInitAccountCodes_validCorpId()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->corpIdPerItin()[_itin].push_back("CRP01");
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->corpIdPerItin()[_itin].push_back("CRP02");
    CPPUNIT_ASSERT_NO_THROW(builder.initAccountCodes(_itin));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->getRequest()->corpIdVec().size());
    CPPUNIT_ASSERT_EQUAL(std::string("CRP01"), _trx->getRequest()->corpIdVec()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("CRP02"), _trx->getRequest()->corpIdVec()[1]);
    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId());
  }

  void testInitAccountCodes_invalidCorpId()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->invalidCorpIdPerItin()[_itin].push_back(
        "CINV01");
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->invalidCorpIdPerItin()[_itin].push_back(
        "CINV02");

    CPPUNIT_ASSERT_NO_THROW(builder.initAccountCodes(_itin));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->getRequest()->incorrectCorpIdVec().size());
    CPPUNIT_ASSERT_EQUAL(std::string("CINV01"), _trx->getRequest()->incorrectCorpIdVec()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("CINV02"), _trx->getRequest()->incorrectCorpIdVec()[1]);
  }

  void testInitAccountCodes_validAccountCode()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->accountCodeIdPerItin()[_itin].push_back(
        "ACC11");
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->accountCodeIdPerItin()[_itin].push_back(
        "ACC22");

    CPPUNIT_ASSERT_NO_THROW(builder.initAccountCodes(_itin));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->getRequest()->accCodeVec().size());
    CPPUNIT_ASSERT_EQUAL(std::string("ACC11"), _trx->getRequest()->accCodeVec()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("ACC22"), _trx->getRequest()->accCodeVec()[1]);
    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId());
  }

  void testInitFOPBinNumber_valid()
  {
    PseudoObjectsBuilderStub builder(*_trx);

    _fop->fopBinNumber() = "111111";
    _ppi->fopVector().push_back(_fop);
    _ptp->ppiV().push_back(_ppi);

    TktFeesRequest* req = static_cast<TktFeesRequest*>(_trx->getRequest());
    req->paxTypePaymentPerItin()[_itin] = _ptp;

    CPPUNIT_ASSERT_NO_THROW(builder.initFOPBinNumber(_itin));
    CPPUNIT_ASSERT_EQUAL(FopBinNumber("111111"), _trx->getRequest()->formOfPayment());
  }

  void testDiag881_No_PUs()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    createDiag(Diagnostic881);
    FarePath* fp = 0;
    _memHandle.get(fp);
    builder._farePath = fp;

    builder.diag881(_paxType);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(
                       "**** PSEUDO FAREPATH DETAIL FOR PASSENGER = ADT ****") !=
                   std::string::npos);
  }

  void testDiag881_PU_No_FU()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    createDiag(Diagnostic881);
    FarePath* fp = 0;
    _memHandle.get(fp);
    builder._farePath = fp;
    PricingUnit pu;
    fp->pricingUnit().push_back(&pu);
    builder.diag881(_paxType);
    CPPUNIT_ASSERT_EQUAL(std::string("**** PSEUDO FAREPATH DETAIL FOR PASSENGER = ADT ****\n"
                                     " **  PU  ** FARE COMPONENT ** TRAVEL SEG ** DIFF HIGH **\n"
                                     "     OW\n"),
                         _trx->diagnostic().toString());
  }

  void testDiag881_PU_With_FU()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    createDiag(Diagnostic881);
    FarePath* fp = 0;
    _memHandle.get(fp);
    builder._farePath = fp;
    PricingUnit pu;
    fp->pricingUnit().push_back(&pu);

    FareUsage fu;
    PaxTypeFare ptf;
    FareInfo fareInfo;
    fareInfo.fareClass() = "FARE1";
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, fm, &tCRInfo);
    ptf.setFare(&fare);
    fu.paxTypeFare() = &ptf;

    fu.travelSeg().push_back(_tvlSegLaxSfo);
    fu.travelSeg().push_back(_tvlSegSfoLax);

    _tvlSegLaxSfo->segmentOrder() = 1;
    _tvlSegSfoLax->segmentOrder() = 2;
    _itin->travelSeg().push_back(_tvlSegLaxSfo);
    _itin->travelSeg().push_back(_tvlSegSfoLax);

    pu.fareUsage().push_back(&fu);

    pu.puType() = PricingUnit::Type::ROUNDTRIP;
    builder._currentItin = _itin;

    builder.diag881(_paxType);
    CPPUNIT_ASSERT_EQUAL(std::string("**** PSEUDO FAREPATH DETAIL FOR PASSENGER = ADT ****\n"
                                     " **  PU  ** FARE COMPONENT ** TRAVEL SEG ** DIFF HIGH **\n"
                                     "     RT\n"
                                     "            1        FARE1   1 LAX-SFO\n"
                                     "                             2 SFO-LAX\n"),
                         _trx->diagnostic().toString());
  }

  void testDiag881_printAccountCode()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    builder._currentItin = _itin;
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->accountCodeIdPerItin()[_itin].push_back(
        "ACC11");

    DiagCollector* collector = 0;
    collector = _memHandle.create<DiagCollector>();
    collector->activate();
    builder.printAccountCode(*collector);
    CPPUNIT_ASSERT_EQUAL(std::string("ACCOUNT CODE: ACC11\n"), collector->str());
    //    CPPUNIT_ASSERT_EQUAL(std::string("ACCOUNT CODE: ACC11\n"), _trx->diagnostic().toString());
  }

  void testDiag881_printCorpId()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    builder._currentItin = _itin;
    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->corpIdPerItin()[_itin].push_back("CRP01");

    DiagCollector* collector = 0;
    collector = _memHandle.create<DiagCollector>();
    collector->activate();
    builder.printCorpId(*collector);
    CPPUNIT_ASSERT_EQUAL(std::string("CORPORATE ID: CRP01\n"), collector->str());
  }

  void testDiag881_printTktDesignator()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    builder._currentItin = _itin;

    dynamic_cast<TktFeesRequest*>(_trx->getRequest())->tktDesignatorPerItin()[_itin].insert(
        std::pair<int16_t, TktDesignator>(0, "DESISN1"));

    DiagCollector* collector = 0;
    collector = _memHandle.create<DiagCollector>();
    collector->activate();
    builder.printTktDesignator(*collector);
    CPPUNIT_ASSERT_EQUAL(std::string("TKT DESIGNATOR: SEG-0 DESISN1\n"), collector->str());
  }

  void testDiag881_printFopBin()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    builder._currentItin = _itin;

    _fop->fopBinNumber() = "123456";
    _fop->chargeAmount() = 5.12;
    _ppi->fopVector().push_back(_fop);
    _ptp->ppiV().push_back(_ppi);
    _ptp->currency() = "USD";

    TktFeesRequest* req = static_cast<TktFeesRequest*>(_trx->getRequest());
    req->paxTypePaymentPerItin()[_itin] = _ptp;

    DiagCollector* collector = 0;
    collector = _memHandle.create<DiagCollector>();
    collector->activate();
    builder.printFopBin(*collector);

    CPPUNIT_ASSERT_EQUAL(std::string("FOP BIN NUMBER: 123456\n"
                                     "CHARGE AMOUNT : 5.12 USD\n"),
                         collector->str());
  }

  void testInsertFareUsageTvlSegToItsPU()
  {
    PseudoObjectsBuilderStub builder(*_trx);
    FarePath* fp = 0;
    _memHandle.get(fp);
    builder._farePath = fp;
    PricingUnit pu;
    fp->pricingUnit().push_back(&pu);

    FareUsage fu;
    fu.travelSeg().push_back(_tvlSegLaxSfo);
    fu.travelSeg().push_back(_tvlSegSfoLax);
    pu.fareUsage().push_back(&fu);

    builder.insertFareUsageTvlSegToItsPU();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pu.travelSeg().size());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PseudoObjectsBuilderTest);
}
