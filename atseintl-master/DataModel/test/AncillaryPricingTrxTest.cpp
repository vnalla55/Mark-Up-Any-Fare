#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggagePolicy.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class AncillaryPricingTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryPricingTrxTest);
  CPPUNIT_TEST(testBaggagePolicy);
  CPPUNIT_TEST_SUITE_END();

private:
  AncillaryPricingTrx* _trx;
  DateTime* _ticketDateInTransaction;
  DateTime* _ticketDateInHandle;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<AncillaryPricingTrx>();
    _trx->setRequest(_memHandle.create<AncRequest>());

    _ticketDateInTransaction = _memHandle.insert(new DateTime(2011, 10, 2));
    _ticketDateInHandle = _memHandle.insert(new DateTime(2010, 3, 14));

    _trx->ticketingDate() = *_ticketDateInTransaction;
    _trx->dataHandle().setTicketDate(*_ticketDateInHandle);
  }

  void tearDown() { _memHandle.clear(); }

  void testBaggagePolicy()
  {
    CPPUNIT_ASSERT_EQUAL(BaggagePolicy::ALL_TRAVELS,
                         _trx->getBaggagePolicy().getDisclosurePolicy());
    CPPUNIT_ASSERT(_trx->getBaggagePolicy().changeTktDateForAncillaryCharges());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryPricingTrxTest);
}
