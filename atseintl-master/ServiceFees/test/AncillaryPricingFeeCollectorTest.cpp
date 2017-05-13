#include <boost/assign/std/vector.hpp>

#include "Common/Config/ConfigMan.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag875Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "ServiceFees/AncillaryPricingFeeCollector.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace boost::assign;
using namespace std;
namespace tse
{

namespace
{
ServiceGroup serviceGroups[] = { "BG", "ML" };
}

class AncillaryPricingFeeCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryPricingFeeCollectorTest);
  CPPUNIT_TEST(testShouldProcessAllGroups);
  CPPUNIT_TEST(testGetUserApplCode_1S);
  CPPUNIT_TEST(testGetUserApplCode_1F);
  CPPUNIT_TEST(testGetUserApplCode_1J);
  CPPUNIT_TEST(testGetUserApplCode_1B);
  CPPUNIT_TEST(testGetUserApplCode_Blank);

  CPPUNIT_TEST(testUpdateServiceFeesGroupState_NoFees_Empty);
  CPPUNIT_TEST(testUpdateServiceFeesGroupState_Not_Empty);

  CPPUNIT_TEST(testIsAgencyActive_True);
  CPPUNIT_SKIP_TEST(testProcessCarrierMMRecords_Retrieved);
  CPPUNIT_SKIP_TEST(testProcessCarrierMMRecords_No_carriers);
  CPPUNIT_SKIP_TEST(testValidMerchCxrGroup);
  CPPUNIT_TEST(testCheckAllSegsConfirmedWhenAllConfirmed);
  CPPUNIT_TEST(testAddInvalidGroupsPricing);
  CPPUNIT_TEST(testGetNumberOfSegments);
  CPPUNIT_TEST(testProcess_with_Diag881_Active);

  CPPUNIT_TEST_SUITE_END();

