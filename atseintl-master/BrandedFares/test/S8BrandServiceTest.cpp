#include "test/include/CppUnitHelperMacros.h"

#include "BrandedFares/BrandingRequestResponseHandler.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/S8BrandService.h"
#include "Common/BrandingUtil.h"
#include "Common/Config/ConfigMan.h"
#include "Common/FareDisplayUtil.h"
#include "Common/MetricsMan.h"
#include "Common/MetricsUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareUsage.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/PaxTypeInfo.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestFallbackUtil.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackBrandDirectionality);

namespace
{
const std::string nameSvc = "S8_BRAND_SVC";

class S8BrandServiceMock : public S8BrandService
{
public:
  S8BrandServiceMock(TseServer& server) : S8BrandService(nameSvc, server) {}

  void validateBrandsAndPrograms(PricingTrx& trx, const std::vector<FareMarket*>& markets) const {}
};
}

class S8BrandServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(S8BrandServiceTest);
  CPPUNIT_TEST(testProcess_Metrics);
  CPPUNIT_TEST(testProcess_Pricing);
  CPPUNIT_TEST(testProcess_Shopping);
  CPPUNIT_TEST(testProcess_FareDisplay);
  CPPUNIT_TEST(testCollectPaxTypes_ActualPaxType_Empty);
  CPPUNIT_TEST(testCollectPaxTypes_ActualPaxType_NotEmpty);
  CPPUNIT_TEST(testGetPaxTypes_Non_YY_carrier);
  CPPUNIT_TEST(testGetPaxTypes_YY_carrier);
  CPPUNIT_TEST(testPrintBFErrorCodeFDDiagnosticInvalidTrxType);
  CPPUNIT_TEST(testPrintBFErrorCodeFDDiagnostic);
  CPPUNIT_TEST(testProcessFareDisplayDiag889);
  CPPUNIT_TEST(testGetMarketsToFillWithBrands);
  CPPUNIT_TEST(testBuildMarketRequest);
  CPPUNIT_TEST(testBuildMarketRequestWithDirectionality);
  CPPUNIT_TEST(testBrandingServiceUnavailable);
  CPPUNIT_TEST(testProcess_WP_NOMATCH);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _myServer = _memHandle.create<MockTseServer>();
    DiskCache::initialize(Global::config());
    _memHandle.create<MockDataManager>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _trx->dataHandle().setIsFareDisplay(false);
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    _request = _memHandle.create<FareDisplayRequest>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _request->collectOCFees() = 'T';
    _request->validatingCarrier() = "AA";
    _trx->setRequest(_request);
    _fdTrx = _memHandle.create<FareDisplayTrx>();
    _fdTrx->setRequest(_request);
    Customer* customer = _memHandle.create<Customer>();
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = customer;
    _request->ticketingAgent() = agent;
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);
    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);

    _trx->billing() = _memHandle.create<Billing>();

    _sfs = _memHandle.insert(new S8BrandServiceMock(*_myServer));
    _s8BrandService = _memHandle.insert(new S8BrandService(nameSvc, *_myServer));

    tse::MetricsMan* metricsMan = _memHandle.create<tse::MetricsMan>();
    metricsMan->initialize(tse::MetricsUtil::MAX_METRICS_ENUM);
    MockGlobal::setMetricsMan(metricsMan);
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    TestConfigInitializer::setValue("PRICING_ACTIVATION_DATE", "2013-05-12", "S8_BRAND_SVC", true);
    TestConfigInitializer::setValue("FQ_ACTIVATION_DATE", "2013-05-12", "S8_BRAND_SVC", true);
    fallback::value::fallbackBrandDirectionality.set(true);
    prepareItin();
    createFareMarkets();
    FareClassCode fareClass = "GOGO";
    Fare* f1 = createFare(_trx->fareMarket().front(),
                          Fare::FS_Domestic,
                          GlobalDirection::US,
                          ONE_WAY_MAY_BE_DOUBLED,
                          "",
                          fareClass);
    f1->nucFareAmount() = 2582.45;
    _ptf1 = createPaxTypeFare(
        f1, *_trx->fareMarket().front(), PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    FareClassAppInfo* appInfo1 = _memHandle.create<FareClassAppInfo>();
    appInfo1->_fareType = "BUR";
    _ptf1->fareClassAppInfo() = appInfo1;
    _trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
  }

  void tearDown()
  {
    _memHandle.clear();
    MockGlobal::clear();
  }

  const Loc* getLoc(const LocCode& locCode)
  {
    return TestLocFactory::create("/vobs/atseintl/test/testdata/data/Loc" + locCode + ".xml");
  }

  Fare* createFare(FareMarket* fm1,
                   Fare::FareState state,
                   GlobalDirection gd,
                   Indicator owrt,
                   CurrencyCode currency,
                   FareClassCode& fareClass)
  {
    Fare* f1 = _memHandle.create<Fare>();
    FareInfo* fi1 = _memHandle.create<FareInfo>();
    TariffCrossRefInfo* tcri1 = _memHandle.create<TariffCrossRefInfo>();

    fi1->_globalDirection = gd;
    fi1->_owrt = owrt;
    fi1->_currency = currency;
    fi1->_fareClass = fareClass;
    f1->initialize(state, fi1, *fm1, tcri1);
    return f1;
  }

  PaxTypeFare* createPaxTypeFare(
      Fare* f1, FareMarket& fm1, PaxTypeCode paxTypeCode, VendorCode vendorCode, Indicator adultInd)
  {
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    PaxType* pt1 = _memHandle.create<PaxType>();
    PaxTypeInfo* pti1 = _memHandle.create<PaxTypeInfo>();

    pt1->paxType() = paxTypeCode;
    pt1->vendorCode() = vendorCode;
    pti1->adultInd() = adultInd;
    pt1->paxTypeInfo() = pti1;
    ptf1->initialize(f1, pt1, &fm1);
    return ptf1;
  }

  AirSeg* createAirSeg(const Loc* loc1, const Loc* loc2, BrandCode brandCode = "")
  {
    AirSeg* as = _memHandle.create<AirSeg>();
    as->origin() = const_cast<Loc*>(loc1);
    as->destination() = const_cast<Loc*>(loc2);
    as->boardMultiCity() = loc1->city();
    as->offMultiCity() = loc2->city();
    as->origAirport() = loc1->loc();
    as->destAirport() = loc2->loc();
    as->carrier() = "AA";
    as->setBrandCode(brandCode);
    return as;
  }

  void prepareItin()
  {
    const tse::Loc* sfo(getLoc("SFO"));
    const tse::Loc* dfw(getLoc("DFW"));
    const tse::Loc* lax(getLoc("LAX"));
    const tse::Loc* iah(getLoc("IAH"));
    const tse::Loc* jfk(getLoc("JFK"));

    _itin->travelSeg().push_back(createAirSeg(sfo, dfw));
    _itin->travelSeg().push_back(createAirSeg(dfw, lax, "FL"));
    _itin->travelSeg().push_back(createAirSeg(lax, iah, "FL"));
    _itin->travelSeg().push_back(createAirSeg(iah, jfk, "BC"));
  }

  void createFareMarkets()
  {
    std::vector<TravelSeg*>::iterator first = _itin->travelSeg().begin();

    for (; first != _itin->travelSeg().end(); ++first)
    {
      std::vector<TravelSeg*>::iterator last = first;

      for (; last != _itin->travelSeg().end(); ++last)
      {
        FareMarket* fm = _memHandle.create<FareMarket>();
        fm->travelSeg().insert(fm->travelSeg().begin(), first, last + 1);
        fm->origin() = (*first)->origin();
        fm->destination() = (*last)->destination();
        fm->governingCarrier() = "AA";
        _trx->fareMarket().push_back(fm);
        _fdTrx->fareMarket().push_back(fm);
      }
    }
  }

