
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "RexPricing/RefundDiscountApplier.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>
#include "gmock/gmock.h"

namespace tse
{

using namespace boost::assign;

enum Cat
{
  cat19 = 19,
  cat22 = 22
};

class RefundDiscountApplierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RefundDiscountApplierTest);

  CPPUNIT_TEST(testApplyDiscount_Percent);
  CPPUNIT_TEST(testApplyDiscount_Specified);
  CPPUNIT_TEST(testApplyDiscount_SpecifiedZero);

  CPPUNIT_TEST(testApplyDiscount_InfantWithSeat);
  CPPUNIT_TEST(testApplyDiscount_Child);
  CPPUNIT_TEST(testApplyDiscount_Youth);
  CPPUNIT_TEST(testApplyDiscount_Senior);
  CPPUNIT_TEST(testApplyDiscount_ChildOrInfant);
  CPPUNIT_TEST(testApplyDiscount_AnyCat22);
  CPPUNIT_TEST(testApplyDiscount_AllTags_NotDiscounted);

  CPPUNIT_TEST(testPaxTypeDiscountStatus_isInfantWithoutSeat);
  CPPUNIT_TEST(testPaxTypeDiscountStatus_isInfantWithSeat);
  CPPUNIT_TEST(testPaxTypeDiscountStatus_isChild);
  CPPUNIT_TEST(testPaxTypeDiscountStatus_isYouth);
  CPPUNIT_TEST(testPaxTypeDiscountStatus_isSenior);
  CPPUNIT_TEST(testPaxTypeDiscountStatus_isChildOrInfant);

  CPPUNIT_TEST(testApply_DiscountTagsEmpty);
  CPPUNIT_TEST(testApply_DiscountTagsHasInfantWithoutSeat);
  CPPUNIT_TEST(testApply_DiscountTagsHasOther);

  CPPUNIT_TEST(testGetDiscountTypes_DiscountTagsEmpty);
  CPPUNIT_TEST(testGetDiscountTypes_ValidValues);
  CPPUNIT_TEST(testGetDiscountTypes_InvalidValues);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memH;
  PaxType* _paxType;
  PaxTypeInfo* _paxTypeInfo;
  RefundDiscountApplier* _applier;

  void setUp()
  {
    _paxType = _memH.insert(new PaxType);
    _paxTypeInfo = _memH.insert(new PaxTypeInfo);
    _paxType->paxTypeInfo() = _paxTypeInfo;

    _applier = _memH.insert(new RefundDiscountApplier(*_paxType));
  }

  void tearDown() { _memH.clear(); }

  void assertMoney(const Money& expect, const Money& result)
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expect.value(), result.value(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(expect.code(), result.code());
  }



  DiscountInfo* createDiscount(Cat cat,
                               Percent perc,
                               const MoneyAmount& amt1 = 0.0,
                               const CurrencyCode& cur1 = "",
                               const MoneyAmount& amt2 = 0.0,
                               const CurrencyCode& cur2 = "")
  {
    DiscountInfo* info = _memH.insert(new DiscountInfo);
    info->category() = cat;
    info->discPercent() = perc;
    info->fareAmt1() = amt1;
    info->cur1() = cur1;
    info->fareAmt2() = amt2;
    info->cur2() = cur2;

    return info;
  }

  PaxType& createPaxType(Indicator pax)
  {
    PaxType* paxType = _memH.insert(new PaxType);
    PaxTypeInfo* info = _memH.insert(new PaxTypeInfo);
    info->childInd() = 'N';
    info->infantInd() = 'N';
    paxType->paxTypeInfo() = info;
    int moreThenOneSeat = 3;
    switch (pax)
    {
    case RefundDiscountApplier::INFANT_WITHOUT_SEAT:
      paxType->paxType() = "INF";
      info->infantInd() = 'Y';
      break;

    case RefundDiscountApplier::INFANT_WITH_SEAT:
      paxType->paxType() = "INS";
      info->infantInd() = 'Y';
      info->numberSeatsReq() = moreThenOneSeat;
      break;

    case RefundDiscountApplier::CHILD:
      paxType->paxType() = "CNN";
      info->childInd() = 'Y';
      break;

    case RefundDiscountApplier::YOUTH:
      paxType->paxType() = "YTH";
      break;

    case RefundDiscountApplier::SENIOR:
      paxType->paxType() = "SRC";
      break;

    case RefundDiscountApplier::CHILD_OR_INFANT:
      paxType->paxType() = "INN";
      info->infantInd() = 'Y';
      break;

    case RefundDiscountApplier::ANY_CAT22:
      paxType->paxType() = "MIL";
      break;

    default:
      ;
    }

    info->initPsgType();
    return *paxType;
  }

  void testApplyDiscount_Percent()
  {
    Money fee(300, NUC);
    CPPUNIT_ASSERT(_applier->applyDiscount(fee, *createDiscount(cat19, 10.0)));
    assertMoney(Money(30.0, NUC), fee);
  }

  void testApplyDiscount_Specified()
  {
    Money fee(300, NUC);
    CPPUNIT_ASSERT(!_applier->applyDiscount(fee, *createDiscount(cat19, 0.0, 10.0, USD)));
    assertMoney(Money(300.0, NUC), fee);
  }

  void testApplyDiscount_SpecifiedZero()
  {
    Money fee(300, NUC);
    CPPUNIT_ASSERT(_applier->applyDiscount(fee, *createDiscount(cat19, 0.0, 0.0, USD)));
    assertMoney(Money(0.0, NUC), fee);
  }

  void testApplyDiscount_InfantWithSeat()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::INFANT_WITH_SEAT));
    std::vector<Indicator> tags(1, RefundDiscountApplier::INFANT_WITH_SEAT);
    Money fee(200, NUC);
    DiscountInfo* discount = createDiscount(cat19, 20.0);

    CPPUNIT_ASSERT(_applier->applyDiscount(fee, tags, discount));
    assertMoney(Money(40.0, NUC), fee);
  }

  void testApplyDiscount_Child()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::CHILD));
    std::vector<Indicator> tags(1, RefundDiscountApplier::CHILD);
    Money fee(200, NUC);
    DiscountInfo* discount = createDiscount(cat19, 30.0);

    CPPUNIT_ASSERT(_applier->applyDiscount(fee, tags, discount));
    assertMoney(Money(60.0, NUC), fee);
  }

  void testApplyDiscount_Youth()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::YOUTH));
    std::vector<Indicator> tags(1, RefundDiscountApplier::YOUTH);
    Money fee(200, NUC);
    DiscountInfo* discount = createDiscount(cat22, 15.0);

    CPPUNIT_ASSERT(_applier->applyDiscount(fee, tags, discount));
    assertMoney(Money(30.0, NUC), fee);
  }

  void testApplyDiscount_Senior()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::SENIOR));
    std::vector<Indicator> tags(1, RefundDiscountApplier::SENIOR);
    Money fee(200, NUC);
    DiscountInfo* discount = createDiscount(cat22, 25.0);

    CPPUNIT_ASSERT(_applier->applyDiscount(fee, tags, discount));
    assertMoney(Money(50.0, NUC), fee);
  }

  void testApplyDiscount_ChildOrInfant()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::CHILD_OR_INFANT));
    std::vector<Indicator> tags(1, RefundDiscountApplier::CHILD_OR_INFANT);
    Money fee(200, NUC);
    DiscountInfo* discount = createDiscount(cat19, 0.0, 0.0, USD);

    CPPUNIT_ASSERT(_applier->applyDiscount(fee, tags, discount));
    assertMoney(Money(0.0, NUC), fee);
  }

  void testApplyDiscount_AnyCat22()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::ANY_CAT22));
    std::vector<Indicator> tags(1, RefundDiscountApplier::ANY_CAT22);
    Money fee(200, NUC);
    DiscountInfo* discount = createDiscount(cat22, 50.0);

    CPPUNIT_ASSERT(_applier->applyDiscount(fee, tags, discount));
    assertMoney(Money(100.0, NUC), fee);
  }

  void testApplyDiscount_AllTags_NotDiscounted()
  {
    _paxType->paxType() = "FNF";
    std::vector<Indicator> tags;
    tags += RefundDiscountApplier::INFANT_WITHOUT_SEAT, RefundDiscountApplier::INFANT_WITH_SEAT,
        RefundDiscountApplier::CHILD, RefundDiscountApplier::YOUTH, RefundDiscountApplier::SENIOR,
        RefundDiscountApplier::CHILD_OR_INFANT, RefundDiscountApplier::ANY_CAT22;

    Money fee(200, NUC);
    DiscountInfo* discount = createDiscount(cat19, 50.0);

    CPPUNIT_ASSERT(!_applier->applyDiscount(fee, tags, discount));
    assertMoney(Money(200.0, NUC), fee);
  }

  void testPaxTypeDiscountStatus_isInfantWithoutSeat()
  {
    RefundDiscountApplier::PaxTypeDiscountStatus paxStatus(
        createPaxType(RefundDiscountApplier::INFANT_WITHOUT_SEAT));
    CPPUNIT_ASSERT(paxStatus.isInfantWithoutSeat());
    CPPUNIT_ASSERT(!paxStatus.isInfantWithSeat());
    CPPUNIT_ASSERT(!paxStatus.isChild());
    CPPUNIT_ASSERT(!paxStatus.isYouth());
    CPPUNIT_ASSERT(!paxStatus.isSenior());
    CPPUNIT_ASSERT(paxStatus.isChildOrInfant());
  }

  void testPaxTypeDiscountStatus_isInfantWithSeat()
  {
    RefundDiscountApplier::PaxTypeDiscountStatus paxStatus(
        createPaxType(RefundDiscountApplier::INFANT_WITH_SEAT));
    CPPUNIT_ASSERT(!paxStatus.isInfantWithoutSeat());
    CPPUNIT_ASSERT(paxStatus.isInfantWithSeat());
    CPPUNIT_ASSERT(!paxStatus.isChild());
    CPPUNIT_ASSERT(!paxStatus.isYouth());
    CPPUNIT_ASSERT(!paxStatus.isSenior());
    CPPUNIT_ASSERT(paxStatus.isChildOrInfant());
  }

  void testPaxTypeDiscountStatus_isChild()
  {
    RefundDiscountApplier::PaxTypeDiscountStatus paxStatus(
        createPaxType(RefundDiscountApplier::CHILD));
    CPPUNIT_ASSERT(!paxStatus.isInfantWithoutSeat());
    CPPUNIT_ASSERT(!paxStatus.isInfantWithSeat());
    CPPUNIT_ASSERT(paxStatus.isChild());
    CPPUNIT_ASSERT(!paxStatus.isYouth());
    CPPUNIT_ASSERT(!paxStatus.isSenior());
    CPPUNIT_ASSERT(paxStatus.isChildOrInfant());
  }

  void testPaxTypeDiscountStatus_isYouth()
  {
    RefundDiscountApplier::PaxTypeDiscountStatus paxStatus(
        createPaxType(RefundDiscountApplier::YOUTH));
    CPPUNIT_ASSERT(!paxStatus.isInfantWithoutSeat());
    CPPUNIT_ASSERT(!paxStatus.isInfantWithSeat());
    CPPUNIT_ASSERT(!paxStatus.isChild());
    CPPUNIT_ASSERT(paxStatus.isYouth());
    CPPUNIT_ASSERT(!paxStatus.isSenior());
    CPPUNIT_ASSERT(!paxStatus.isChildOrInfant());
  }

  void testPaxTypeDiscountStatus_isSenior()
  {
    RefundDiscountApplier::PaxTypeDiscountStatus paxStatus(
        createPaxType(RefundDiscountApplier::SENIOR));
    CPPUNIT_ASSERT(!paxStatus.isInfantWithoutSeat());
    CPPUNIT_ASSERT(!paxStatus.isInfantWithSeat());
    CPPUNIT_ASSERT(!paxStatus.isChild());
    CPPUNIT_ASSERT(!paxStatus.isYouth());
    CPPUNIT_ASSERT(paxStatus.isSenior());
    CPPUNIT_ASSERT(!paxStatus.isChildOrInfant());
  }

  void testPaxTypeDiscountStatus_isChildOrInfant()
  {
    RefundDiscountApplier::PaxTypeDiscountStatus paxStatus(
        createPaxType(RefundDiscountApplier::CHILD_OR_INFANT));
    CPPUNIT_ASSERT(!paxStatus.isInfantWithoutSeat());
    CPPUNIT_ASSERT(!paxStatus.isInfantWithSeat());
    CPPUNIT_ASSERT(!paxStatus.isChild());
    CPPUNIT_ASSERT(!paxStatus.isYouth());
    CPPUNIT_ASSERT(!paxStatus.isSenior());
    CPPUNIT_ASSERT(paxStatus.isChildOrInfant());
  }

  VoluntaryRefundsInfo& createRecord3(Indicator tag1 = ' ',
                                      Indicator tag2 = ' ',
                                      Indicator tag3 = ' ',
                                      Indicator tag4 = ' ')
  {
    VoluntaryRefundsInfo& rec3 = *_memH.insert(new VoluntaryRefundsInfo);
    rec3.discountTag1() = tag1;
    rec3.discountTag2() = tag2;
    rec3.discountTag3() = tag3;
    rec3.discountTag4() = tag4;
    return rec3;
  }

  void testApply_DiscountTagsEmpty()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::INFANT_WITHOUT_SEAT));

    Money fee(100.0, USD);
    CPPUNIT_ASSERT(!_applier->apply(fee, createRecord3()));
    assertMoney(Money(100.0, USD), fee);
  }

  void testApply_DiscountTagsHasInfantWithoutSeat()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::INFANT_WITHOUT_SEAT));

    Money fee(100.0, USD);
    VoluntaryRefundsInfo& rec3 = createRecord3(RefundDiscountApplier::INFANT_WITHOUT_SEAT,
                                               RefundDiscountApplier::CHILD_OR_INFANT);
    CPPUNIT_ASSERT(_applier->apply(fee, rec3));
    assertMoney(Money(0.0, USD), fee);
  }

  void testApply_DiscountTagsHasOther()
  {
    _applier->_discountStatus.assign(createPaxType(RefundDiscountApplier::INFANT_WITHOUT_SEAT));

    Money fee(100.0, USD);
    VoluntaryRefundsInfo& rec3 = createRecord3(RefundDiscountApplier::CHILD_OR_INFANT);
    CPPUNIT_ASSERT(!_applier->apply(fee, rec3));
    assertMoney(Money(100.0, USD), fee);
  }

  void testGetDiscountTypes_DiscountTagsEmpty()
  {
    VoluntaryRefundsInfo& rec3 = createRecord3();
    std::vector<Indicator> expect;

    CPPUNIT_ASSERT(expect == _applier->getDiscountTypes(rec3));
  }

  void testGetDiscountTypes_ValidValues()
  {
    VoluntaryRefundsInfo& rec3 = createRecord3(RefundDiscountApplier::CHILD,
                                               RefundDiscountApplier::YOUTH,
                                               RefundDiscountApplier::SENIOR,
                                               RefundDiscountApplier::CHILD_OR_INFANT);

    std::vector<Indicator> expect;
    expect += RefundDiscountApplier::CHILD, RefundDiscountApplier::YOUTH,
        RefundDiscountApplier::SENIOR, RefundDiscountApplier::CHILD_OR_INFANT;

    CPPUNIT_ASSERT(expect == _applier->getDiscountTypes(rec3));
  }

  void testGetDiscountTypes_InvalidValues()
  {
    VoluntaryRefundsInfo& rec3 = createRecord3('K', 'O', 'T', 'A');
    std::vector<Indicator> expect;

    CPPUNIT_ASSERT(expect == _applier->getDiscountTypes(rec3));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RefundDiscountApplierTest);

