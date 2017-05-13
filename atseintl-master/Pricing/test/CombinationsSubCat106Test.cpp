#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "Diagnostic/DiagCollector.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CarrierCombination.h"
#include "Pricing/CombinationsSubCat106.h"

namespace tse
{

class CombinationsSubCat106Stub : public CombinationsSubCat106
{
public:
  CombinationsSubCat106Stub(PricingTrx& trx,
                            DiagCollector& diag,
                            const VendorCode& vendor,
                            const uint32_t itemNo,
                            const PricingUnit& pu,
                            const FareUsage& fu,
                            Combinations::ValidationFareComponents& components,
                            bool& negativeApplication)
    : CombinationsSubCat106(
          trx, diag, vendor, itemNo, pu, fu, components, negativeApplication)
  {
  }

  virtual ~CombinationsSubCat106Stub() {}

protected:
  virtual bool isInLoc(const LocCode& validatingLoc,
                       const Indicator restrictionLocType,
                       const LocCode& restrictionLoc,
                       const VendorCode& validatingVendor) const
  {
    return validatingLoc == restrictionLoc;
  }
};

class CombinationsSubCat106Test : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
  public:
    MyDataHandle()
    {
      CarrierCombination* comb = _memHandle.create<CarrierCombination>();
      combs.push_back(comb);
    }

    const std::vector<CarrierCombination*>&
    getCarrierCombination(const VendorCode& vendor, const int itemNo)
    {
      return combs;
    }

    TestMemHandle _memHandle;
    std::vector<CarrierCombination*> combs;
  };

  CPPUNIT_TEST_SUITE(CombinationsSubCat106Test);

  CPPUNIT_TEST(testGEOLoc1Empty);
  CPPUNIT_TEST(testGEOLoc1OrigMatch);
  CPPUNIT_TEST(testGEOLoc1DestMatch);
  CPPUNIT_TEST(testGEOLoc1NoMatch);
  CPPUNIT_TEST(testGEOLoc2OrigDestMatch);
  CPPUNIT_TEST(testGEOLoc2DestOrigMatch);
  CPPUNIT_TEST(testGEOLoc2OrigNoMatch);
  CPPUNIT_TEST(testGEOLoc2DestNoMatch);
  CPPUNIT_TEST(testGEOLoc2NothingMatch);

  CPPUNIT_TEST(testMatch_NoCarrierComb_Fail);
  CPPUNIT_TEST(testMatch_Pass);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    DiagCollector* diag = _memHandle.create<DiagCollector>();
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    FareUsage* fu = _memHandle.create<FareUsage>();
    Combinations::ValidationFareComponents* components =
        _memHandle(new Combinations::ValidationFareComponents);
    bool* negativeApplication = _memHandle(new bool(false));
    static const VendorCode vendor = "ATP";
    _subCat106 = _memHandle(new CombinationsSubCat106Stub(
        *trx, *diag, vendor, 0, *pu, *fu, *components, *negativeApplication));
    _targetFu = _memHandle.create<FareUsage>();
    _targetFu->paxTypeFare() = _memHandle.create<PaxTypeFare>();
    _targetFu->paxTypeFare()->fareMarket() = _memHandle.create<FareMarket>();
    _targetFu->rec2Cat10() = _memHandle.create<CombinabilityRuleInfo>();
    _targetFu->paxTypeFare()->fareMarket()->boardMultiCity() = "KRK";
    _targetFu->paxTypeFare()->fareMarket()->offMultiCity() = "DFW";
    _cxrComb = _memHandle.create<CarrierCombination>();
    _dbHandle = _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void callAndAssert(char expectedRet)
  {
    char realRet = Combinations::IDLE;
    _subCat106->matchGEO(realRet, *_cxrComb, *_targetFu->paxTypeFare(), *_targetFu->rec2Cat10());
    CPPUNIT_ASSERT_EQUAL(expectedRet, realRet);
  }

  void setUpCxrCombLocs(const LocCode& loc1, const LocCode& loc2)
  {
    _cxrComb->loc1() = loc1;
    _cxrComb->loc2() = loc2;
  }

  void testGEOLoc1Empty() { callAndAssert(Combinations::IDLE); }

  void testGEOLoc1OrigMatch()
  {
    setUpCxrCombLocs("KRK", "");
    callAndAssert(Combinations::MATCH);
  }

  void testGEOLoc1DestMatch()
  {
    setUpCxrCombLocs("DFW", "");
    callAndAssert(Combinations::MATCH);
  }

  void testGEOLoc1NoMatch()
  {
    setUpCxrCombLocs("WAS", "");
    callAndAssert(Combinations::NO_MATCH);
  }

  void testGEOLoc2OrigDestMatch()
  {
    setUpCxrCombLocs("KRK", "DFW");
    callAndAssert(Combinations::MATCH);
  }

  void testGEOLoc2DestOrigMatch()
  {
    setUpCxrCombLocs("DFW", "KRK");
    callAndAssert(Combinations::MATCH);
  }

  void testGEOLoc2OrigNoMatch()
  {
    setUpCxrCombLocs("WAS", "DFW");
    callAndAssert(Combinations::NO_MATCH);
  }

  void testGEOLoc2DestNoMatch()
  {
    setUpCxrCombLocs("KRK", "WAS");
    callAndAssert(Combinations::NO_MATCH);
  }

  void testGEOLoc2NothingMatch()
  {
    setUpCxrCombLocs("WAS", "YTO");
    callAndAssert(Combinations::NO_MATCH);
  }

  void testMatch_NoCarrierComb_Fail()
  {
    _dbHandle->combs.clear();
    CPPUNIT_ASSERT(!_subCat106->match());
  }

  void testMatch_Pass()
  {
    CPPUNIT_ASSERT(_subCat106->match());
  }

protected:
  CombinationsSubCat106* _subCat106;
  TestMemHandle _memHandle;
  MyDataHandle* _dbHandle;
  FareUsage* _targetFu;
  CarrierCombination* _cxrComb;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationsSubCat106Test);
}
