#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "RexPricing/PenaltyEstimator.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/PricingTrx.h"


namespace tse
{

class PenaltyEstimatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PenaltyEstimatorTest);

  CPPUNIT_TEST(testHighestOfAllFCs_1);
  CPPUNIT_TEST(testHighestOfAllFCs_2);
  CPPUNIT_TEST(testHighestOfChangedFCs_1);
  CPPUNIT_TEST(testHighestOfChangedFCs_2);
  CPPUNIT_TEST(testEachOfChangedFCs);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memH;
  PricingTrx* _trx;
  CurrencyCode _paymentCurrency;

public:
  void setUp()
  {
    _trx = _memH.create<PricingTrx>();
    _paymentCurrency = "USD";
  }

  void tearDown()
  {
    _memH.clear();
  }

  void testHighestOfAllFCs_1()
  {
    ProcessTagPermutation* perm = createProcessTagPermutation();
    perm->processTags()[0]->record3()->orig() =
        createVoluntaryChangesInfo(100.0, HIGHEST_OF_CHANGED_FC);

    perm->processTags()[1]->record3()->orig() =
        createVoluntaryChangesInfo(200.0, HIGHEST_OF_ALL_FC);

    perm->processTags()[2]->record3()->orig() =
        createVoluntaryChangesInfo(150.0, HIGHEST_FROM_CHANGED_PU_ADDS);

    std::vector<uint32_t> unmatchedFCs;

    for(uint32_t i = 0; i < 3; ++i)
    {
      unmatchedFCs.push_back(i);
    }

    PenaltyEstimator penaltyEstimator(unmatchedFCs, _paymentCurrency, *_trx);
    penaltyEstimator(perm);

    CPPUNIT_ASSERT_EQUAL(200.0, perm->getEstimatedChangeFee());
  }

  void testHighestOfAllFCs_2()
  {
    ProcessTagPermutation* perm = createProcessTagPermutation();

    perm->processTags()[0]->record3()->orig() =
        createVoluntaryChangesInfo(100.0, HIGHEST_FROM_CHANGED_PU_ADDS);

    perm->processTags()[1]->record3()->orig() =
        createVoluntaryChangesInfo(200.0, HIGHEST_FROM_CHANGED_PU_ADDS);

    perm->processTags()[2]->record3()->orig() =
        createVoluntaryChangesInfo(150.0, HIGHEST_OF_CHANGED_FC);

    std::vector<uint32_t> unmatchedFCs;

    for(uint32_t i = 0; i < 3; ++i)
    {
      unmatchedFCs.push_back(i);
    }

    PenaltyEstimator penaltyEstimator(unmatchedFCs, _paymentCurrency, *_trx);
    penaltyEstimator(perm);

    CPPUNIT_ASSERT_EQUAL(200.0, perm->getEstimatedChangeFee());
  }

  void testHighestOfChangedFCs_1()
  {
    ProcessTagPermutation* perm = createProcessTagPermutation();

    perm->processTags()[0]->record3()->orig() =
        createVoluntaryChangesInfo(100.0, HIGHEST_OF_CHANGED_FC);

    perm->processTags()[1]->record3()->orig() =
        createVoluntaryChangesInfo(200.0, HIGHEST_OF_CHANGED_FC);

    perm->processTags()[2]->record3()->orig() =
        createVoluntaryChangesInfo(150.0, EACH_OF_CHANGED_FC);

    std::vector<uint32_t> unmatchedFCs;
    unmatchedFCs.push_back(0);
    unmatchedFCs.push_back(2);

    PenaltyEstimator penaltyEstimator(unmatchedFCs, _paymentCurrency, *_trx);
    penaltyEstimator(perm);

    CPPUNIT_ASSERT_EQUAL(150.0, perm->getEstimatedChangeFee());
  }

  void testHighestOfChangedFCs_2()
  {
    ProcessTagPermutation* perm = createProcessTagPermutation();

    perm->processTags()[0]->record3()->orig() =
        createVoluntaryChangesInfo(100.0, HIGHEST_OF_CHANGED_FC);

    perm->processTags()[1]->record3()->orig() =
        createVoluntaryChangesInfo(200.0, HIGHEST_OF_CHANGED_FC);

    perm->processTags()[2]->record3()->orig() =
        createVoluntaryChangesInfo(150.0, EACH_OF_CHANGED_FC);

    std::vector<uint32_t> unmatchedFCs;

    PenaltyEstimator penaltyEstimator(unmatchedFCs, _paymentCurrency, *_trx);
    penaltyEstimator(perm);

    CPPUNIT_ASSERT_EQUAL(0.0, perm->getEstimatedChangeFee());
  }

  void testEachOfChangedFCs()
  {
    ProcessTagPermutation* perm = createProcessTagPermutation();

    perm->processTags()[0]->record3()->orig() =
        createVoluntaryChangesInfo(50.0, EACH_OF_CHANGED_FC);

    perm->processTags()[1]->record3()->orig() =
        createVoluntaryChangesInfo(50.0, EACH_OF_CHANGED_FC);

    perm->processTags()[2]->record3()->orig() =
        createVoluntaryChangesInfo(100.0, EACH_OF_CHANGED_FC);

    std::vector<uint32_t> unmatchedFCs;

    for(uint32_t i = 0; i < 3; ++i)
    {
      unmatchedFCs.push_back(i);
    }

    PenaltyEstimator penaltyEstimator(unmatchedFCs, _paymentCurrency, *_trx);
    penaltyEstimator(perm);

    CPPUNIT_ASSERT_EQUAL(200.0, perm->getEstimatedChangeFee());
  }



private:

  const static Indicator HIGHEST_OF_CHANGED_FC = '1';
  const static Indicator HIGHEST_OF_ALL_FC = '2';
  const static Indicator EACH_OF_CHANGED_FC = '3';
  const static Indicator HIGHEST_FROM_CHANGED_PU = '4';
  const static Indicator HIGHEST_FROM_CHANGED_PU_ADDS = '5';

  ProcessTagPermutation* createProcessTagPermutation()
  {
    ProcessTagPermutation* permutation = _memH(new ProcessTagPermutation());
    ProcessTagInfo* pti =  0;

    for(uint16_t i = 1; i <= 3; ++i)
    {
      FareCompInfo* fci = _memH(new FareCompInfo());
      fci->fareCompNumber() = i;
      pti = _memH(new ProcessTagInfo());
      pti->fareCompInfo() = fci;
      permutation->processTags().push_back(pti);
    }

    return permutation;
  }

  VoluntaryChangesInfo* createVoluntaryChangesInfo(MoneyAmount moneyAmout,
                                                   Indicator feeAppl)
  {
    VoluntaryChangesInfo* r3 = _memH(new VoluntaryChangesInfo());
    r3->penaltyAmt1() = moneyAmout;
    r3->feeAppl() = feeAppl;

    return r3;
  }



};

CPPUNIT_TEST_SUITE_REGISTRATION(PenaltyEstimatorTest);

}
