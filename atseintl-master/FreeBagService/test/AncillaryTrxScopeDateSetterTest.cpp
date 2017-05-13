#include "Common/DateTime.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "FreeBagService/AncillaryTrxScopeDateSetter.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class AncillaryTrxScopeDateSetterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryTrxScopeDateSetterTest);

  CPPUNIT_TEST(test_d07Exist);
  CPPUNIT_TEST(test_d07DoesntExist);
  CPPUNIT_TEST(test_switchTicketingDate_if_WPBaggage);
  CPPUNIT_TEST(test_switchTicketingDate_if_SWS);
  CPPUNIT_TEST(test_switchTicketingDate_if_MISC6);
  CPPUNIT_TEST(test_noSwitchTicketingDate_if_not_MISC6_not_SWS_not_WPBaggage);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _itin = _memHandle.create<Itin>();
    _ancTrx = _memHandle.create<AncillaryPricingTrx>();
    _ancReq = _memHandle.create<AncRequest>();

    _localTime = _memHandle.insert(new DateTime(DateTime::localTime()));
    _d07DateForItin = _memHandle.insert(new DateTime(2011, 10, 2));

    _ancReq->ticketingDatesPerItin().insert(std::make_pair(_itin, *_d07DateForItin));
    _ancReq->ancRequestType() = AncRequest::WPBGRequest;

    _ancTrx->setRequest(_ancReq);
    _ancTrx->dataHandle().setTicketDate(*_localTime);
    _ancTrx->ticketingDate() = *_localTime;
    _ancTrx->dataHandle().useTLS() = false;

    _ancTrx->billing() = _memHandle.create<Billing>();
    _ancTrx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    _ancTrx->billing()->actionCode() = "MISC6";
    _ancReq->ancRequestType() = AncRequest::WPBGRequest;
  }

  void tearDown() { _memHandle.clear(); }

  void test_d07Exist()
  {
    AncillaryTrxScopeDateSetter dateSetter(*_ancTrx);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_localTime);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_localTime);

    dateSetter.updateDate(_itin);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_d07DateForItin);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_d07DateForItin);
  }

  void test_d07DoesntExist()
  {
    AncillaryTrxScopeDateSetter dateSetter(*_ancTrx);
    Itin* itinForWhichD07WasNotDefined = _memHandle.create<Itin>();

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_localTime);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_localTime);

    dateSetter.updateDate(itinForWhichD07WasNotDefined);

    _localTime->setHistoricalIncludesTime();
    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_localTime);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_localTime);
  }

  void test_switchTicketingDate_if_WPBaggage()
  {
    _ancTrx->billing() = _memHandle.create<Billing>();
    _ancTrx->billing()->requestPath() = "";
    _ancTrx->billing()->actionCode() = "";
    _ancReq->ancRequestType() = AncRequest::WPBGRequest;

    AncillaryTrxScopeDateSetter dateSetter(*_ancTrx);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_localTime);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_localTime);

    dateSetter.updateDate(_itin);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_d07DateForItin);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_d07DateForItin);
  }

  void test_switchTicketingDate_if_SWS()
  {
    _ancTrx->billing() = _memHandle.create<Billing>();
    _ancTrx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    _ancTrx->billing()->actionCode() = "";
    _ancReq->ancRequestType() = AncRequest::M70Request;

    AncillaryTrxScopeDateSetter dateSetter(*_ancTrx);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_localTime);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_localTime);

    dateSetter.updateDate(_itin);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_d07DateForItin);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_d07DateForItin);
  }

  void test_switchTicketingDate_if_MISC6()
  {
    _ancTrx->billing() = _memHandle.create<Billing>();
    _ancTrx->billing()->requestPath() = "";
    _ancTrx->billing()->actionCode() = "MISC6";
    _ancReq->ancRequestType() = AncRequest::M70Request;

    AncillaryTrxScopeDateSetter dateSetter(*_ancTrx);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_localTime);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_localTime);

    dateSetter.updateDate(_itin);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_d07DateForItin);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_d07DateForItin);
  }

  void test_noSwitchTicketingDate_if_not_MISC6_not_SWS_not_WPBaggage()
  {
    _ancTrx->billing() = _memHandle.create<Billing>();
    _ancTrx->billing()->requestPath() = "";
    _ancTrx->billing()->actionCode() = "";
    _ancReq->ancRequestType() = AncRequest::M70Request;

    AncillaryTrxScopeDateSetter dateSetter(*_ancTrx);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_localTime);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_localTime);

    dateSetter.updateDate(_itin);

    CPPUNIT_ASSERT_EQUAL(_ancTrx->dataHandle().ticketDate(), *_d07DateForItin);
    CPPUNIT_ASSERT_EQUAL(_ancTrx->ticketingDate(), *_localTime);
  }

private:
  TestMemHandle _memHandle;
  AncillaryPricingTrx* _ancTrx;
  AncRequest* _ancReq;
  DateTime* _d07DateForItin;
  DateTime* _localTime;
  Itin* _itin;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryTrxScopeDateSetterTest);
}