protected:
  PricingTrx* _trx;
  FareDisplayRequest* _request;
  S8BrandService* _sfs;
  S8BrandService* _s8BrandService;
  TestMemHandle _memHandle;
  TseServer* _myServer;
  Itin* _itin;
  PricingOptions* _options;
  PaxTypeFare* _ptf1;
  FareDisplayTrx* _fdTrx;

  void createDiag(DiagnosticTypes diagType)
  {
    _trx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      _trx->diagnostic().activate();
    }
    _trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
  }

public:
  void testProcess_Metrics()
  {
    MetricsTrx metricsTrx;
    CPPUNIT_ASSERT(_sfs->process(metricsTrx));
  }

  void testProcess_Pricing()
  {
    createDiag(Diagnostic890);
    CPPUNIT_ASSERT(_sfs->process(*_trx));
  }

  void testGetMarketsToFillWithBrands()
  {
    std::vector<FareMarket*> markets;
    _sfs->getMarketsToFillWithBrands(_trx->fareMarket(), markets);

    CPPUNIT_ASSERT(std::find(markets.begin(), markets.end(), _trx->fareMarket()[4]) !=
                   markets.end());
    CPPUNIT_ASSERT(std::find(markets.begin(), markets.end(), _trx->fareMarket()[5]) !=
                   markets.end());
    CPPUNIT_ASSERT(std::find(markets.begin(), markets.end(), _trx->fareMarket()[7]) !=
                   markets.end());
    CPPUNIT_ASSERT(std::find(markets.begin(), markets.end(), _trx->fareMarket()[9]) !=
                   markets.end());
    CPPUNIT_ASSERT_EQUAL((int)markets.size(), 4);
  }

  void testBuildMarketRequest()
  {
    std::vector<FareMarket*> markets;

    markets.push_back(_trx->fareMarket()[4]);
    markets.push_back(_trx->fareMarket()[5]);
    markets.push_back(_trx->fareMarket()[7]);
    markets.push_back(_trx->fareMarket()[9]);

    BrandingRequestResponseHandler bRRH(*_trx);
    IAIbfUtils::FMsForBranding fMsForBranding;
    bRRH.setClientId(BR_CLIENT_PBB);
    _sfs->buildBrandingRequest(markets, *_trx, bRRH, fMsForBranding);

    struct
    {
      bool operator()(const FareMarket* a, const FareMarket* b)
      {
        return a->marketIDVec().front() < b->marketIDVec().front();
      }
    } marketIdOrder;
    std::sort(markets.begin(), markets.end(), marketIdOrder);
    std::vector<FareMarket*>::iterator iBeg = markets.begin();
    std::vector<FareMarket*>::iterator iEnd = markets.end();
    int index = 1;
    for (; iBeg != iEnd; ++iBeg)
    {
      CPPUNIT_ASSERT_EQUAL(index++, (*iBeg)->marketIDVec().front());
    }
  }

  void testBuildMarketRequestWithDirectionality()
  {
    fallback::value::fallbackBrandDirectionality.set(false);

    std::vector<FareMarket*> markets;

    markets.push_back(_trx->fareMarket()[4]);
    markets.push_back(_trx->fareMarket()[5]);
    markets.push_back(_trx->fareMarket()[7]);
    markets.push_back(_trx->fareMarket()[9]);

    BrandingRequestResponseHandler bRRH(*_trx);
    IAIbfUtils::FMsForBranding fMsForBranding;
    bRRH.setClientId(BR_CLIENT_PBB);
    _sfs->buildBrandingRequest(markets, *_trx, bRRH, fMsForBranding);

    std::vector<FareMarket*>::iterator iBeg = markets.begin();
    std::vector<FareMarket*>::iterator iEnd = markets.end();
    for (; iBeg != iEnd; ++iBeg)
    {
      // for each fm expect two o&d (both directions)
      CPPUNIT_ASSERT_EQUAL((size_t)2, (*iBeg)->marketIDVec().size());
    }
  }

  void testProcess_Shopping()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(_sfs->process(*_trx));
  }

  void testProcess_FareDisplay()
  {
    _trx->dataHandle().setIsFareDisplay(true);
    _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    createDiag(Diagnostic888);
    _trx->diagnostic().diagParamMap().insert(
        std::make_pair(Diagnostic::DISPLAY_DETAIL, "S8BRANDREQ"));
    CPPUNIT_ASSERT(_sfs->process(*static_cast<FareDisplayTrx*>(_trx)));
    CPPUNIT_ASSERT_EQUAL(PricingTrx::BG_ERROR, _trx->bfErrorCode());

    CPPUNIT_ASSERT_EQUAL(
        std::string("*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : SFO - DFW   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : SFO - LAX   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : SFO - IAH   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : SFO - JFK   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : DFW - LAX   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : DFW - IAH   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : DFW - JFK   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : LAX - IAH   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : LAX - JFK   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"
                    "*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                    "------------ FARE MARKET : IAH - JFK   CXR - AA -------------\n"
                    "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                    "A  AA      DATA NOT FOUND\n"),
        _trx->diagnostic().toString());
  }

  void testCollectPaxTypes_ActualPaxType_Empty()
  {
    std::vector<PaxTypeCode> paxTypes;
    CarrierCode carrier = "AA";
    PaxType paxType;
    paxType.paxType() = "ADT";

    _sfs->collectPaxTypes(carrier, paxType, paxTypes);
    CPPUNIT_ASSERT(paxTypes.empty());
  }

  void testCollectPaxTypes_ActualPaxType_NotEmpty()
  {
    std::vector<PaxTypeCode> paxTypes;
    std::vector<PaxType*> pType;
    CarrierCode carrier = "AA";
    PaxType paxType, paxTypeActual;

    preparePaxTypes(paxTypes, pType, paxType, paxTypeActual);

    _sfs->collectPaxTypes(carrier, paxType, paxTypes);
    CPPUNIT_ASSERT(!paxTypes.empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)paxTypes.size());
    CPPUNIT_ASSERT(paxTypes[0] == "APP");
  }

  void testGetPaxTypes_Non_YY_carrier()
  {
    std::vector<PaxTypeCode> paxTypes;

    std::vector<PaxType*> pType;
    PaxType paxType, paxTypeActual;

    preparePaxTypes(paxTypes, pType, paxType, paxTypeActual);

    _sfs->getPaxTypes(*_trx, paxTypes);

    CPPUNIT_ASSERT(!paxTypes.empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)paxTypes.size());
    CPPUNIT_ASSERT(paxTypes[0] == "APP");
  }

  void testGetPaxTypes_YY_carrier()
  {
    std::vector<PaxTypeCode> paxTypes;

    std::vector<PaxType*> pType;
    PaxType paxType, paxTypeActual;

    preparePaxTypes(paxTypes, pType, paxType, paxTypeActual);

    _options->iataFares() = 'I';
    _sfs->getPaxTypes(*_trx, paxTypes);

    CPPUNIT_ASSERT(!paxTypes.empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)paxTypes.size());
    CPPUNIT_ASSERT(paxTypes[0] == "APP");
  }

  void preparePaxTypes(std::vector<PaxTypeCode>& paxTypes,
                       std::vector<PaxType*>& pType,
                       PaxType& paxType,
                       PaxType& paxTypeActual)
  {
    CarrierCode carrier = "AA";
    paxType.paxType() = "ADT";

    paxTypeActual.paxType() = "APP";
    pType.push_back(&paxTypeActual);

    paxType.actualPaxType().insert(make_pair(carrier, &pType));

    _trx->paxType().push_back(&paxType);
  }

  void createFdDiag(DiagnosticTypes diagType = Diagnostic195)
  {
    _fdTrx->diagnostic().diagnosticType() = diagType;
    _fdTrx->getRequest()->diagnosticNumber() = diagType;
    if (diagType != DiagnosticNone)
    {
      _fdTrx->diagnostic().activate();
    }
  }

  void testPrintBFErrorCodeFDDiagnosticInvalidTrxType()
  {
    createFdDiag(Diagnostic195);
    _fdTrx->diagnostic().diagParamMap().insert(
        std::make_pair(Diagnostic::DISPLAY_DETAIL, "S8BRANDREQ"));
    _fdTrx->setTrxType(PricingTrx::PRICING_TRX);
    _sfs->printBFErrorCodeFDDiagnostic(*_fdTrx);
    CPPUNIT_ASSERT(_fdTrx->response().str().empty());
  }

  void testPrintBFErrorCodeFDDiagnostic()
  {
    createFdDiag(Diagnostic195);
    _fdTrx->diagnostic().diagParamMap().insert(
        std::make_pair(Diagnostic::DISPLAY_DETAIL, "S8BRANDRES"));
    _fdTrx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    _sfs->printBFErrorCodeFDDiagnostic(*_fdTrx);
    CPPUNIT_ASSERT_EQUAL(std::string("REQUESTED CARRIER AA INACTIVE FOR BRAND SERVICE\n \n"),
                         _fdTrx->response().str());
  }

  void testProcessFareDisplayDiag889()
  {
    createFdDiag(Diagnostic889);
    _fdTrx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    _fdTrx->bfErrorCode() = PricingTrx::CARRIER_NOT_ACTIVE;
    CPPUNIT_ASSERT(_sfs->process(*_fdTrx));
    CPPUNIT_ASSERT_EQUAL(std::string("REQUESTED CARRIER AA INACTIVE FOR BRAND SERVICE\n \n"),
                         _fdTrx->response().str());
  }

  void testBrandingServiceUnavailable()
  {
    TestConfigInitializer::setValue("URL", "InvalidUrl", "S8_BRAND_SVC");
    std::string message;
    try
    {
      CPPUNIT_ASSERT(_sfs->process(*_trx));
    }
    catch (ErrorResponseException& ex)
    {
      message = ex.message();
    }

    std::string expected = "BRANDING SERVICE IS UNAVAILABLE";
    CPPUNIT_ASSERT_EQUAL(message, expected);
  }

  void testProcess_WP_NOMATCH()
  {
    TestConfigInitializer::setValue("URL", "InvalidUrl", "S8_BRAND_SVC");
    _trx = _memHandle.create<AltPricingTrx>();
    _trx->altTrxType() = AltPricingTrx::WP_NOMATCH;
    std::string message;
    try
    {
      CPPUNIT_ASSERT(_sfs->process(*_trx));
    }
    catch (ErrorResponseException& ex)
    {
      message = ex.message();
    }

    CPPUNIT_ASSERT(message.empty());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(S8BrandServiceTest);
}
