//----------------------------------------------------------------------------
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "DataModel/Fare.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleItem.h"
#include "Rules/RuleItemCaller.h"
#include "Rules/TransfersInfoWrapper.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>
#include <gmock/gmock.h>

using namespace boost::assign;
using namespace ::testing;

namespace tse
{
namespace
{
class DataHandleGMock : public DataHandleMock
{
public:
  MOCK_METHOD3(getRuleItemInfo,
               const RuleItemInfo*(const CategoryRuleInfo*,
                                   const CategoryRuleItemInfo*,
                                   const DateTime&));
};

PaxTypeFare dummyPtf{};
bool dummyBool{};
RuleItem dummyRI{};

class RuleItemCallerGMock : public RuleItemCaller
{
public:
  RuleItemCallerGMock(PricingTrx* trx,
                      const CategoryRuleInfo& r2,
                      const std::vector<CategoryRuleItemInfo>& r2Segments)
    : RuleItemCaller(*trx, r2, r2Segments, dummyPtf, dummyBool, dummyRI, false)
  {}

  Record3ReturnTypes operator()(CategoryRuleItemInfo*) const override { return PASS; }
  Record3ReturnTypes operator()(CategoryRuleItemInfo*, bool) const override { return PASS; }
  Record3ReturnTypes operator()(CategoryRuleItemInfo*, const RuleItemInfo*) const override
  {
    return PASS;
  }
  Record3ReturnTypes
  isR8DirectionPass(Indicator directionality, bool isR8LocationSwapped) const override
  {
    return PASS;
  }

  MOCK_CONST_METHOD2(isDirectionPass,
       Record3ReturnTypes(const CategoryRuleItemInfo* cfrItem, bool isLocationSwapped));
  MOCK_CONST_METHOD2(validateT994DateOverride,
     Record3ReturnTypes(const RuleItemInfo* r3, uint32_t catNo));
};
}

class TransfersInfoWrapperTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TransfersInfoWrapperTest);

  CPPUNIT_TEST(testSelectApplCharge_noCurrency);
  CPPUNIT_TEST(testSelectApplCharge_firstCurrency);
  CPPUNIT_TEST(testSelectApplCharge_secondCurrency);

  CPPUNIT_TEST(testSelectApplCharge_firstCurrency_fareCurrency);
  CPPUNIT_TEST(testSelectApplCharge_secondCurrency_fareCurrency);

  CPPUNIT_TEST(testSelectApplCharge_bothCurrencies_noFareCurrency_allEqual);
  CPPUNIT_TEST(testSelectApplCharge_bothCurrencies_noFareCurrency_selectionFirstCurrencyFirstAmt);
  CPPUNIT_TEST(testSelectApplCharge_bothCurrencies_noFareCurrency_selectionSecondCurrencyFirstAmt);
  CPPUNIT_TEST(testSelectApplCharge_bothCurrencies_noFareCurrency_selectionFirstCurrencySecondAmt);
  CPPUNIT_TEST(testSelectApplCharge_bothCurrencies_noFareCurrency_selectionSecondCurrencySecondAmt);

  CPPUNIT_TEST(testChargeForPaxType_child_zeroFareAmount);
  CPPUNIT_TEST(testChargeForPaxType_infant_zeroFareAmount);
  CPPUNIT_TEST(testChargeForPaxType_adult_zeroFareAmount);

  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxAny);
  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxChild);
  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxAdultChild);
  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxAdultChildDisc);
  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxAdult);
  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxAdultChildDiscInfantDisc);
  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxAdultChildInfantFree);
  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxAdultChildDiscInfantFree);
  CPPUNIT_TEST(testChargeForPaxType_child_chargePaxInfant);

  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxAny);
  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxChild);
  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxAdultChild);
  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxAdultChildDisc);
  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxAdult);
  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxAdultChildDiscInfantDisc);
  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxAdultChildInfantFree);
  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxAdultChildDiscInfantFree);
  CPPUNIT_TEST(testChargeForPaxType_infant_chargePaxInfant);

  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxAny);
  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxChild);
  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxAdultChild);
  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxAdultChildDisc);
  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxAdult);
  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxAdultChildDiscInfantDisc);
  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxAdultChildInfantFree);
  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxAdultChildDiscInfantFree);
  CPPUNIT_TEST(testChargeForPaxType_adult_chargePaxInfant);

  CPPUNIT_TEST(testDetectRelationIndicators_And);
  CPPUNIT_TEST(testDetectRelationIndicators_Or);

  CPPUNIT_TEST(testSetCurrentTrInfo_SingleR3);
  CPPUNIT_TEST(testSetCurrentTrInfo_ThenOr);
  CPPUNIT_TEST(testSetCurrentTrInfo_ThenAnd);
  CPPUNIT_TEST(testSetCurrentTrInfo_ThenAnd_FailDirectionality);
  CPPUNIT_TEST(testSetCurrentTrInfo_ThenAnd_FailDateOverride);

  CPPUNIT_TEST_SUITE_END();

