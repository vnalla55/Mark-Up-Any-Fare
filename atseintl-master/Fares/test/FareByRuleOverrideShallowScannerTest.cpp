#include <vector>

#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "Fares/FareByRuleOverrideShallowScanner.h"
#include "Rules/RuleConst.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class FareByRuleOverrideShallowScannerTest : public CppUnit::TestFixture
{

  class MockFareByRuleOverrideShallowScanner : public FareByRuleOverrideShallowScanner
  {
  public:
    MockFareByRuleOverrideShallowScanner(PricingTrx& trx,
                                         const FareMarket& fareMarket,
                                         Itin& itin,
                                         const FareByRuleApp& fbrApp,
                                         const FareByRuleCtrlInfo& fbrCtrlInfo,
                                         const FareByRuleItemInfo& fbrItemInfo,
                                         PaxTypeFare& dummyPtFare,
                                         TestMemHandle& memHandle)
      : FareByRuleOverrideShallowScanner(
            trx, fareMarket, itin, fbrApp, fbrCtrlInfo, fbrItemInfo, dummyPtFare, BLANK),
        _memHandle(memHandle),
        _getAllCallCount(0),
        _validateCallCount(0)
    {
      GeneralFareRuleInfo* rec2;
      _memHandle.get(rec2);
      _rec2s.push_back(rec2);
    }

    const std::vector<GeneralFareRuleInfo*>& getAllPossiblyMatchingRec2s(uint16_t categoryNumber)
    {
      ++_getAllCallCount;
      return _rec2s;
    }

    bool validate(const Rec2Wrapper& rec2)
    {
      ++_validateCallCount;
      return true;
    }

    int getAllCallCount() const { return _getAllCallCount; }
    int validateCallCount() const { return _validateCallCount; }

  private:
    std::vector<GeneralFareRuleInfo*> _rec2s;
    TestMemHandle& _memHandle;

    int _getAllCallCount;
    int _validateCallCount;
  };

  class MyDataHandle : public DataHandleMock
  {
  public:
    const std::vector<MultiAirportCity*>& getMultiCityAirport(const LocCode& city)
    {
      return *_memHandle.create<std::vector<MultiAirportCity*> >();
    }

  private:
    TestMemHandle _memHandle;
  };

  CPPUNIT_TEST_SUITE(FareByRuleOverrideShallowScannerTest);
  CPPUNIT_TEST(passSystemAssumptionTest);
  CPPUNIT_TEST(isValidTest);
  CPPUNIT_TEST(isValidTest2);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    TestFixture::setUp();
    _trx.setOptions(&_options);
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _ptFare.fare()->setFareInfo(_memHandle.create<FareInfo>());

  }

  void tearDown() { _memHandle.clear(); }

  void passSystemAssumptionTest()
  {
    MockFareByRuleOverrideShallowScanner scanner(
        _trx, _fareMarket, _itin, _fbrApp, _fbrCtrlInfo, _fbrItemInfo, _ptFare, _memHandle);
    scanner._skipSecurity = false;
    CPPUNIT_ASSERT(scanner.passSystemAssumption(35));

    scanner._skipSecurity = true;
    scanner._tarrifCategory = RuleConst::PRIVATE_TARIFF;
    CPPUNIT_ASSERT(!scanner.passSystemAssumption(35));

    scanner._skipSecurity = false;
    CPPUNIT_ASSERT(!scanner.passSystemAssumption(15));

    scanner._skipSecurity = true;
    CPPUNIT_ASSERT(scanner.passSystemAssumption(15));

    scanner._skipSecurity = false;
    CPPUNIT_ASSERT(scanner.passSystemAssumption(1)); // any other
  }

  void isValidTest()
  {
    MockFareByRuleOverrideShallowScanner scanner(
        _trx, _fareMarket, _itin, _fbrApp, _fbrCtrlInfo, _fbrItemInfo, _ptFare, _memHandle);
    scanner._fbrShallowScanCategories.push_back(1);

    CPPUNIT_ASSERT(scanner.isValid());

    CPPUNIT_ASSERT(scanner.getAllCallCount() == 1);
    CPPUNIT_ASSERT(scanner.validateCallCount() == 1);
  }

  void isValidTest2()
  {
    MockFareByRuleOverrideShallowScanner scanner(
        _trx, _fareMarket, _itin, _fbrApp, _fbrCtrlInfo, _fbrItemInfo, _ptFare, _memHandle);
    scanner._fbrShallowScanCategories.push_back(1);
    scanner._fbrShallowScanCategories.push_back(15);

    CPPUNIT_ASSERT(scanner.isValid());
    CPPUNIT_ASSERT(scanner.getAllCallCount() == 2);
    CPPUNIT_ASSERT(scanner.validateCallCount() == 2);
  }

private:
  TestMemHandle _memHandle;
  PricingTrx _trx;
  PricingOptions _options;
  FareMarket _fareMarket;
  Itin _itin;
  FareByRuleApp _fbrApp;
  FareByRuleCtrlInfo _fbrCtrlInfo;
  FareByRuleItemInfo _fbrItemInfo;
  PaxTypeFare _ptFare;
  MyDataHandle* _mdh;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareByRuleOverrideShallowScannerTest);
}
