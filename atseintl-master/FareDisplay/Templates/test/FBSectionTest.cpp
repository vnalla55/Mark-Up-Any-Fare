#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/FBSection.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FBSectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FBSectionTest);

  CPPUNIT_TEST(addCategoryLine_TestIC);
  CPPUNIT_TEST(addCategoryLine_TestFB);
  CPPUNIT_TEST(addCategoryLine_TestCat1);

  CPPUNIT_TEST_SUITE_END();

private:
  class MyDataHandle : public DataHandleMock
  {
    RuleCategoryDescInfo _ret;

  public:
    const RuleCategoryDescInfo*
    getRuleCategoryDesc(const CatNumber& key)
    {
      return &_ret;
    }
  };

public:

  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _fbs = _memHandle.insert(new FBSection(*_trx));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void addCategoryLine_TestIC()
  {
    _fbs->addCategoryLine(IC_RULE_CATEGORY);
    CPPUNIT_ASSERT_EQUAL(std::string("IC  -\n"), _trx->response().str());
  }

  void addCategoryLine_TestFB()
  {
    _fbs->addCategoryLine(RETAILER_CATEGORY);
    CPPUNIT_ASSERT_EQUAL(std::string("RR  -\n"), _trx->response().str());
  }

  void addCategoryLine_TestCat1()
  {
    _fbs->addCategoryLine(RuleCategories::ELIGIBILITY_RULE);
    CPPUNIT_ASSERT_EQUAL(std::string("01  -\n"), _trx->response().str());
  }

private:
  TestMemHandle _memHandle;
  FBSection* _fbs;
  FareDisplayTrx* _trx;

};
CPPUNIT_TEST_SUITE_REGISTRATION(FBSectionTest);
}
