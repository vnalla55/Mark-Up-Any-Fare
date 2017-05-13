#include "DataModel/ExcItin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPenalty.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include <boost/assign/std/vector.hpp>

#include "gmock/gmock.h"

namespace tse
{
using namespace boost::assign;

class MockTrx : public RefundPricingTrx
{
public:
  virtual Money convertCurrency(const Money& source, const CurrencyCode& targetCurr) const
  {
    if (source.code() == "PLN" && targetCurr == NUC)
      return Money(source.value() * 0.5, targetCurr);

    if (source.code() == NUC && targetCurr == "PLN")
      return Money(source.value() * 2, targetCurr);

    return source;
  }
};

class RefundPermutationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RefundPermutationTest);

  CPPUNIT_TEST(testGetRepriceIndicator_HistoricalTicketBasedOnly);
  CPPUNIT_TEST(testGetRepriceIndicator_HistoricalTravelCommenBasedOnly);
  CPPUNIT_TEST(testGetRepriceIndicator_BlankOnly);
  CPPUNIT_TEST(testGetRepriceIndicator_MixedWithHistoricalTicketBased);
  CPPUNIT_TEST(testGetRepriceIndicator_MixedWithoutHistoricalTicketBased);

  CPPUNIT_TEST(testFind_Pass);
  CPPUNIT_TEST(testFind_Fail);

  CPPUNIT_TEST(testRefundable_singleFeeTrue);
  CPPUNIT_TEST(testRefundable_singleFeeFalse);
  CPPUNIT_TEST(testRefundable_multipleFeeTrue);
  CPPUNIT_TEST(testRefundable_multipleFeeFalse);
  CPPUNIT_TEST(testRefundable_multiplePenaltyTrue);
  CPPUNIT_TEST(testRefundable_multiplePenaltyFalse);

  CPPUNIT_TEST(testRefundable_PUPenalty1FareUsage);
  CPPUNIT_TEST(testRefundable_PUPenalty3FareUsage);
  CPPUNIT_TEST(testRefundable_FCPenalty1FareUsage);
  CPPUNIT_TEST(testRefundable_FCPenalty3FareUsage);

  CPPUNIT_TEST(testFormOfRefundIndValueA);
  CPPUNIT_TEST(testFormOfRefundIndValueV);
  CPPUNIT_TEST(testFormOfRefundIndValueM);
  CPPUNIT_TEST(testFormOfRefundIndValueBLANK);
  CPPUNIT_TEST(testFormOfRefundIndValueS);

  CPPUNIT_TEST_SUITE_END();

  RefundPermutation* _perm;
  TestMemHandle _memH;

