#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/FareDisplayService.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareDisplayRequest.h"
#include "Common/TseStringTypes.h"
#include "Common/DateTime.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"
#include "test/include/MockGlobal.h"
#include "DBAccess/Loc.h"
#include "DataModel/Fare.h"
#include "test/include/MockTseServer.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/FareDisplayOptions.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "Diagnostic/DiagCollector.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FareDisplayServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayServiceTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcess_PricingDiagnostic);
  CPPUNIT_TEST(testGetRequestTypeDiagnostic);
  CPPUNIT_TEST(testGetRequestTypeFQRequest);
  CPPUNIT_TEST(testGetRequestTypeRBRequest);
  CPPUNIT_TEST(testGetRequestTypeADRequest);
  CPPUNIT_TEST(testGetRequestTypeFTRequest);
  CPPUNIT_TEST(testGetRequestTypeMPRequest);
  CPPUNIT_TEST_SUITE_END();

  class FareDisplayServiceMock : public FareDisplayService
  {
  public : FareDisplayServiceMock(tse::TseServer& tseServer)
  : FareDisplayService("FARE_DISPLAY_SVC", tseServer)
  , _merged(false)
  {
  }

  void mergeFares(FareDisplayTrx& trx, const bool needAllFares = false)
  {
    _merged = true;
    FareDisplayService::mergeFares(trx, needAllFares);
  }

  bool _merged;

  };

public:
  void setUp()
  {
    _srv = _memHandle.create<MockTseServer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _request = _memHandle.create<FareDisplayRequest>();
    _options = _memHandle.create<FareDisplayOptions>();
    _trx->setRequest(_request);
    _trx->setOptions(_options);

    _fqService = _memHandle.insert(new FareDisplayServiceMock(*_srv));
  }
  void tearDown()
  {
    _memHandle.clear();
  }

  void createItin()
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fi = _memHandle.create<FareInfo>();
    ptf->setFare(fare);
    fare->setFareInfo(fi);

    Itin* itin = _memHandle.create<Itin>();
    _trx->itin().push_back(itin);

    FareMarket* fm  = _memHandle.create<FareMarket>();
    itin->fareMarket().push_back(fm);
    _trx->fareMarket().push_back(fm);

    fm->allPaxTypeFare().push_back(ptf);

    AirSeg* as = _memHandle.create<AirSeg>();
    fm->travelSeg().push_back(as);
    as->origin() = _trx->dataHandle().getLoc("DFW", DateTime::localTime());
    as->destination() = _trx->dataHandle().getLoc("LON", DateTime::localTime());
  }

  void testProcess()
  {
    createItin();

    CPPUNIT_ASSERT(_fqService->process(*_trx));
    CPPUNIT_ASSERT(_fqService->_merged);
  }

  // Test should return true and ignore pricing diagnostic request
  void testProcess_PricingDiagnostic()
  {
    createItin();
    _trx->diagnostic().activate();

    CPPUNIT_ASSERT(_fqService->process(*_trx));
    CPPUNIT_ASSERT(!_fqService->_merged);
  }

  void testGetRequestTypeDiagnostic()
  {
    _request->diagnosticNumber() = DIAG_200_ID;
    CPPUNIT_ASSERT_EQUAL(FareDisplayService::DIAGNOSTIC_REQUEST, _fqService->getRequestType(*_trx));
  }

  void testGetRequestTypeFQRequest()
  {
    CPPUNIT_ASSERT_EQUAL(FareDisplayService::FQ_REQUEST, _fqService->getRequestType(*_trx));
  }

  void testGetRequestTypeRBRequest()
  {
    _request->requestType() = "RB";
    CPPUNIT_ASSERT_EQUAL(FareDisplayService::RB_REQUEST, _fqService->getRequestType(*_trx));
  }

  void testGetRequestTypeADRequest()
  {
    _request->inclusionCode() = ADDON_FARES;
    CPPUNIT_ASSERT_EQUAL(FareDisplayService::AD_REQUEST, _fqService->getRequestType(*_trx));
  }

  void testGetRequestTypeFTRequest()
  {
    _request->requestType() = FARE_TAX_REQUEST;
    CPPUNIT_ASSERT_EQUAL(FareDisplayService::FT_REQUEST, _fqService->getRequestType(*_trx));
  }

  void testGetRequestTypeMPRequest()
  {
    _request->requestType() = FARE_MILEAGE_REQUEST;
    CPPUNIT_ASSERT_EQUAL(FareDisplayService::MP_REQUEST, _fqService->getRequestType(*_trx));
  }

private:
  TestMemHandle _memHandle;
  MockTseServer* _srv;
  FareDisplayServiceMock* _fqService;
  FareDisplayTrx* _trx;
  FareDisplayRequest* _request;
  FareDisplayOptions* _options;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayServiceTest);
}
