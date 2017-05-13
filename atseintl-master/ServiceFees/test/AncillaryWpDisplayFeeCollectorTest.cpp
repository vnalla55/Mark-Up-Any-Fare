#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "ServiceFees/AncillaryWpDisplayFeeCollector.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;
namespace tse
{

class AncillaryWpDisplayFeeCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryWpDisplayFeeCollectorTest);

  CPPUNIT_TEST(testProcess_with_Diag881_Active);
  CPPUNIT_TEST(testProcess_with_Diag878_Active);
  CPPUNIT_TEST(testProcess_with_Diag879_Active);
  CPPUNIT_TEST(testSamePaxType_False);
  CPPUNIT_TEST(testSamePaxType_True);

  CPPUNIT_TEST_SUITE_END();

public:
  void createDiag(DiagnosticTypes diagType = Diagnostic875, bool isDDINFO = false)
  {
    _trx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      if (isDDINFO)
        _trx->diagnostic().diagParamMap().insert(make_pair(Diagnostic::DISPLAY_DETAIL, "INFO"));
      _trx->diagnostic().activate();
    }
    _collector->createDiag();
  }

  void createAncDiag(DiagnosticTypes diagType)
  {
    _ancTrx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      _ancTrx->diagnostic().activate();
    }
  }

  void testProcess_with_Diag881_Active()
  {
    createDiag(Diagnostic881);

    _collector->process();
    CPPUNIT_ASSERT(_trx->diagnostic().toString().empty());
  }

  void testProcess_with_Diag878_Active()
  {
    _ancRequest->ancRequestType() = AncRequest::WPAERequest;
    _ancRequest->collectOCFees() = 'T';
    createAncDiag(Diagnostic878);

    _collector->checkDiag878And879();
    CPPUNIT_ASSERT(_ancTrx->diagnostic().toString().empty());
  }

  void testProcess_with_Diag879_Active()
  {
    _ancRequest->ancRequestType() = AncRequest::WPAERequest;
    _ancRequest->collectOCFees() = 'T';
    createAncDiag(Diagnostic879);

    _collector->checkDiag878And879();
    CPPUNIT_ASSERT(_ancTrx->diagnostic().toString().empty());
  }

  void testSamePaxType_False()
  {
    Itin itin;
    _ancTrx->itin().push_back(&itin);
    PaxType pax1, pax2;
    pax1.paxType() = "ADT";
    pax1.requestedPaxType() = ADULT;
    pax1.number() = 1;
    pax2.paxType() = "CNN";
    pax2.requestedPaxType() = "CNN";
    pax2.number() = 1;
    _ancRequest->paxTypesPerItin()[&itin].push_back(&pax1);
    _ancRequest->paxTypesPerItin()[&itin].push_back(&pax2);

    CPPUNIT_ASSERT(!_collector->samePaxType(*_ancTrx));
  }

  void testSamePaxType_True()
  {
    Itin itin;
    _ancTrx->itin().push_back(&itin);
    PaxType pax1, pax2;
    pax1.paxType() = "ADT";
    pax1.requestedPaxType() = ADULT;
    pax1.number() = 1;
    pax2.paxType() = "ADT";
    pax2.requestedPaxType() = "ADT";
    pax2.number() = 1;
    _ancRequest->paxTypesPerItin()[&itin].push_back(&pax1);
    _ancRequest->paxTypesPerItin()[&itin].push_back(&pax2);

    CPPUNIT_ASSERT(_collector->samePaxType(*_ancTrx));
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<AncillaryPricingTrx>();
    _ancTrx = _memHandle.create<AncillaryPricingTrx>();
    _ancRequest = _memHandle.create<AncRequest>();
    _collector = _memHandle.insert(new AncillaryWpDisplayFeeCollector(*_trx));
    _ancRequest->collectOCFees() = 'T';
    _ancRequest->validatingCarrier() = "AA";
    const DateTime tktDate = DateTime(2011, 3, 23, 8, 15, 0);
    _ancRequest->ticketingDT() = tktDate;
    _ancTrx->setRequest(_ancRequest);
  }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  AncillaryPricingTrx* _ancTrx;
  AncRequest* _ancRequest;
  PricingTrx* _trx;
  AncillaryWpDisplayFeeCollector* _collector;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryWpDisplayFeeCollectorTest);
}
