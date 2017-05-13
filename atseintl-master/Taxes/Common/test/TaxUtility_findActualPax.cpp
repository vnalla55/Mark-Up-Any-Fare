#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/DiscountInfo.h"
#include "Taxes/Common/TaxUtility.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestXMLHelper.h"

#include <string>

namespace tse
{

class TaxUtility_findActualPax : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUtility_findActualPax);
  CPPUNIT_TEST(findActualPaxType_reqADT_fareADT);
  CPPUNIT_TEST(findActualPaxType_reqCNN_fareADT);
  CPPUNIT_TEST(findActualPaxType_reqCNN_fareCNN);
  CPPUNIT_TEST(findActualPaxType_reqINF_fareCNN);
  CPPUNIT_TEST(findActualPaxType_reqCNN_fareADT_isDiscount);
  CPPUNIT_TEST(findActualPaxType_reqINF_fareCNN_isDiscount);
  CPPUNIT_TEST_SUITE_END();

  FarePath _fp;
  TestMemHandle _memHandle;

  PricingUnit _pu;
  FareUsage _fu;
  FareUsage _fu2;
  AirSeg _ts;
  AirSeg _ts2;
  PaxTypeFare _ptf;
  PaxTypeFare _ptf2;
  Itin _it;
  PaxType _pt1;
  PaxType _pt2;
  DiscountInfo _di;
  PaxTypeFare::PaxTypeFareAllRuleData _ptfard;
  PaxTypeFareRuleData _ptfrd;
  PricingTrx _trx;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fp.itin() = &_it;
    _it.travelSeg().push_back(&_ts);
    _it.travelSeg().push_back(&_ts2);
    _fp.pricingUnit().push_back(&_pu);
    _pu.fareUsage().push_back(&_fu);
    _pu.fareUsage().push_back(&_fu2);
    _fu.travelSeg().push_back(&_ts);
    _fu2.travelSeg().push_back(&_ts2);
    _fu.paxTypeFare() = &_ptf;
    _fu2.paxTypeFare() = &_ptf2;
    _ptf.actualPaxType() = &_pt1;
    _ptf2.actualPaxType() = &_pt1;
    _ptfard.fareRuleData =  &_ptfrd;
    _ptfrd.ruleItemInfo() = &_di;
    (*_ptf2.paxTypeFareRuleDataMap())[19] = &_ptfard;
    _di.category() = 19;
    _ptf2.status().set(PaxTypeFare::PTF_Discounted, true);
    _fp.paxType() = &_pt2;
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void findActualPaxType_reqADT_fareADT()
  {
    _pt1.paxType() = "ADT";
    _pt2.paxType() = "ADT";
    const PaxType* pt = taxUtil::findActualPaxType(_trx, &_fp, 0);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ADT"),  pt->paxType());
  }

  void findActualPaxType_reqCNN_fareADT()
  {
    _pt1.paxType() = "ADT";
    _pt2.paxType() = "CNN";
    const PaxType* pt = taxUtil::findActualPaxType(_trx, &_fp, 0);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ADT"),  pt->paxType());
  }

  void findActualPaxType_reqCNN_fareCNN()
  {
    _pt1.paxType() = "CNN";
    _pt2.paxType() = "CNN";
    const PaxType* pt = taxUtil::findActualPaxType(_trx, &_fp, 0);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("CNN"),  pt->paxType());
  }

  void findActualPaxType_reqINF_fareCNN()
  {
    _pt1.paxType() = "CNN";
    _pt2.paxType() = "INF";
    const PaxType* pt = taxUtil::findActualPaxType(_trx, &_fp, 0);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("CNN"),  pt->paxType());
  }

  void findActualPaxType_reqCNN_fareADT_isDiscount()
  {
    _pt1.paxType() = "ADT";
    _pt2.paxType() = "CNN";
    const PaxType* pt = taxUtil::findActualPaxType(_trx, &_fp, 1);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("CNN"),  pt->paxType());
  }

  void findActualPaxType_reqINF_fareCNN_isDiscount()
  {
    _pt1.paxType() = "CNN";
    _pt2.paxType() = "INF";
    const PaxType* pt = taxUtil::findActualPaxType(_trx, &_fp, 1);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("CNN"),  pt->paxType());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUtility_findActualPax);
};
