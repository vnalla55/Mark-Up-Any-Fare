
#include "test/include/CppUnitHelperMacros.h"

#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Taxes/Common/TaxUtility.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class TaxUtility_isTransitSeqTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUtility_isTransitSeqTest);
  CPPUNIT_TEST(isTransitSeq_NoTransitSeq_Test);
  CPPUNIT_TEST(isTransitSeq_NoTransitTaxonlyId_Test);
  CPPUNIT_TEST(isTransitSeq_IsTransitTaxonlyId_Test);
  CPPUNIT_TEST_SUITE_END();

  TaxCodeReg _taxCodeReg;
  TaxRestrictionTransit _taxRestrictionTransit;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _taxCodeReg.restrictionTransit().push_back(_taxRestrictionTransit);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void isTransitSeq_NoTransitSeq_Test()
  {
    TaxCodeReg taxCodeReg;
    CPPUNIT_ASSERT(!taxUtil::isTransitSeq(taxCodeReg));
  }

  void isTransitSeq_NoTransitTaxonlyId_Test()
  {
    _taxCodeReg.restrictionTransit().front().transitTaxonly() = tse::Indicator('N');

    CPPUNIT_ASSERT(!taxUtil::isTransitSeq(_taxCodeReg));
  }

  void isTransitSeq_IsTransitTaxonlyId_Test()
  {
    _taxCodeReg.restrictionTransit().front().transitTaxonly() = tse::Indicator('Y');

    CPPUNIT_ASSERT(taxUtil::isTransitSeq(_taxCodeReg));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUtility_isTransitSeqTest);
};
