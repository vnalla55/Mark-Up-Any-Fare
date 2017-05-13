#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DSS/FlightCountMgr.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <vector>

namespace tse
{

class FlightCountMgrTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FlightCountMgrTest);
  CPPUNIT_TEST(testGetCarrierList_Shopper);
  CPPUNIT_TEST(testGetCarrierList_NotShopper);

  CPPUNIT_TEST(testGetFlightCount_NoSend);
  CPPUNIT_TEST(testGetFlightCount_NoReceive);
  CPPUNIT_TEST(testGetFlightCount_NoSchedRequest);
  CPPUNIT_TEST(testGetFlightCount_Pass);

  CPPUNIT_TEST_SUITE_END();

public:
  void testGetCarrierList_Shopper()
  {
    std::set<CarrierCode> cxrs;
    _mgr->getCarrierList(*_trx, cxrs);
    CPPUNIT_ASSERT_EQUAL(size_t(2), cxrs.size());
  }

  void testGetCarrierList_NotShopper()
  {
    std::set<CarrierCode> cxrs;
    _opt->allCarriers() = 'N';
    _mgr->getCarrierList(*_trx, cxrs);
    CPPUNIT_ASSERT_EQUAL(size_t(3), cxrs.size());
  }

  void testGetFlightCount_NoSend()
  {
    _mgr->_retWrite = false;
    CPPUNIT_ASSERT(!_mgr->getFlightCount(*_trx, _fc));
  }

  void testGetFlightCount_NoReceive()
  {
    _mgr->_retRead = false;
    CPPUNIT_ASSERT(!_mgr->getFlightCount(*_trx, _fc));
  }

  void testGetFlightCount_NoSchedRequest()
  {
    _trx->setOptions(0);
    CPPUNIT_ASSERT(!_mgr->getFlightCount(*_trx, _fc));
  }

  void testGetFlightCount_Pass()
  {
    CPPUNIT_ASSERT(_mgr->getFlightCount(*_trx, _fc));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _fc.size());
  }

  Loc* createLoc(const char* loc)
  {
    Loc* l = _memHandle.create<Loc>();
    l->loc() = loc;
    return l;
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _opt = _memHandle.create<FareDisplayOptions>();
    _trx->setOptions(_opt);
    _opt->allCarriers() = 'Y';
    _trx->preferredCarriers().insert("C1");
    _trx->preferredCarriers().insert("C2");
    FareMarket* fm = _memHandle.create<FareMarket>();
    AirSeg* as = _memHandle.create<AirSeg>();
    as->origin() = fm->origin() = createLoc("KRK");
    as->destination() = fm->destination() = createLoc("JFK");
    fm->governingCarrier() = "LO";
    fm->geoTravelType() = GeoTravelType::International;
    _trx->fareMarket().push_back(fm);
    fm->boardMultiCity() = as->boardMultiCity() = "KRK";
    fm->offMultiCity() = as->offMultiCity() = "NYC";
    _trx->travelSeg().push_back(as);
    _trx->itin().push_back(_memHandle.create<Itin>());
    _req = _memHandle.create<FareDisplayRequest>();
    _trx->setRequest(_req);
    _req->ticketingAgent() = _memHandle.create<Agent>();
    _trx->billing() = _memHandle.create<Billing>();
    _mgr = _memHandle.create<FlightCountMgrMock>();
  }

  void tearDown() { _memHandle.clear(); }

private:
  class MyDataHandle : public DataHandleMock
  {
  public:
    const std::vector<CarrierCode>& getCarriersForMarket(const LocCode& market1,
                                                         const LocCode& market2,
                                                         bool includeAddon,
                                                         const DateTime& date)
    {
      std::vector<CarrierCode>* ret = _memHandle.create<std::vector<tse::CarrierCode> >();
      if (market1 == "KRK" && market2 == "JFK")
      {
        ret->push_back("AA");
        ret->push_back("LO");
        return *ret;
      }
      else if (market1 == "KRK" && market2 == "NYC")
      {
        ret->push_back("AA");
        ret->push_back("LO");
        ret->push_back("BA");
        return *ret;
      }
      return DataHandleMock::getCarriersForMarket(market1, market2, includeAddon, date);
    }

  private:
    TestMemHandle _memHandle;
  };

  class FlightCountMgrMock : public FlightCountMgr
  {
  public:
    std::string _payload;
    bool _retWrite, _retRead;

    FlightCountMgrMock()
    {
      _retWrite = _retRead = true;
      _payload = "<DSS><FSD COD=\"LO\" NST=\"0\" DIR=\"1\" ONL=\"2\"/><FSD COD=\"AA\" NST=\"4\" "
                 "DIR=\"5\" ONL=\"6\"/></DSS>";
    }

    bool
    writeSocket(eo::Socket& socket, ac::SocketUtils::Message& req, bool standardHeaderNotSupported)
        const
    {
      return _retWrite;
    }

    bool
    readSocket(eo::Socket& socket, ac::SocketUtils::Message& res, bool standardHeaderNotSupported)
        const
    {
      res.payload = _payload;
      return _retRead;
    }

    bool initializeDSS(eo::Socket& socket) const { return true; }
  };

  TestMemHandle _memHandle;
  FlightCountMgrMock* _mgr;
  FareDisplayTrx* _trx;
  FareDisplayOptions* _opt;
  FareDisplayRequest* _req;
  std::vector<FlightCount*> _fc;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FlightCountMgrTest);
}