class DiscountAppliedDiscountValue1 : public testing::Test
{
public:
  PaxTypeInfo paxTypeInfo;
  std::vector<Indicator> tags { '1' };
  DiscountInfo discount;
  PaxType paxType;

  void SetUp() override
  {
    paxTypeInfo.infantInd() = 'Y';
    paxTypeInfo.initPsgType();
    discount.category() = cat19;
    discount.discPercent() = 10;
    paxType.paxTypeInfo() = &paxTypeInfo;
  }
};

TEST_F(DiscountAppliedDiscountValue1, IsCat19DiscountAppliedIfInfantWithSeat)
{
  paxTypeInfo.numberSeatsReq() = 3;
  paxType.paxType() = "INS";
  RefundDiscountApplier discountApplier(paxType);
  Money fee(200, NUC);

  discountApplier.applyDiscount(fee, tags, &discount);
  ASSERT_THAT(fee.value(), testing::DoubleEq(20.0));
}

TEST_F(DiscountAppliedDiscountValue1, IsCat19DiscountAppliedIfInfantWithoutSeat)
{
  paxType.paxType() = "INF";
  RefundDiscountApplier discountApplier(paxType);
  Money fee(200, NUC);

  discountApplier.applyDiscount(fee, tags, &discount);
  ASSERT_THAT(fee.value(), testing::DoubleEq(20.0));
}
} // end of tse namespace