public:
  void testShouldProcessAllGroups() { CPPUNIT_ASSERT(!_collector->shouldProcessAllGroups()); }
  void testGetUserApplCode_1S()
  {
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1S";
    CPPUNIT_ASSERT_EQUAL(UserApplCode("SABR"), _collector->getUserApplCode());
  }
  void testGetUserApplCode_1F()
  {
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1F";
    CPPUNIT_ASSERT_EQUAL(UserApplCode("INFI"), _collector->getUserApplCode());
  }
  void testGetUserApplCode_1J()
  {
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1J";
    CPPUNIT_ASSERT_EQUAL(UserApplCode("AXES"), _collector->getUserApplCode());
  }
  void testGetUserApplCode_1B()
  {
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1B";
    CPPUNIT_ASSERT_EQUAL(UserApplCode("ABAC"), _collector->getUserApplCode());
  }
  void testGetUserApplCode_Blank()
  {
    // willl default to SABR
    CPPUNIT_ASSERT_EQUAL(UserApplCode("SABR"), _collector->getUserApplCode());
  }

  void testUpdateServiceFeesGroupState_NoFees_Empty()
  {
    ServiceFeesGroup sfg;

    _collector->updateServiceFeesGroupState(&sfg);

    CPPUNIT_ASSERT_EQUAL(ServiceFeesGroup::EMPTY, sfg.state());
  }

  void testUpdateServiceFeesGroupState_Not_Empty()
  {
    createServiceFeesGroup(ServiceGroup("ML"), false);

    _collector->updateServiceFeesGroupState(_sfg);

    CPPUNIT_ASSERT_EQUAL(ServiceFeesGroup::VALID, _sfg->state());
  }

  void createServiceFeesGroup(const ServiceGroup& serviceGroup, bool empty = true)
  {
    vector<OCFees*>* ocF = _memHandle.create<vector<OCFees*> >();
    if (!empty)
    {
      ocF->push_back(_memHandle.create<OCFees>());
      (*ocF)[0]->carrierCode() = "AA";
      _sfg->ocFeesMap().insert(make_pair(_farePath, *ocF));
    }

    _sfg->groupCode() = serviceGroup;
    _sfg->groupDescription() = "serviceGroup";

    _itin->ocFeesGroup().push_back(_sfg);
  }

  void testIsAgencyActive_True() { CPPUNIT_ASSERT(_collector->isAgencyActive()); }

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

  void insertCarrier(const CarrierCode& carrier, bool operating)
  {
    (operating ? _collector->_operatingCxr : _collector->_marketingCxr)[0].insert(carrier);
    _collector->_cXrGrpOCF.insert(make_pair(carrier, *_gValid));
  }

  void testProcessCarrierMMRecords_Retrieved()
  {
    createDiag();
    insertCarrier("AA", true);

    _collector->processCarrierMMRecords();
    CPPUNIT_ASSERT_EQUAL(string::npos,
                         _collector->diag875()->str().find("NO CARRIERS ACTIVE FOR OC PROCESSING"));
  }

  void testProcessCarrierMMRecords_No_carriers()
  {
    createDiag();

    _collector->processCarrierMMRecords();
    CPPUNIT_ASSERT(string::npos !=
                   _collector->diag875()->str().find("NO CARRIERS ACTIVE FOR OC PROCESSING"));
  }

  void testValidMerchCxrGroup()
  {
    CarrierCode cxr = "AA";
    ServiceGroup sG = "BG";
    CPPUNIT_ASSERT(_collector->validMerchCxrGroup(cxr, sG));
  }

  void testCheckAllSegsConfirmedWhenAllConfirmed()
  {
    _collector->_farePath = _farePath;
    CPPUNIT_ASSERT(_collector->checkAllSegsConfirmed(
        _trx->travelSeg().begin(), _trx->travelSeg().end(), false));
    CPPUNIT_ASSERT_EQUAL(true, _itin->allSegsConfirmed());
  }

  void testAddInvalidGroupsPricing()
  {
    std::vector<ServiceGroup> groupNA; // Group codes not active
    std::vector<ServiceGroup> groupNP; // Group codes not active
    std::vector<ServiceGroup> groupNV; // Group codes invalid
    ServiceGroup group2 = "UP";
    ServiceGroup group3 = "FF";

    groupNA.push_back(group2);
    groupNV.push_back(group3);

    _collector->addInvalidGroups(*_itin, groupNA, groupNP, groupNV);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _itin->ocFeesGroup().size());
    CPPUNIT_ASSERT_EQUAL(group2, _itin->ocFeesGroup()[0]->groupCode());
    CPPUNIT_ASSERT_EQUAL(group3, _itin->ocFeesGroup()[1]->groupCode());
  }

  void testGetNumberOfSegments()
  {
    _trx->travelSeg().push_back(_seg1);
    _trx->travelSeg().push_back(_seg2Arunk);
    _trx->travelSeg().push_back(_seg3);
    int number =
        _collector->getNumberOfSegments(_trx->travelSeg().begin(), _trx->travelSeg().end());
    CPPUNIT_ASSERT_EQUAL(2, number);
  }

  void testProcess_with_Diag881_Active()
  {
    createDiag(Diagnostic881);

    _collector->process();
    CPPUNIT_ASSERT(_trx->diagnostic().toString().empty());
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _ancTrx = _memHandle.create<AncillaryPricingTrx>();
    _collector = _memHandle.insert(new AncillaryPricingFeeCollector(*_trx));
    _trx->setRequest(_memHandle.create<AncRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();

    _farePath = _memHandle.create<FarePath>();
    _itin = _memHandle.create<Itin>();
    _farePath->itin() = _itin;
    _trx->itin().push_back(_itin);
    _itin->farePath().push_back(_farePath);

    _travelSegs = _memHandle.create<vector<TravelSeg*> >();
    _seg1 = _memHandle.create<AirSeg>();
    _seg2Arunk = _memHandle.create<ArunkSeg>();
    _seg3 = _memHandle.create<AirSeg>();
    _travelSegs->push_back(_seg1);
    _travelSegs->push_back(_seg2Arunk);
    _travelSegs->push_back(_seg3);

    _sfg = _memHandle.create<ServiceFeesGroup>();
    _gValid = _memHandle.create<vector<ServiceGroup*> >();
    _gValid->push_back(serviceGroups);
    _gValid->push_back(serviceGroups + 1);
  }
  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  AncillaryPricingTrx* _ancTrx;
  PricingTrx* _trx;
  AncillaryPricingFeeCollector* _collector;
  Itin* _itin;
  FarePath* _farePath;
  ServiceFeesGroup* _sfg;
  vector<ServiceGroup*>* _gValid;

  vector<TravelSeg*>* _travelSegs;
  AirSeg* _seg1;
  ArunkSeg* _seg2Arunk;
  AirSeg* _seg3;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryPricingFeeCollectorTest);
}
