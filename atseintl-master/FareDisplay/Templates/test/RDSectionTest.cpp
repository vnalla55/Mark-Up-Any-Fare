#include "DataModel/FareDisplayOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/RDSection.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class RDSectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RDSectionTest);

  CPPUNIT_TEST(addCategoryLine_TestIC);
  CPPUNIT_TEST(addCategoryLine_TestFB);
  CPPUNIT_TEST(addCategoryLine_TestCat1);
  CPPUNIT_TEST(testAddRetailerCategoryDetails);

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
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _options = _memHandle.create<FareDisplayOptions>();;
    _trx->setOptions(_options);
    _rds = _memHandle.insert(new RDSection(*_trx));
    _fareDisplayInfo = _memHandle.create<FareDisplayInfo>();
    _oss = _memHandle.create<std::ostringstream>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void addCategoryLine_TestIC()
  {
    CPPUNIT_ASSERT(_rds->addCatDescription(IC_RULE_CATEGORY, *_fareDisplayInfo, _oss));
    CPPUNIT_ASSERT_EQUAL(std::string("IC.\n"), _oss->str());
  }

  void addCategoryLine_TestFB()
  {
    CPPUNIT_ASSERT(_rds->addCatDescription(RETAILER_CATEGORY, *_fareDisplayInfo, _oss));
    CPPUNIT_ASSERT_EQUAL(std::string("RR.\n"), _oss->str());
  }

  void addCategoryLine_TestCat1()
  {
    CPPUNIT_ASSERT(_rds->addCatDescription(RuleCategories::ELIGIBILITY_RULE, *_fareDisplayInfo, _oss));
    CPPUNIT_ASSERT_EQUAL(std::string("01.\n"), _oss->str());
  }

  void testAddRetailerCategoryDetails()
  {
    const CatNumber catNumber = RETAILER_CATEGORY;
    _options->retailerDisplay() = TRUE_INDICATOR;
    PaxTypeFare paxTypeFare;
    _rds->addRetailerCategoryDetails(catNumber, paxTypeFare);

    bool isEmpty = _trx->response().str().empty();
    CPPUNIT_ASSERT_EQUAL(isEmpty, true);
  }

private:
  TestMemHandle _memHandle;
  FareDisplayInfo* _fareDisplayInfo;
  std::ostringstream* _oss;
  RDSection* _rds;
  FareDisplayTrx* _trx;
  FareDisplayOptions* _options;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RDSectionTest);
}