public:
  void setUp() { _perm = _memH.insert(new RefundPermutation); }

  void tearDown() { _memH.clear(); }

  template <typename T>
  void populateByteInRecords3(T& (VoluntaryRefundsInfo::*set_method)(), const std::vector<T>& bytes)
  {
    typedef typename std::vector<T>::const_iterator It;
    for (It i = bytes.begin(); i != bytes.end(); ++i)
    {
      VoluntaryRefundsInfo* vri = _memH.insert(new VoluntaryRefundsInfo);
      (vri->*set_method)() = *i;
      RefundProcessInfo* info = _memH.insert(new RefundProcessInfo);
      info->assign(vri, _memH.insert(new PaxTypeFare), 0);
      _perm->processInfos().push_back(info);
    }
  }

  void testGetRepriceIndicator_HistoricalTicketBasedOnly()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::HISTORICAL_TICKET_BASED, RefundPermutation::HISTORICAL_TICKET_BASED,
        RefundPermutation::HISTORICAL_TICKET_BASED;

    populateByteInRecords3(&VoluntaryRefundsInfo::repriceInd, ind);

    _perm->setRepriceIndicator();
    CPPUNIT_ASSERT_EQUAL(RefundPermutation::HISTORICAL_TICKET_BASED, _perm->_repriceInd);
  }

  void testGetRepriceIndicator_HistoricalTravelCommenBasedOnly()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED,
        RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED,
        RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED;

    populateByteInRecords3(&VoluntaryRefundsInfo::repriceInd, ind);

    _perm->setRepriceIndicator();
    CPPUNIT_ASSERT_EQUAL(RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED, _perm->_repriceInd);
  }

  void testGetRepriceIndicator_BlankOnly()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::BLANK, RefundPermutation::BLANK, RefundPermutation::BLANK;

    populateByteInRecords3(&VoluntaryRefundsInfo::repriceInd, ind);

    _perm->setRepriceIndicator();
    CPPUNIT_ASSERT_EQUAL(RefundPermutation::HISTORICAL_TICKET_BASED, _perm->_repriceInd);
  }

  void testGetRepriceIndicator_MixedWithHistoricalTicketBased()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED, RefundPermutation::BLANK,
        RefundPermutation::HISTORICAL_TICKET_BASED;

    populateByteInRecords3(&VoluntaryRefundsInfo::repriceInd, ind);

    _perm->setRepriceIndicator();
    CPPUNIT_ASSERT_EQUAL(RefundPermutation::HISTORICAL_TICKET_BASED, _perm->_repriceInd);
  }

  void testGetRepriceIndicator_MixedWithoutHistoricalTicketBased()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED, RefundPermutation::BLANK,
        RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED;

    populateByteInRecords3(&VoluntaryRefundsInfo::repriceInd, ind);

    _perm->setRepriceIndicator();
    CPPUNIT_ASSERT_EQUAL(RefundPermutation::HISTORICAL_TICKET_BASED, _perm->_repriceInd);
  }

  void testFind_Pass()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::BLANK, RefundPermutation::BLANK, RefundPermutation::BLANK;

    populateByteInRecords3(&VoluntaryRefundsInfo::repriceInd, ind);

    std::vector<RefundProcessInfo*>::const_iterator it = _perm->processInfos().begin() + 1;

    CPPUNIT_ASSERT(_perm->find(&(*it)->paxTypeFare()) == it);
  }

  void testFind_Fail()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::BLANK, RefundPermutation::BLANK, RefundPermutation::BLANK;

    populateByteInRecords3(&VoluntaryRefundsInfo::repriceInd, ind);

    PaxTypeFare ptf;

    CPPUNIT_ASSERT(_perm->find(&ptf) == _perm->processInfos().end());
  }

  RefundPenalty::Fee& createFee(bool nonRefundable = false)
  {
    return createFee(Money(100.0, "PLN"), nonRefundable);
  }

  RefundPenalty::Fee& createFee(const Money& mny, bool nonRefundable = false)
  {
    return *_memH.insert(new RefundPenalty::Fee(mny, false, nonRefundable));
    ;
  }

  typedef std::vector<RefundPenalty::Fee> FeeVec;

  RefundPenalty*
  createPenalty(const FeeVec& feeVec, const RefundPenalty::Scope scope = RefundPenalty::FC)
  {
    RefundPenalty* pen = _memH.insert(new RefundPenalty);
    pen->assign(feeVec, scope);
    return pen;
  }

  void addFareUsage(PricingUnit& pricingUnit, int count = 1)
  {
    for (int i = 0; i < count; ++i)
    {
      FareUsage* fareUsage = _memH.insert(new FareUsage());
      pricingUnit.fareUsage() += fareUsage;
    }
  }

  void testRefundable_singleFeeTrue()
  {
    PricingUnit pu0;
    addFareUsage(pu0);
    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(false)));

    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0;
    CPPUNIT_ASSERT(_perm->refundable(pricingUnits));
  }

  void testRefundable_singleFeeFalse()
  {
    PricingUnit pu0;
    addFareUsage(pu0);
    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(true)));

    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0;
    CPPUNIT_ASSERT(!_perm->refundable(pricingUnits));
  }

  void testRefundable_multipleFeeTrue()
  {
    PricingUnit pu0;
    addFareUsage(pu0, 3);

    FeeVec feeVec;
    feeVec.push_back(createFee(true));
    feeVec.push_back(createFee(false));
    feeVec.push_back(createFee(true));
    _perm->_penaltyFees[&pu0] = createPenalty(feeVec);

    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0;
    CPPUNIT_ASSERT(_perm->refundable(pricingUnits));
  }

  void testRefundable_multipleFeeFalse()
  {
    PricingUnit pu0;
    addFareUsage(pu0, 3);

    FeeVec feeVec;
    feeVec.push_back(createFee(true));
    feeVec.push_back(createFee(true));
    feeVec.push_back(createFee(true));
    _perm->_penaltyFees[&pu0] = createPenalty(feeVec);

    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0;
    CPPUNIT_ASSERT(!_perm->refundable(pricingUnits));
  }

  void testRefundable_multiplePenaltyTrue()
  {
    PricingUnit pu0, pu1, pu2;
    addFareUsage(pu0, 1);
    addFareUsage(pu1, 1);
    addFareUsage(pu2, 1);
    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(true)));
    _perm->_penaltyFees[&pu1] = createPenalty(FeeVec(1, createFee(false)));
    _perm->_penaltyFees[&pu2] = createPenalty(FeeVec(1, createFee(true)));

    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0, &pu1, &pu2;
    CPPUNIT_ASSERT(_perm->refundable(pricingUnits));
  }

  void testRefundable_multiplePenaltyFalse()
  {
    PricingUnit pu0, pu1, pu2;
    addFareUsage(pu0, 1);
    addFareUsage(pu1, 1);
    addFareUsage(pu2, 1);
    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(true)));
    _perm->_penaltyFees[&pu1] = createPenalty(FeeVec(1, createFee(true)));
    _perm->_penaltyFees[&pu2] = createPenalty(FeeVec(1, createFee(true)));

    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0, &pu1, &pu2;
    CPPUNIT_ASSERT(!_perm->refundable(pricingUnits));
  }

  void testRefundable_PUPenalty1FareUsage()
  {
    PricingUnit pu0;
    addFareUsage(pu0, 1);
    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0;

    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(true)), RefundPenalty::PU);
    CPPUNIT_ASSERT(!_perm->refundable(pricingUnits));

    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(false)), RefundPenalty::PU);
    CPPUNIT_ASSERT(_perm->refundable(pricingUnits));
  }

  void testRefundable_PUPenalty3FareUsage()
  {
    PricingUnit pu0;
    addFareUsage(pu0, 3);
    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0;

    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(true)), RefundPenalty::PU);
    CPPUNIT_ASSERT(!_perm->refundable(pricingUnits));

    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(false)), RefundPenalty::PU);
    CPPUNIT_ASSERT(_perm->refundable(pricingUnits));
  }

  void testRefundable_FCPenalty1FareUsage()
  {
    PricingUnit pu0;
    addFareUsage(pu0, 1);
    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0;

    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(true)), RefundPenalty::FC);
    CPPUNIT_ASSERT(!_perm->refundable(pricingUnits));

    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(false)), RefundPenalty::FC);
    CPPUNIT_ASSERT(_perm->refundable(pricingUnits));
  }

  void testRefundable_FCPenalty3FareUsage()
  {
    PricingUnit pu0;
    addFareUsage(pu0, 3);
    RefundPermutation::PricingUnits pricingUnits;
    pricingUnits += &pu0;

    // Specific case.
    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(true)), RefundPenalty::FC);
    CPPUNIT_ASSERT(_perm->refundable(pricingUnits));

    _perm->_penaltyFees[&pu0] = createPenalty(FeeVec(1, createFee(false)), RefundPenalty::FC);
    CPPUNIT_ASSERT(_perm->refundable(pricingUnits));
  }

  RefundPricingTrx* createTrx()
  {
    RefundPricingTrx* trx = _memH.insert(new MockTrx);
    trx->exchangeItin().push_back(_memH.insert(new ExcItin));
    trx->exchangeItin().front()->calculationCurrency() = NUC;
    PaxType* pax = _memH.insert(new PaxType);
    pax->paxType() = "ADT";
    trx->exchangePaxType() = pax;

    return trx;
  }

  void testFormOfRefundIndValueA()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::ANY_FORM_OF_PAYMENT, RefundPermutation::ANY_FORM_OF_PAYMENT;
    populateByteInRecords3(&VoluntaryRefundsInfo::formOfRefund, ind);

    CPPUNIT_ASSERT_EQUAL(static_cast<Indicator>(RefundPermutation::ANY_FORM_OF_PAYMENT),
                         _perm->formOfRefundInd());
  }

  void testFormOfRefundIndValueV()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::ORIGINAL_FOP, RefundPermutation::MCO, RefundPermutation::VOUCHER,
        RefundPermutation::ANY_FORM_OF_PAYMENT;
    populateByteInRecords3(&VoluntaryRefundsInfo::formOfRefund, ind);

    CPPUNIT_ASSERT_EQUAL(static_cast<Indicator>(RefundPermutation::VOUCHER),
                         _perm->formOfRefundInd());
  }

  void testFormOfRefundIndValueM()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::ANY_FORM_OF_PAYMENT, RefundPermutation::MCO,
        RefundPermutation::ORIGINAL_FOP;
    populateByteInRecords3(&VoluntaryRefundsInfo::formOfRefund, ind);

    CPPUNIT_ASSERT_EQUAL(static_cast<Indicator>(RefundPermutation::MCO), _perm->formOfRefundInd());
  }

  void testFormOfRefundIndValueBLANK()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::ANY_FORM_OF_PAYMENT, RefundPermutation::ORIGINAL_FOP,
        RefundPermutation::ANY_FORM_OF_PAYMENT;
    populateByteInRecords3(&VoluntaryRefundsInfo::formOfRefund, ind);

    CPPUNIT_ASSERT_EQUAL(static_cast<Indicator>(RefundPermutation::ORIGINAL_FOP),
                         _perm->formOfRefundInd());
  }

  void testFormOfRefundIndValueS()
  {
    std::vector<Indicator> ind;
    ind += RefundPermutation::ORIGINAL_FOP, RefundPermutation::MCO, RefundPermutation::SCRIPT,
        RefundPermutation::VOUCHER, RefundPermutation::ANY_FORM_OF_PAYMENT;
    populateByteInRecords3(&VoluntaryRefundsInfo::formOfRefund, ind);

    CPPUNIT_ASSERT_EQUAL(static_cast<Indicator>(RefundPermutation::SCRIPT),
                         _perm->formOfRefundInd());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RefundPermutationTest);

