#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Rules/RuleController.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Loc.h"
#include "Rules/RuleUtil.h"

#include <vector>
#include <iostream>
#include <set>

namespace tse
{
namespace
{
class TestRuleController : public RuleController
{
public:
  void
  reuseWM(const WarningMap& srcWarningMap, const WarningMap& trgWarningMap, uint16_t categoryNumber)
      const
  {
    reuseWarningMap(srcWarningMap, trgWarningMap, categoryNumber);
  }

protected:
  virtual Record3ReturnTypes callCategoryRuleItemSet(CategoryRuleItemSet& catRuleIS,
                                                     const CategoryRuleInfo&,
                                                     const std::vector<CategoryRuleItemInfoSet*>&,
                                                     RuleControllerDataAccess& da,
                                                     RuleProcessingData& rpData,
                                                     bool isLocationSwapped,
                                                     bool isFareRule,
                                                     bool skipCat15Security)
  {
    return NOTPROCESSED;
  }

  /*    virtual */ void applySurchargeGenRuleForFMS(PricingTrx& trx,
                                                    RuleControllerDataAccess& da,
                                                    uint16_t categoryNumber,
                                                    RuleControllerParam& rcParam,
                                                    bool skipCat15Security)
  {
  }

  /* virtual */ Record3ReturnTypes applySystemDefaultAssumption(PricingTrx& trx,
                                                                RuleControllerDataAccess& da,
                                                                const uint16_t category,
                                                                bool& displayDiag)
  {
    return NOTPROCESSED;
  }

  /* virtual*/ Record3ReturnTypes validateBaseFare(uint16_t category,
                                                   const FareByRuleItemInfo* fbrItemInfo,
                                                   bool& checkFare,
                                                   PaxTypeFare* fbrBaseFare,
                                                   RuleControllerDataAccess& da)
  {
    return NOTPROCESSED;
    ;
  }

  /* virtual*/ Record3ReturnTypes revalidateC15BaseFareForDisc(uint16_t category,
                                                               bool& checkFare,
                                                               PaxTypeFare* ptf,
                                                               RuleControllerDataAccess& da)
  {
    return NOTPROCESSED;
    ;
  }

  /*virtual*/ Record3ReturnTypes doCategoryPostProcessing(PricingTrx& trx,
                                                          RuleControllerDataAccess& da,
                                                          const uint16_t category,
                                                          RuleProcessingData& rpData,
                                                          const Record3ReturnTypes preResult)
  {
    return NOTPROCESSED;
    ;
  }
};
}

class RuleControllerReuseWarningTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleControllerReuseWarningTest);
  CPPUNIT_TEST(testReuseWarningMap);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testReuseWarningMap()
  {
    TestRuleController rc;

    WarningMap warningMap1;
    WarningMap warningMap2;

    warningMap1.set(WarningMap::cat2_warning_1);
    warningMap1.set(WarningMap::cat2_warning_2);
    warningMap1.set(WarningMap::cat5_warning_1);

    rc.reuseWM(warningMap1, warningMap2, 2);
    rc.reuseWM(warningMap1, warningMap2, 5);

    CPPUNIT_ASSERT(warningMap2.isSet(WarningMap::cat2_warning_1));
    CPPUNIT_ASSERT(warningMap2.isSet(WarningMap::cat2_warning_2));
    CPPUNIT_ASSERT(warningMap2.isSet(WarningMap::cat5_warning_1));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(RuleControllerReuseWarningTest);
}
