#include "test/include/CppUnitHelperMacros.h"

#include "Common/TrxUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/FarePath.h"
#include "FreeBagService/FreeBagService.h"
#include "Server/TseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

// MOCKS
namespace
{
}

class FreeBagServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FreeBagServiceTest);
  CPPUNIT_TEST(checkFarePathEmptyItin);
  CPPUNIT_TEST(checkFarePathEmptyFarePath);
  CPPUNIT_TEST(checkFarePathFoundFarePath);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _freeBagService = _memHandle.insert(new FreeBagService("Test", *(TseServer::getInstance())));
  }

  void tearDown() { _memHandle.clear(); }

  void checkFarePathEmptyItin()
  {
    AncillaryPricingTrx* trx = _memHandle.create<AncillaryPricingTrx>();
    bool retVal = _freeBagService->checkFarePath(*trx);
    CPPUNIT_ASSERT(!retVal);
  }

  void checkFarePathEmptyFarePath()
  {
    AncillaryPricingTrx* trx = _memHandle.create<AncillaryPricingTrx>();
    trx->itin().push_back(_memHandle.create<Itin>());
    trx->itin().push_back(_memHandle.create<Itin>());

    bool retVal = _freeBagService->checkFarePath(*trx);
    CPPUNIT_ASSERT(!retVal);
  }

  void checkFarePathFoundFarePath()
  {
    AncillaryPricingTrx* trx = _memHandle.create<AncillaryPricingTrx>();
    Itin* itin = _memHandle.create<Itin>();
    itin->farePath().push_back(_memHandle.create<FarePath>());
    trx->itin().push_back(itin);

    bool retVal = _freeBagService->checkFarePath(*trx);
    CPPUNIT_ASSERT(retVal);
  }

protected:
  TestMemHandle _memHandle;
  DateTime _defaultActivationDate;
  FreeBagService* _freeBagService;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FreeBagServiceTest);
} // tse
