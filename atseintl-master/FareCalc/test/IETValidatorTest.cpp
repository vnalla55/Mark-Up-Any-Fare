#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"
#include "FareCalc/IETValidator.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diagnostic.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestLocFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{

using boost::assign::operator+=;

class IETValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IETValidatorTest);
  CPPUNIT_TEST(testValidatingCarrier);
  CPPUNIT_TEST(testValidatingCarrierNoStatus);
  CPPUNIT_TEST(testValidatingCarrierStatusDeactivated);
  CPPUNIT_TEST(testValidatingCarrierStatusBeta);
  CPPUNIT_TEST(testValidatingCarrierStatusNotInitialized);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    const std::string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
    const std::string LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);
    _request = _memHandle.create<PricingRequest>();
    _agent = _memHandle.create<Agent>();
    _agent->cxrCode() = "1S";
    _request->ticketingAgent() = _agent;
    _trx->setRequest(_request);
    _airSeg1 = _memHandle.create<AirSeg>();
    _airSeg2 = _memHandle.create<AirSeg>();

    _airSeg1->origin() = TestLocFactory::create(LOC_DFW);
    _airSeg1->destination() = TestLocFactory::create(LOC_LON);
    _airSeg2->origin() = TestLocFactory::create(LOC_LON);
    _airSeg2->destination() = TestLocFactory::create(LOC_DFW);
    _airSeg1->segmentOrder() = 0;
    _airSeg2->segmentOrder() = 1;
    _airSeg1->carrier() = "AA";
    _airSeg1->setOperatingCarrierCode("AA");
    _airSeg2->carrier() = "BA";
    _airSeg1->setOperatingCarrierCode("DL");
    _itin->travelSeg() += _airSeg1, _airSeg2;
    _itin->validatingCarrier() = "AA";
    _iET = _memHandle.insert(new IETValidator(*_trx));
  }

  void tearDown() { _memHandle.clear(); }

protected:
  void testValidatingCarrier()
  {
    _itin->validatingCarrier() = "AA";
    std::string message;
    _iET->validate(*_trx, *_itin, message);
    CPPUNIT_ASSERT_EQUAL(std::string("AA HAS NO INTERLINE TICKETING AGREEMENT WITH DL"), message);
  }
  void testValidatingCarrierNoStatus()
  {
    _itin->validatingCarrier() = "EH";
    std::string message;
    _iET->validate(*_trx, *_itin, message);
    CPPUNIT_ASSERT_EQUAL(std::string("EH HAS NO TICKETING AGREEMENT-CHANGE VALIDATING CARRIER"),
                         message);
  }
  void testValidatingCarrierStatusDeactivated()
  {
    _itin->validatingCarrier() = "BA";
    std::string message;
    _iET->validate(*_trx, *_itin, message);
    CPPUNIT_ASSERT_EQUAL(std::string("BA HAS NO TICKETING AGREEMENT-CHANGE VALIDATING CARRIER"),
                         message);
  }
  void testValidatingCarrierStatusBeta()
  {
    _itin->validatingCarrier() = "VR";
    std::string message;
    _iET->validate(*_trx, *_itin, message);
    CPPUNIT_ASSERT_EQUAL(std::string("VR HAS NO TICKETING AGREEMENT-CHANGE VALIDATING CARRIER"),
                         message);
  }
  void testValidatingCarrierStatusNotInitialized()
  {
    _itin->validatingCarrier() = "JJ";
    std::string message;
    _iET->validate(*_trx, *_itin, message);
    CPPUNIT_ASSERT_EQUAL(std::string("JJ HAS NO TICKETING AGREEMENT-CHANGE VALIDATING CARRIER"),
                         message);
  }

  PricingTrx* _trx;
  Itin* _itin;
  PricingRequest* _request;
  AirSeg* _airSeg1;
  AirSeg* _airSeg2;
  Agent* _agent;
  mutable TestMemHandle _memHandle;
  IETValidator* _iET;
};
CPPUNIT_TEST_SUITE_REGISTRATION(IETValidatorTest);

} // namespace
