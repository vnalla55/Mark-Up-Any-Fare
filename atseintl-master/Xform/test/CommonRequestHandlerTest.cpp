#include "Common/DateTime.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Xform/CommonRequestHandler.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
static const int32_t UTC_OFFSET_HOURS_BETWEEN_DFW_MEL = 17;
static const int32_t UTC_OFFSET_HOURS_BETWEEN_MEL_DFW = -UTC_OFFSET_HOURS_BETWEEN_DFW_MEL;

namespace
{

class MyDataHandle : public DataHandleMock
{
public:
  MyDataHandle(TestMemHandle& memHandle, PricingTrx* trx, Itin* itin)
    : _memHandle(memHandle), _pricingTrx(trx), _itin(itin)
  {
  }

  bool getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                              const DSTGrpCode& dstgrp2,
                              short& utcOffset,
                              const DateTime& dateTime1,
                              const DateTime& dateTime2)
  {
    if (_itin->travelSeg().empty())
    {
      return false;
    }

    LocCode agentLoc = _pricingTrx->getRequest()->ticketingAgent()->agentLocation()->loc();
    LocCode originLoc = _itin->travelSeg()[0]->origAirport();

    if ((agentLoc == "DFW") && (originLoc == "DFW"))
    {
      utcOffset = 0;
    }
    else if ((agentLoc == "DFW") && (originLoc == "MEL"))
    {
      utcOffset = UTC_OFFSET_HOURS_BETWEEN_MEL_DFW * static_cast<int32_t>(MINUTES_PER_HOUR);
    }
    else
    {
      return false;
    }

    return true;
  }

  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "MEL")
    {
      return _memHandle.create<Loc>();
    }

    return DataHandleMock::getLoc(locCode, date);
  }

private:
  TestMemHandle& _memHandle;
  PricingTrx* _pricingTrx;
  Itin* _itin;
};
}

// MOCKS
class TestCommonRequestHandler : public CommonRequestHandler
{
public:
  TestCommonRequestHandler(Trx*& trx) : CommonRequestHandler(trx) {}

protected:
  virtual void createTransaction(DataHandle& /*dataHandle*/, const std::string& /*content*/) {}
  virtual void parse(DataHandle& /*dataHandle*/, const std::string& /*content*/) {}

  virtual bool startElement(int, const IAttributes&) { return false; }
  virtual bool endElement(int) { return false; }
};

class CommonRequestHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CommonRequestHandlerTest);

  CPPUNIT_TEST(testGetValue_string);

  CPPUNIT_TEST_SUITE_END();

private:
  TestCommonRequestHandler* _handler;
  Trx* _trx;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<AncillaryPricingTrx>();

    _handler = _memHandle.insert(new TestCommonRequestHandler(_trx));

    _handler->_pricingTrx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    _handler->_request = _memHandle.create<AncRequest>();
    _handler->_pricingTrx->setRequest(_handler->_request);

    _handler->_pricingTrx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _handler->_pricingTrx->getRequest()->ticketingAgent()->agentLocation() =
        _memHandle.create<Loc>();

    _handler->_itin = _memHandle.create<Itin>();
    _handler->_itin->travelSeg().push_back(_memHandle.create<AirSeg>());

    _memHandle.insert(new MyDataHandle(_memHandle, _handler->_pricingTrx, _handler->_itin));
  }

  void tearDown() { _memHandle.clear(); }

  void setAgentLocation(const LocCode& loc)
  {
    Loc* location =
        const_cast<Loc*>(_handler->_pricingTrx->getRequest()->ticketingAgent()->agentLocation());
    location->loc() = loc;
  }

  void setSegmentOriginAndDepartureDate(const LocCode& originAirport, const DateTime& departureDate)
  {
    _handler->_itin->travelSeg()[0]->origAirport() = originAirport;
    _handler->_itin->travelSeg()[0]->departureDT() = departureDate;
  }

  void testGetValue_string()
  {
    const std::string testText = "test text";
    _handler->characters(testText.c_str(), 9);

    std::string valueStr1;
    _handler->getValue(valueStr1);

    CPPUNIT_ASSERT(valueStr1 == testText);

    std::string valueStr2;
    _handler->characters(0, 0);
    _handler->getValue(valueStr2);
    CPPUNIT_ASSERT(valueStr2.empty());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(CommonRequestHandlerTest);
}
// namespace
