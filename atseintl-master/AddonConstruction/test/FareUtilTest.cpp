#include "AddonConstruction/AtpcoConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/FareUtil.h"
#include "AddonConstruction/SmfGatewayPair.h"
#include "AddonConstruction/test/ConstructionJobMock.h"
#include "Common/Logger.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <cmath>

namespace tse
{
class FareUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareUtilTest);

  CPPUNIT_TEST(testCalculateTotalAmountFailOnEmptyCurrency);
  CPPUNIT_TEST(testCalculateTotalAmountFailOnInvalidData);
  CPPUNIT_TEST(testCalculateTotalAmountFailOnEmptyCurrency2);
  CPPUNIT_TEST(testCalculateTotalAmountFailOnInvalidData2);
  CPPUNIT_TEST(testNucToFareCurrencyFail);

  CPPUNIT_TEST_SUITE_END();

  AtpcoConstructedFare _cf;
  PricingTrx* _trx;
  MoneyAmount _totalAmount;
  ConstructionJobMock* _cJob;
  ConstructedFareInfo _cfi;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _cf.specifiedFare() = _memHandle.create<FareInfo>();
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _cJob = _memHandle.insert(new ConstructionJobMock(*_trx));
    _cJob->setVendorCode(ATPCO_VENDOR_CODE);
    _cJob->carrier() = "AA";
  }

  void tearDown() { _memHandle.clear(); }

  void testCalculateTotalAmountFailOnEmptyCurrency()
  {
    CPPUNIT_ASSERT(!FareUtil::calculateTotalAmount(_cf, *_cJob, _totalAmount));
  }

  void testCalculateTotalAmountFailOnInvalidData()
  {
    const_cast<CurrencyCode&>(_cf.specifiedFare()->currency()) = NUC;
    const_cast<FareInfo*&>(_cf.specifiedFare())->fareAmount() =
        NAN; // Not A Number to avoid calling converter which uses DB connection
    const_cast<FareInfo*&>(_cf.specifiedFare())->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    CPPUNIT_ASSERT(!FareUtil::calculateTotalAmount(_cf, *_cJob, _totalAmount));
  }

  void testCalculateTotalAmountFailOnEmptyCurrency2()
  {
    CPPUNIT_ASSERT(!FareUtil::calculateTotalAmount(_cfi, *_cJob));
  }

  void testCalculateTotalAmountFailOnInvalidData2()
  {
    _cfi.fareInfo().currency() = NUC;
    _cfi.specifiedFareAmount() = NAN;
    _cfi.fareInfo().owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    CPPUNIT_ASSERT(!FareUtil::calculateTotalAmount(_cfi, *_cJob));
  }

  void testNucToFareCurrencyFail()
  {
    _cfi.fareInfo().currency() = NUC;
    _cfi.constructedNucAmount() = NAN;
    CPPUNIT_ASSERT(!FareUtil::nucToFareCurrency(_cfi, *_cJob));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareUtilTest);
} // end namespace tse