protected:
  static const PaxTypeCode PAX_ADULT, PAX_CHILD, PAX_INFANT;
  static const Indicator CHARGE_PAX_UNIMPORTANT;

  TestMemHandle _memH;
  TransfersInfoWrapper* _transfers;
  CategoryRuleInfo* _crInfo;
  PricingTrx* _trx;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _memH.create<TestDataHandle>(_memH);
    _memH.create<RootLoggerGetOff>();

    _trx = _memH.create<PricingTrx>();
    _trx->setRequest(_memH.create<PricingRequest>());

    _transfers = _memH.create<TransfersInfoWrapper>();

    _crInfo = _memH.create<CategoryRuleInfo>();
    _crInfo->vendorCode() = "ATP";
    _crInfo->carrierCode() = "AA";
    _crInfo->tariffNumber() = 1;
    _crInfo->ruleNumber() = "0001";
    _crInfo->sequenceNumber() = 123123;
    _transfers->crInfo(_crInfo);
  }

  void tearDown() { _memH.clear(); }

  PaxTypeFare& createFare(const MoneyAmount& amt)
  {
    FareInfo* fi = _memH.create<FareInfo>();
    fi->currency() = USD;
    fi->fareAmount() = amt;

    Fare* f = _memH.create<Fare>();
    f->setFareInfo(fi);

    PaxTypeFare* fare = _memH.create<PaxTypeFare>();
    fare->setFare(f);
    return *fare;
  }

  void initCrInfo(const std::vector<uint32_t>& relIndicators)
  {
    for (uint32_t relInd : relIndicators)
    {
      if (relInd == CategoryRuleItemInfo::THEN || _crInfo->categoryRuleItemInfoSet().empty())
        _crInfo->addItemInfoSetNosync(new CategoryRuleItemInfoSet);
      CategoryRuleItemInfoSet& set = *_crInfo->categoryRuleItemInfoSet().back();
      CategoryRuleItemInfo crii;
      crii.setRelationalInd(static_cast<CategoryRuleItemInfo::LogicalOperators>(relInd));
      set.push_back(crii);
    }
  }

  struct Currency
  {
    Currency(const CurrencyCode& c = "", int16_t d = -1, const CurrencyFactor& f = 0.0)
      : code(c), noDec(d), factor(f)
    {
    }

    CurrencyCode code;
    int16_t noDec;
    CurrencyFactor factor;
  };

  static const Currency CURRENCY_NONE, CURRENCY_USD, CURRENCY_PLN, CURRENCY_CAD, CURRENCY_NUC;

  struct Charge
  {
    Charge(const MoneyAmount& nucAmt1 = 0.0,
           const MoneyAmount& nucAmt2 = 0.0,
           const Currency& curr = CURRENCY_NONE)
      : first(curr.factor * nucAmt1), second(curr.factor * nucAmt2), currency(curr)
    {
    }

    Charge getNUC() const
    {
      return Charge(first / currency.factor, second / currency.factor, CURRENCY_NUC);
    }

    bool operator==(const Charge& rhs) const
    {
      return (first - rhs.first) < EPSILON && (second - rhs.second) < EPSILON &&
             currency.code == rhs.currency.code && currency.noDec == rhs.currency.noDec;
    }

    MoneyAmount first, second;
    Currency currency;
  };

  static const Charge CHARGE_DEFAULT;

  void populateInfo(const Charge& charge1, const Charge& charge2)
  {
    TransfersInfo1* info = _memH.create<TransfersInfo1>();
    _transfers->setCurrentTrInfo(info);

    info->transfersChargeAppl() = RuleConst::CHARGE_PAX_ANY;
    info->charge1Cur1Amt() = charge1.first;
    info->charge2Cur1Amt() = charge1.second;
    info->charge1Cur2Amt() = charge2.first;
    info->charge2Cur2Amt() = charge2.second;
    info->cur1() = charge1.currency.code;
    info->noDec1() = charge1.currency.noDec;
    info->cur2() = charge2.currency.code;
    info->noDec2() = charge2.currency.noDec;
  }

  typedef Charge Charges[2];

  void populateInfo(const Charges& charge) { populateInfo(charge[0], charge[1]); }

  void assertSelectApplCharge(const Charge& expect)
  {
    Charge result(CHARGE_DEFAULT);
    CurrencyCode code = "XXX";
    int16_t noDec = -1;

    CPPUNIT_ASSERT(_transfers->selectApplCharge(*_trx,
                                                result.first,
                                                result.currency.code,
                                                result.currency.noDec,
                                                result.second,
                                                code,
                                                noDec,
                                                createFare(100.0),
                                                PAX_ADULT));
    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(result.currency.code, code);
    CPPUNIT_ASSERT_EQUAL(result.currency.noDec, noDec);
  }

  void testSelectApplCharge_noCurrency()
  {
    Charges charge = { Charge(), Charge() };
    populateInfo(charge);
    assertSelectApplCharge(charge[0]);
  }

  void testSelectApplCharge_firstCurrency()
  {
    Charges charge = { Charge(1.0, 3.0, CURRENCY_USD), Charge() };
    populateInfo(charge);
    assertSelectApplCharge(charge[0]);
  }

  void testSelectApplCharge_secondCurrency()
  {
    Charges charge = { Charge(), Charge(1.0, 3.0, CURRENCY_USD) };
    populateInfo(charge);
    assertSelectApplCharge(charge[1]);
  }

  void testSelectApplCharge_firstCurrency_fareCurrency()
  {
    Charges charge = { Charge(1.0, 3.0, CURRENCY_USD), Charge(2.0, 4.0, CURRENCY_PLN) };
    populateInfo(charge);
    assertSelectApplCharge(charge[0]);
  }

  void testSelectApplCharge_secondCurrency_fareCurrency()
  {
    Charges charge = { Charge(2.0, 4.0, CURRENCY_PLN), Charge(1.0, 0.5, CURRENCY_USD) };
    populateInfo(charge);
    assertSelectApplCharge(charge[1]);
  }

  void testSelectApplCharge_bothCurrencies_noFareCurrency_allEqual()
  {
    Charges charge = { Charge(2.0, 1.0, CURRENCY_PLN), Charge(2.0, 1.0, CURRENCY_CAD) };
    populateInfo(charge);
    assertSelectApplCharge(charge[0].getNUC());
  }

  void testSelectApplCharge_bothCurrencies_noFareCurrency_selectionFirstCurrencyFirstAmt()
  {
    Charges charge = { Charge(1.0, 1.0, CURRENCY_PLN), Charge(2.0, 1.0, CURRENCY_CAD) };
    populateInfo(charge);
    assertSelectApplCharge(charge[0].getNUC());
  }

  void testSelectApplCharge_bothCurrencies_noFareCurrency_selectionSecondCurrencyFirstAmt()
  {
    Charges charge = { Charge(2.0, 1.0, CURRENCY_PLN), Charge(1.0, 1.0, CURRENCY_CAD) };
    populateInfo(charge);
    assertSelectApplCharge(charge[1].getNUC());
  }

  void testSelectApplCharge_bothCurrencies_noFareCurrency_selectionFirstCurrencySecondAmt()
  {
    Charges charge = { Charge(2.0, 1.0, CURRENCY_PLN), Charge(2.0, 3.0, CURRENCY_CAD) };
    populateInfo(charge);
    assertSelectApplCharge(charge[0].getNUC());
  }

  void testSelectApplCharge_bothCurrencies_noFareCurrency_selectionSecondCurrencySecondAmt()
  {
    Charges charge = { Charge(2.0, 3.0, CURRENCY_PLN), Charge(2.0, 1.0, CURRENCY_CAD) };
    populateInfo(charge);
    assertSelectApplCharge(charge[1].getNUC());
  }

  void assertChargeForPaxType(const Indicator& appl,
                              const MoneyAmount& amt,
                              const PaxTypeCode& pax,
                              bool expect)
  {
    CPPUNIT_ASSERT_EQUAL(expect, _transfers->chargeForPaxType(*_trx, appl, createFare(amt), pax));
  }

  void assertChargeForPaxTypeChild(const Indicator& appl, bool expect)
  {
    assertChargeForPaxType(appl, 10.0, PAX_CHILD, expect);
  }

  void assertChargeForPaxTypeInfant(const Indicator& appl, bool expect)
  {
    assertChargeForPaxType(appl, 10.0, PAX_INFANT, expect);
  }

  void assertChargeForPaxTypeAdult(const Indicator& appl, bool expect)
  {
    assertChargeForPaxType(appl, 10.0, PAX_ADULT, expect);
  }

  void testChargeForPaxType_child_zeroFareAmount()
  {
    assertChargeForPaxType(CHARGE_PAX_UNIMPORTANT, 0.0, PAX_CHILD, true);
  }

  void testChargeForPaxType_infant_zeroFareAmount()
  {
    assertChargeForPaxType(CHARGE_PAX_UNIMPORTANT, 0.0, PAX_INFANT, false);
  }

  void testChargeForPaxType_adult_zeroFareAmount()
  {
    assertChargeForPaxType(CHARGE_PAX_UNIMPORTANT, 0.0, PAX_ADULT, true);
  }

  void testChargeForPaxType_child_chargePaxAny()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_ANY, true);
  }

  void testChargeForPaxType_child_chargePaxChild()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_CHILD, true);
  }

  void testChargeForPaxType_child_chargePaxAdultChild()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_ADULT_CHILD, true);
  }

  void testChargeForPaxType_child_chargePaxAdultChildDisc()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC, true);
  }

  void testChargeForPaxType_child_chargePaxAdult()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_ADULT, false);
  }

  void testChargeForPaxType_child_chargePaxAdultChildDiscInfantDisc()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC, true);
  }

  void testChargeForPaxType_child_chargePaxAdultChildInfantFree()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_ADULT_CHILD_INFANT_FREE, true);
  }

  void testChargeForPaxType_child_chargePaxAdultChildDiscInfantFree()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE, true);
  }

  void testChargeForPaxType_child_chargePaxInfant()
  {
    assertChargeForPaxTypeChild(RuleConst::CHARGE_PAX_INFANT, false);
  }

  void testChargeForPaxType_infant_chargePaxAny()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_ANY, true);
  }

  void testChargeForPaxType_infant_chargePaxChild()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_CHILD, false);
  }

  void testChargeForPaxType_infant_chargePaxAdultChild()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_ADULT_CHILD, false);
  }

  void testChargeForPaxType_infant_chargePaxAdultChildDisc()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC, false);
  }

  void testChargeForPaxType_infant_chargePaxAdult()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_ADULT, false);
  }

  void testChargeForPaxType_infant_chargePaxAdultChildDiscInfantDisc()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC, true);
  }

  void testChargeForPaxType_infant_chargePaxAdultChildInfantFree()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_ADULT_CHILD_INFANT_FREE, false);
  }

  void testChargeForPaxType_infant_chargePaxAdultChildDiscInfantFree()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE, false);
  }

  void testChargeForPaxType_infant_chargePaxInfant()
  {
    assertChargeForPaxTypeInfant(RuleConst::CHARGE_PAX_INFANT, true);
  }

  void testChargeForPaxType_adult_chargePaxAny()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_ANY, true);
  }

  void testChargeForPaxType_adult_chargePaxChild()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_CHILD, false);
  }

  void testChargeForPaxType_adult_chargePaxAdultChild()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_ADULT_CHILD, true);
  }

  void testChargeForPaxType_adult_chargePaxAdultChildDisc()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC, true);
  }

  void testChargeForPaxType_adult_chargePaxAdult()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_ADULT, true);
  }

  void testChargeForPaxType_adult_chargePaxAdultChildDiscInfantDisc()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC, true);
  }

  void testChargeForPaxType_adult_chargePaxAdultChildInfantFree()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_ADULT_CHILD_INFANT_FREE, true);
  }

  void testChargeForPaxType_adult_chargePaxAdultChildDiscInfantFree()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE, true);
  }

  void testChargeForPaxType_adult_chargePaxInfant()
  {
    assertChargeForPaxTypeAdult(RuleConst::CHARGE_PAX_INFANT, false);
  }

  void testDetectRelationIndicators_And()
  {
    typedef CategoryRuleItemInfo CRII;

    std::vector<uint32_t> relIndicators;
    relIndicators += CRII::THEN, CRII::IF, CRII::THEN, CRII::AND;
    initCrInfo(relIndicators);

    _transfers->detectRelationIndicators();
    CPPUNIT_ASSERT(_transfers->isRelationAndExists());
    CPPUNIT_ASSERT(!_transfers->isRelationOrExists());
  }

  void testDetectRelationIndicators_Or()
  {
    typedef CategoryRuleItemInfo CRII;

    std::vector<uint32_t> relIndicators;
    relIndicators += CRII::THEN, CRII::IF, CRII::AND, CRII::THEN, CRII::OR;
    initCrInfo(relIndicators);

    _transfers->detectRelationIndicators();
    CPPUNIT_ASSERT(!_transfers->isRelationAndExists());
    CPPUNIT_ASSERT(_transfers->isRelationOrExists());
  }

  // setCurrentTrInfo tests

  CategoryRuleItemInfo createR2s(const CategoryRuleItemInfo::LogicalOperators relation)
  {
    CategoryRuleItemInfo r2s;
    r2s.setRelationalInd(relation);
    r2s.setItemcat(9);
    return r2s;
  }

  TransfersInfo1* createR3(const char* max, uint32_t dateOverride = 0)
  {
    TransfersInfo1* r3 = _memH(new TransfersInfo1());
    r3->noTransfersMax() = max;
    r3->overrideDateTblItemNo() = dateOverride;
    return r3;
  }

  void testSetCurrentTrInfo_SingleR3()
  {
    using R2s = CategoryRuleItemInfo;
    const std::vector<CategoryRuleItemInfo> r2Segments = {createR2s(R2s::THEN)};
    const TransfersInfo1* r3s[] = {createR3("10")};
    const NiceMock<RuleItemCallerGMock> caller(_trx, *_crInfo, r2Segments);
    NiceMock<DataHandleGMock> dh;

    ON_CALL(caller, isDirectionPass(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(caller, validateT994DateOverride(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(dh, getRuleItemInfo(_, _, _)).WillByDefault(Return(r3s[0]));

    _transfers->setCurrentTrInfo(r3s[0], caller, false);

    CPPUNIT_ASSERT_EQUAL(uint16_t(10), _transfers->noTransfersMax());
  }

  void testSetCurrentTrInfo_ThenOr()
  {
    using R2s = CategoryRuleItemInfo;
    const std::vector<CategoryRuleItemInfo> r2Segments = {createR2s(R2s::THEN), createR2s(R2s::OR)};
    const TransfersInfo1* r3s[] = {createR3("1"), createR3("2")};
    const NiceMock<RuleItemCallerGMock> caller(_trx, *_crInfo, r2Segments);
    NiceMock<DataHandleGMock> dh;

    ON_CALL(caller, isDirectionPass(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(caller, validateT994DateOverride(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(dh, getRuleItemInfo(_, &r2Segments[0], _)).WillByDefault(Return(r3s[0]));
    ON_CALL(dh, getRuleItemInfo(_, &r2Segments[1], _)).WillByDefault(Return(r3s[1]));

    _transfers->setCurrentTrInfo(r3s[0], caller, false);

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), _transfers->noTransfersMax());
  }

  void testSetCurrentTrInfo_ThenAnd()
  {
    using R2s = CategoryRuleItemInfo;
    const std::vector<CategoryRuleItemInfo> r2Segments = {createR2s(R2s::THEN), createR2s(R2s::AND)};
    const TransfersInfo1* r3s[] = {createR3("1"), createR3("2")};
    const NiceMock<RuleItemCallerGMock> caller(_trx, *_crInfo, r2Segments);
    NiceMock<DataHandleGMock> dh;

    ON_CALL(caller, isDirectionPass(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(caller, validateT994DateOverride(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(dh, getRuleItemInfo(_, &r2Segments[0], _)).WillByDefault(Return(r3s[0]));
    ON_CALL(dh, getRuleItemInfo(_, &r2Segments[1], _)).WillByDefault(Return(r3s[1]));

    _transfers->setCurrentTrInfo(r3s[0], caller, false);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), _transfers->noTransfersMax());
  }

  void testSetCurrentTrInfo_ThenAnd_FailDirectionality()
  {
    using R2s = CategoryRuleItemInfo;
    const std::vector<CategoryRuleItemInfo> r2Segments = {createR2s(R2s::THEN), createR2s(R2s::AND)};
    const TransfersInfo1* r3s[] = {createR3("1"), createR3("2")};
    const NiceMock<RuleItemCallerGMock> caller(_trx, *_crInfo, r2Segments);
    NiceMock<DataHandleGMock> dh;

    ON_CALL(caller, isDirectionPass(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(caller, isDirectionPass(&r2Segments[1], _)).WillByDefault(Return(tse::FAIL));
    ON_CALL(caller, validateT994DateOverride(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(dh, getRuleItemInfo(_, &r2Segments[0], _)).WillByDefault(Return(r3s[0]));
    ON_CALL(dh, getRuleItemInfo(_, &r2Segments[1], _)).WillByDefault(Return(r3s[1]));

    _transfers->setCurrentTrInfo(r3s[0], caller, false);

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), _transfers->noTransfersMax());
  }

  void testSetCurrentTrInfo_ThenAnd_FailDateOverride()
  {
    using R2s = CategoryRuleItemInfo;
    const std::vector<CategoryRuleItemInfo> r2Segments = {createR2s(R2s::THEN), createR2s(R2s::AND)};
    const TransfersInfo1* r3s[] = {createR3("1", 100), createR3("2")};
    const NiceMock<RuleItemCallerGMock> caller(_trx, *_crInfo, r2Segments);
    NiceMock<DataHandleGMock> dh;

    ON_CALL(caller, isDirectionPass(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(caller, validateT994DateOverride(_, _)).WillByDefault(Return(tse::PASS));
    ON_CALL(caller, validateT994DateOverride(r3s[0], _)).WillByDefault(Return(tse::SKIP));
    ON_CALL(dh, getRuleItemInfo(_, &r2Segments[0], _)).WillByDefault(Return(r3s[0]));
    ON_CALL(dh, getRuleItemInfo(_, &r2Segments[1], _)).WillByDefault(Return(r3s[1]));

    _transfers->setCurrentTrInfo(r3s[0], caller, false);

    CPPUNIT_ASSERT_EQUAL(uint16_t(2), _transfers->noTransfersMax());
  }

protected:
  class TestDataHandle : public DataHandleMock
  {
  public:
    explicit TestDataHandle(TestMemHandle& memH) : _memH(memH) {}

    NUCInfo*
    getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
    {
      if (currency == CURRENCY_PLN.code)
        return getNUCInfo(CURRENCY_PLN.factor);

      if (currency == CURRENCY_CAD.code)
        return getNUCInfo(CURRENCY_CAD.factor);

      return 0;
    }

  private:
    NUCInfo* getNUCInfo(const CurrencyFactor& f)
    {
      NUCInfo* info = _memH.create<NUCInfo>();
      info->_nucFactor = f;
      return info;
    }

    TestMemHandle& _memH;
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(TransfersInfoWrapperTest);

const TransfersInfoWrapperTest::Currency
TransfersInfoWrapperTest::CURRENCY_NONE("", 0, 0.0),
    TransfersInfoWrapperTest::CURRENCY_USD(USD, 2, 1.0),
    TransfersInfoWrapperTest::CURRENCY_PLN("PLN", 2, 4.0),
    TransfersInfoWrapperTest::CURRENCY_CAD("CAD", 2, 2.0),
    TransfersInfoWrapperTest::CURRENCY_NUC(NUC, 2, 1.0);

const TransfersInfoWrapperTest::Charge
TransfersInfoWrapperTest::CHARGE_DEFAULT(-10.0,
                                         -10.0,
                                         TransfersInfoWrapperTest::Currency("XXX", 0, 10.0));

const PaxTypeCode
TransfersInfoWrapperTest::PAX_ADULT(ADULT),
    TransfersInfoWrapperTest::PAX_CHILD(CHILD), TransfersInfoWrapperTest::PAX_INFANT(INFANT);

const Indicator TransfersInfoWrapperTest::CHARGE_PAX_UNIMPORTANT = 'X';

std::ostream& operator<<(std::ostream& os, const TransfersInfoWrapperTest::Charge& charge)
{
  return os << charge.currency.code << std::fixed << std::setprecision(charge.currency.noDec) << "("
            << charge.first << "," << charge.second << ")";
}

} // tse
