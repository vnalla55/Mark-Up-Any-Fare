#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/ExcItin.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class BaseExchangeTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaseExchangeTrxTest);

  CPPUNIT_TEST(testSetActionCodeWhenActionCodeHas3Characters);
  CPPUNIT_TEST(testSetActionCodeWhenActionCodeHasMoreThen3Characters);
  CPPUNIT_TEST(testSetItinIndex);
  CPPUNIT_TEST(testGetItinPos);
  CPPUNIT_TEST(testGetItinPos_motherItinIndex);

  CPPUNIT_TEST_SUITE_END();

public:
  ExchangePricingTrx* _exchangePricingTrx;
  RexPricingTrx* _rexTrx;
  Billing* _billing;
  FareCompInfo* _fareCompInfo;
  ExcItin* _excItin;
  TestMemHandle _memHandle;

  void setUp()
  {
    _exchangePricingTrx = new ExchangePricingTrx();
    _rexTrx = new RexPricingTrx();
    _billing = new Billing();
    _fareCompInfo = new FareCompInfo();
    _excItin = new ExcItin();

    _exchangePricingTrx->billing() = _billing;
    _rexTrx->billing() = _billing;
    _excItin->fareComponent().push_back(_fareCompInfo);
    _rexTrx->exchangeItin().push_back(_excItin);

  }

  void tearDown()
  {
    delete _fareCompInfo;
    delete _excItin;
    delete _billing;
    delete _rexTrx;
    delete _exchangePricingTrx;
    _memHandle.clear();
  }

  void testSetActionCodeWhenActionCodeHas3Characters()
  {
    std::string actionCode = "WFR";

    _exchangePricingTrx->billing()->actionCode() = actionCode;
    _exchangePricingTrx->reqType() = FULL_EXCHANGE;

    _exchangePricingTrx->setActionCode();

    CPPUNIT_ASSERT_EQUAL(actionCode + FULL_EXCHANGE, _exchangePricingTrx->billing()->actionCode());
  }

  void testSetActionCodeWhenActionCodeHasMoreThen3Characters()
  {
    std::string actionCode = "WFRF";

    _rexTrx->billing()->actionCode() = actionCode;

    _rexTrx->setActionCode();

    CPPUNIT_ASSERT_EQUAL(actionCode.substr(0, 3) + AUTOMATED_REISSUE,
                         _rexTrx->billing()->actionCode());
  }

  void testSetItinIndex()
  {
    uint16_t itinIndex = 0;
    Itin* itin = new Itin();
    _rexTrx->itin().push_back(itin);
    _rexTrx->setItinIndex(itinIndex);

    CPPUNIT_ASSERT_EQUAL(_rexTrx->_itinIndex, itinIndex);
    CPPUNIT_ASSERT_EQUAL(_rexTrx->exchangeItin().front()->_excItinIndex, itinIndex);
    CPPUNIT_ASSERT_EQUAL(_rexTrx->exchangeItin().front()->fareComponent().front()->_itinIndex,
                         itinIndex);
  }

  void testGetItinPos()
  {
    uint16_t itinIndex = 0;
    Itin* itin = new Itin();
    _rexTrx->itin().push_back(itin);

    CPPUNIT_ASSERT_EQUAL(_rexTrx->getItinPos(itin), itinIndex);
  }

  void testGetItinPos_motherItinIndex()
  {
    uint16_t itinIndex = 4;
    Itin* itin = new Itin();
    _rexTrx->_motherItinIndex[itin] = itinIndex;
    _rexTrx->itin().push_back(itin);

    CPPUNIT_ASSERT_EQUAL(_rexTrx->getItinPos(itin), itinIndex);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(BaseExchangeTrxTest);

} // end namespace