class ATaxRefundable : public testing::Test
{
public:
  TestConfigInitializer cfg;
  RefundPermutation _permutation;
  std::deque<RefundProcessInfo*> processInfos;
  VoluntaryRefundsInfo r3Record;
};

TEST_F(ATaxRefundable, IsNonRefundableIfNoProcessInfos)
{
  ASSERT_FALSE(_permutation.taxRefundable());
}

TEST_F(ATaxRefundable, IsRefundableIfOneProcessingInfoWithTaxNonrefundableIndBlank)
{
  r3Record.taxNonrefundableInd() = RefundPermutation::BLANK;
  RefundProcessInfo processInfo(&r3Record, nullptr, nullptr);
  processInfos.push_back(&processInfo);
  _permutation.assign(1, processInfos);
  ASSERT_TRUE(_permutation.taxRefundable());
}

TEST_F(ATaxRefundable, IsRefundableIfOneProcessingInfoWithNonrefundableIndX)
{
  r3Record.taxNonrefundableInd() = RefundPermutation::TAX_NON_REFUNDABLE;
  RefundProcessInfo processInfo(&r3Record, nullptr, nullptr);
  processInfos.push_back(&processInfo);
  _permutation.assign(1, processInfos);
  ASSERT_TRUE(_permutation.taxRefundable());
}

TEST_F(ATaxRefundable, IsRefundableIfOneProcessingInfoWithCancellationIndX)
{
  r3Record.taxNonrefundableInd() = RefundPermutation::BLANK;
  r3Record.cancellationInd() = RefundPermutation::HUNDRED_PERCENT_PENALTY;
  RefundProcessInfo processInfo(&r3Record, nullptr, nullptr);
  processInfos.push_back(&processInfo);
  _permutation.assign(1, processInfos);;
  ASSERT_TRUE(_permutation.taxRefundable());
}

TEST_F(ATaxRefundable, IsNonrefundableIfCancellationIndXAndTaxNonrefundableIndX)
{
  r3Record.taxNonrefundableInd() = RefundPermutation::TAX_NON_REFUNDABLE;
  r3Record.cancellationInd() = RefundPermutation::HUNDRED_PERCENT_PENALTY;
  RefundProcessInfo processInfo(&r3Record, nullptr, nullptr);
  processInfos.push_back(&processInfo);
  _permutation.assign(1, processInfos);
  ASSERT_FALSE(_permutation.taxRefundable());
}

} // end of tse namespace
