#include "DBAccess/Record2Types.h"
#include "DBAccess/RuleItemInfo.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Rules/CategoryRuleItem.h"
#include "Rules/test/MockRuleControllerDataAccess.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/TransfersInfoWrapper.h"
#include "Rules/RuleConst.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{
class CategoryRuleItem_doMainCategoryTest : public CppUnit::TestFixture
{
  class CategoryRuleItemMainCategoryMock : public CategoryRuleItem
  {
  public:
    Record3ReturnTypes doMainCategory(PricingTrx& trx,
                                      Itin& itin,
                                      const CategoryRuleInfo& cri,
                                      PaxTypeFare& paxTypeFare,
                                      const std::vector<CategoryRuleItemInfo>& cfrItem,
                                      bool isLocationSwapped,
                                      bool& isCat15Security,
                                      RuleProcessingData& rpData,
                                      bool isFareRule,
                                      bool skipCat15Security,
                                      RuleControllerDataAccess& da)
    {
      return CategoryRuleItem::doMainCategory(trx,
                                              itin,
                                              cri,
                                              paxTypeFare,
                                              cfrItem,
                                              isLocationSwapped,
                                              isCat15Security,
                                              rpData,
                                              isFareRule,
                                              skipCat15Security,
                                              da);
    }
  };

  class MyDataHandle : public DataHandleMock
  {
    const RuleItemInfo* getRuleItemInfo(const CategoryRuleInfo* rule,
                                        const CategoryRuleItemInfo* item,
                                        const DateTime& applDate = DateTime::emptyDate())
    {
      return &_rii;
  //    return DataHandleMock::getRuleItemInfo(rule, item, applDate);
    }

    RuleItemInfo _rii;
  };

  TestMemHandle _memH;
  CategoryRuleItemMainCategoryMock* _crItem;
  PricingTrx* _trx;
  Itin* _itin;
  PaxTypeFare* _paxTypeFare;
  CategoryRuleInfo* _cri;
  std::vector<CategoryRuleItemInfo>* _cfrItem;
  RuleProcessingData* _rpData;
  RuleControllerDataAccess* _da;
  TransfersInfoWrapper* _trInfoWrapper;
  CategoryRuleItemInfo* _crii1;

  CPPUNIT_TEST_SUITE(CategoryRuleItem_doMainCategoryTest);
  CPPUNIT_TEST(default_SKIP);
  CPPUNIT_TEST(noCategoryRuleItem_SKIP);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _memH.create<MyDataHandle>();
    _crItem = _memH.create<CategoryRuleItemMainCategoryMock>();
    _trx = _memH.create<PricingTrx>();
    _trx->setOptions(_memH.create<PricingOptions>());
    _itin = _memH.create<Itin>();
    _paxTypeFare = _memH.create<PaxTypeFare>();
    _paxTypeFare->fareMarket() = _memH.create<FareMarket>();
    _cri = _memH.create<CategoryRuleInfo>();
    _cfrItem = _memH.create<std::vector<CategoryRuleItemInfo> >();
    CategoryRuleItemInfo _crii1;
    _crii1.setItemcat(9);
    _cfrItem->push_back(_crii1);
    _rpData = _memH.create<RuleProcessingData>();
    _da = _memH.create<MockRuleControllerDataAccess>();
    _trInfoWrapper = _memH.create<TransfersInfoWrapper>();
  }
  void tearDown() { _memH.clear(); }

  void default_SKIP()
  {
    bool isCat15Security = false;
    _rpData->trInfoWrapper(_trInfoWrapper);

    CPPUNIT_ASSERT_EQUAL(SKIP,
                         _crItem->doMainCategory(*_trx,
                                                 *_itin,
                                                 *_cri,
                                                 *_paxTypeFare,
                                                 *_cfrItem,
                                                 false,
                                                 isCat15Security,
                                                 *_rpData,
                                                 false,
                                                 false,
                                                 *_da));
  }

  void noCategoryRuleItem_SKIP()
  {
    bool isCat15Security = false;
    _cfrItem->clear();
    CPPUNIT_ASSERT_EQUAL(SKIP,
                         _crItem->doMainCategory(*_trx,
                                                 *_itin,
                                                 *_cri,
                                                 *_paxTypeFare,
                                                 *_cfrItem,
                                                 false,
                                                 isCat15Security,
                                                 *_rpData,
                                                 false,
                                                 false,
                                                 *_da));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CategoryRuleItem_doMainCategoryTest);

} // tse
