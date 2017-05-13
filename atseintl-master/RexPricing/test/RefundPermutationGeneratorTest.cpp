#include "RexPricing/RefundPermutationGenerator.h"

#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "DataModel/Agent.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/Fare.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Rules/RuleUtil.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{
using namespace boost::assign;

class RefundPermutationGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RefundPermutationGeneratorTest);

  CPPUNIT_TEST(testGenerate_emptyOptions);
  CPPUNIT_TEST(testGenerate_oneFareWithOneR3);
  CPPUNIT_TEST(testGenerate_oneFareWithTwoR3);
  CPPUNIT_TEST(testGenerate_twoFaresWithOneR3);
  CPPUNIT_TEST(testGenerate_twoFaresWithTwoR3);
  CPPUNIT_TEST(testGenerate_twoFaresWithThreeR3);
  CPPUNIT_TEST(testGenerate_threeFaresWithTwoR3);

  CPPUNIT_TEST(testFindInOptions_found);
  CPPUNIT_TEST(testFindInOptions_notFound_notFareByRule);
  CPPUNIT_TEST(testFindInOptions_foundBaseFare);
  CPPUNIT_TEST(testFindInOptions_notFoundBaseFare);

  CPPUNIT_SKIP_TEST(testProcessForSMP_found_perms);
  CPPUNIT_TEST(testProcessForSMP_notFound_perms);

  CPPUNIT_TEST_SUITE_END();

  RefundPricingTrx* _trx;
  RefundPermutationGenerator* _gen;
  TestMemHandle _memH;

public:
  RefundPermutationGeneratorTest()
  {
    log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());
  }

  void setUp()
  {
    _trx = _memH.insert(new RefundPricingTrx);
    _gen = _memH.insert(new RefundPermutationGenerator(*_trx, log4cxx::Logger::getLogger("null")));
  }

  void setUpForSMP()
  {
    _trx->setOptions(_memH.create<PricingOptions>());
    _trx->setRequest(_memH.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memH.create<Agent>();
  }

  void tearDown() { _memH.clear(); }

  PaxTypeFare* createPaxTypeFare(int fcNumber)
  {
    PaxTypeFare* ptf = _memH.insert(new PaxTypeFare);
    FareMarket* fm = _memH.insert(new FareMarket);
    ptf->fareMarket() = fm;
    fm->fareCompInfo() = _memH.insert(new FareCompInfo);
    fm->fareCompInfo()->fareCompNumber() = fcNumber;
    fm->fareCompInfo()->fareMarket() = fm;
    return ptf;
  }

  VoluntaryRefundsInfo* createRecord3(int item = 32435)
  {
    VoluntaryRefundsInfo* vri = _memH.insert(new VoluntaryRefundsInfo);
    vri->itemNo() = item;
    vri->repriceInd() = 'A';
    return vri;
  }

  void prepareFarePath(FarePath& farePath, const int index)
  {
    farePath.pricingUnit().push_back(_memH.create<PricingUnit>());
    for (int i = 0; i < index; i++)
    {
      FareUsage* fareUsage = _memH.create<FareUsage>();
      farePath.pricingUnit().front()->fareUsage().push_back(fareUsage);
      PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
      fareUsage->paxTypeFare() = ptf;
      VoluntaryRefundsInfo* voluntaryRefund = createRecord3(i);
      voluntaryRefund->psgType() = "ADT";
      voluntaryRefund->depOfJourneyInd() = ' ';
      voluntaryRefund->puInd() = ' ';
      voluntaryRefund->fareComponentInd() = ' ';
      voluntaryRefund->advCancelFromTo() = ' ';
      ptf->paxType() = _memH.create<PaxType>();
      ptf->paxType()->paxType() = "ADT";
      Fare* fare = _memH.create<Fare>();
      ptf->setFare(fare);
      FareInfo* fareInfo = _memH.create<FareInfo>();
      fare->setFareInfo(fareInfo);
      fareInfo->fareClass() = "ABCD";
    }
  }

  void assertRefundProcessInfo(const PaxTypeFare* ptf,
                               const VoluntaryRefundsInfo* r3,
                               unsigned short fcNumber,
                               const RefundProcessInfo& rpi)
  {
    CPPUNIT_ASSERT_EQUAL(r3, &rpi.record3());
    CPPUNIT_ASSERT_EQUAL(ptf, &rpi.paxTypeFare());
    CPPUNIT_ASSERT_EQUAL(ptf->fareMarket()->fareCompInfo(), &rpi.fareCompInfo());
    CPPUNIT_ASSERT_EQUAL(fcNumber, rpi.fareCompInfo().fareCompNumber());
  }

  typedef const std::vector<const PaxTypeFare*> Fares;
  typedef std::pair<const PaxTypeFare*, const VoluntaryRefundsInfo*> Combination;

  void assertPermutation(unsigned permNumber,
                         std::vector<Combination> combinations,
                         const RefundPermutation& permutation)
  {
    CPPUNIT_ASSERT_EQUAL(permNumber, permutation.number());
    CPPUNIT_ASSERT_EQUAL(combinations.size(), permutation.processInfos().size());
    for (unsigned i = 0; i < combinations.size(); ++i)
      assertRefundProcessInfo(
          combinations[i].first, combinations[i].second, i + 1, *permutation.processInfos()[i]);
  }

  void testGenerate_emptyOptions()
  {
    Fares fares;
    RefundPricingTrx::Permutations permutations;
    CPPUNIT_ASSERT_NO_THROW(_gen->generate(&_trx->refundOptions(), fares, permutations));
    CPPUNIT_ASSERT(permutations.empty());
  }

  enum
  { fc1 = 1,
    fc2 = 2,
    fc3 = 3 };

  void testGenerate_oneFareWithOneR3()
  {
    PaxTypeFare* ptf = createPaxTypeFare(fc1);
    VoluntaryRefundsInfo* r3 = createRecord3();
    _trx->insertOption(ptf, r3);

    Fares fares(1, ptf);

    RefundPricingTrx::Permutations permutations;
    CPPUNIT_ASSERT_NO_THROW(_gen->generate(&_trx->refundOptions(), fares, permutations));
    CPPUNIT_ASSERT_EQUAL(size_t(1), permutations.size());

    std::vector<Combination> comb;
    comb += Combination(ptf, r3);
    assertPermutation(1, comb, *permutations[0]);
  }

  void testGenerate_oneFareWithTwoR3()
  {
    PaxTypeFare* ptf = createPaxTypeFare(fc1);
    VoluntaryRefundsInfo* r3[2] = {createRecord3(), createRecord3()};

    for (unsigned i = 0; i < 2; ++i)
      _trx->insertOption(ptf, r3[i]);

    Fares fares(1, ptf);

    RefundPricingTrx::Permutations permutations;
    CPPUNIT_ASSERT_NO_THROW(_gen->generate(&_trx->refundOptions(), fares, permutations));

    size_t expectPermNumber = 2;
    CPPUNIT_ASSERT_EQUAL(expectPermNumber, permutations.size());

    std::vector<std::vector<Combination>> comb(expectPermNumber);
    comb[0] += Combination(ptf, r3[0]);
    comb[1] += Combination(ptf, r3[1]);

    for (unsigned i = 0; i < expectPermNumber; ++i)
      assertPermutation(i + 1, comb[i], *permutations[i]);
  }

  void testGenerate_twoFaresWithOneR3()
  {
    PaxTypeFare* ptf[2] = {createPaxTypeFare(fc1), createPaxTypeFare(fc2)};
    VoluntaryRefundsInfo* r3[2] = {createRecord3(), createRecord3()};

    for (unsigned i = 0; i < 2; ++i)
      _trx->insertOption(ptf[i], r3[i]);

    Fares fares(ptf, ptf + 2);

    RefundPricingTrx::Permutations permutations;
    CPPUNIT_ASSERT_NO_THROW(_gen->generate(&_trx->refundOptions(), fares, permutations));

    size_t expectPermNumber = 1;
    CPPUNIT_ASSERT_EQUAL(expectPermNumber, permutations.size());

    std::vector<std::vector<Combination>> comb(expectPermNumber);
    comb[0] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[1]);

    for (unsigned i = 0; i < expectPermNumber; ++i)
      assertPermutation(i + 1, comb[i], *permutations[i]);
  }

  void testGenerate_twoFaresWithTwoR3()
  {
    PaxTypeFare* ptf[2] = {createPaxTypeFare(fc1), createPaxTypeFare(fc2)};
    VoluntaryRefundsInfo* r3[4] = {
        createRecord3(), createRecord3(), createRecord3(), createRecord3()};
    for (unsigned i = 0; i < 4; ++i)
      _trx->insertOption(ptf[i / 2], r3[i]);

    Fares fares(ptf, ptf + 2);

    RefundPricingTrx::Permutations permutations;
    CPPUNIT_ASSERT_NO_THROW(_gen->generate(&_trx->refundOptions(), fares, permutations));

    size_t expectPermNumber = 4;
    CPPUNIT_ASSERT_EQUAL(expectPermNumber, permutations.size());

    std::vector<std::vector<Combination>> comb(expectPermNumber);
    comb[0] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[2]);
    comb[1] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[3]);
    comb[2] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[2]);
    comb[3] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[3]);

    for (unsigned i = 0; i < expectPermNumber; ++i)
      assertPermutation(i + 1, comb[i], *permutations[i]);
  }

  void testGenerate_twoFaresWithThreeR3()
  {
    PaxTypeFare* ptf[2] = {createPaxTypeFare(fc1), createPaxTypeFare(fc2)};
    VoluntaryRefundsInfo* r3[6] = {createRecord3(),
                                   createRecord3(),
                                   createRecord3(),
                                   createRecord3(),
                                   createRecord3(),
                                   createRecord3()};
    for (unsigned i = 0; i < 6; ++i)
      _trx->insertOption(ptf[i / 3], r3[i]);

    Fares fares(ptf, ptf + 2);

    RefundPricingTrx::Permutations permutations;
    CPPUNIT_ASSERT_NO_THROW(_gen->generate(&_trx->refundOptions(), fares, permutations));

    size_t expectPermNumber = 9;
    CPPUNIT_ASSERT_EQUAL(expectPermNumber, permutations.size());

    std::vector<std::vector<Combination>> comb(expectPermNumber);
    comb[0] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[3]);
    comb[1] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[4]);
    comb[2] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[5]);
    comb[3] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[3]);
    comb[4] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[4]);
    comb[5] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[5]);
    comb[6] += Combination(ptf[0], r3[2]), Combination(ptf[1], r3[3]);
    comb[7] += Combination(ptf[0], r3[2]), Combination(ptf[1], r3[4]);
    comb[8] += Combination(ptf[0], r3[2]), Combination(ptf[1], r3[5]);

    for (unsigned i = 0; i < expectPermNumber; ++i)
      assertPermutation(i + 1, comb[i], *permutations[i]);
  }

  void testGenerate_threeFaresWithTwoR3()
  {
    PaxTypeFare* ptf[3] = {createPaxTypeFare(fc1), createPaxTypeFare(fc2), createPaxTypeFare(fc3)};
    VoluntaryRefundsInfo* r3[6] = {createRecord3(),
                                   createRecord3(),
                                   createRecord3(),
                                   createRecord3(),
                                   createRecord3(),
                                   createRecord3()};
    for (unsigned i = 0; i < 6; ++i)
      _trx->insertOption(ptf[i / 2], r3[i]);

    Fares fares(ptf, ptf + 3);

    RefundPricingTrx::Permutations permutations;
    CPPUNIT_ASSERT_NO_THROW(_gen->generate(&_trx->refundOptions(), fares, permutations));

    size_t expectPermNumber = 8;
    CPPUNIT_ASSERT_EQUAL(expectPermNumber, permutations.size());

    std::vector<std::vector<Combination>> comb(expectPermNumber);

    comb[0] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[2]), Combination(ptf[2], r3[4]);
    comb[1] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[2]), Combination(ptf[2], r3[5]);

    comb[2] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[3]), Combination(ptf[2], r3[4]);
    comb[3] += Combination(ptf[0], r3[0]), Combination(ptf[1], r3[3]), Combination(ptf[2], r3[5]);

    comb[4] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[2]), Combination(ptf[2], r3[4]);
    comb[5] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[2]), Combination(ptf[2], r3[5]);

    comb[6] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[3]), Combination(ptf[2], r3[4]);
    comb[7] += Combination(ptf[0], r3[1]), Combination(ptf[1], r3[3]), Combination(ptf[2], r3[5]);

    for (unsigned i = 0; i < expectPermNumber; ++i)
      assertPermutation(i + 1, comb[i], *permutations[i]);
  }

  typedef std::pair<RefundPermutationGenerator::OptionsIt, RefundPermutationGenerator::OptionsIt>
  Range;

  void testFindInOptions_found()
  {
    PaxTypeFare ptf[3];
    VoluntaryRefundsInfo r3[4];
    _trx->insertOption(&ptf[0], &r3[0]);
    _trx->insertOption(&ptf[1], &r3[1]);
    _trx->insertOption(&ptf[1], &r3[2]);
    _trx->insertOption(&ptf[2], &r3[3]);

    Range range;
    CPPUNIT_ASSERT_NO_THROW(range = _gen->findInOptions(_trx->refundOptions(), &ptf[1]));
    CPPUNIT_ASSERT(range.first != range.second);
    CPPUNIT_ASSERT(range.second != _trx->refundOptions().end());
    CPPUNIT_ASSERT_EQUAL(const_cast<const PaxTypeFare* const>(&ptf[1]), range.first->first);
    CPPUNIT_ASSERT_EQUAL(const_cast<const PaxTypeFare* const>(&ptf[2]), range.second->first);
  }

  void testFindInOptions_notFound_notFareByRule()
  {
    PaxTypeFare ptf, fareNotByRule;
    VoluntaryRefundsInfo r3;
    _trx->insertOption(&ptf, &r3);

    Range range;
    CPPUNIT_ASSERT_THROW(range = _gen->findInOptions(_trx->refundOptions(), &fareNotByRule),
                         ErrorResponseException);
  }

  void populateBaseFare(PaxTypeFare* ptf)
  {
    PaxTypeFareRuleData* rd = _memH.insert(new PaxTypeFareRuleData);
    rd->baseFare() = createPaxTypeFare(0);
    ptf->setRuleData(25, _trx->dataHandle(), rd, true);
    ptf->status().set(PaxTypeFare::PTF_FareByRule);
  }

  void testFindInOptions_foundBaseFare()
  {
    PaxTypeFare ptf;
    populateBaseFare(&ptf);
    PaxTypeFare* baseFare = ptf.baseFare();

    VoluntaryRefundsInfo r3;
    _trx->insertOption(baseFare, &r3);

    Range range;
    CPPUNIT_ASSERT_NO_THROW(range = _gen->findInOptions(_trx->refundOptions(), &ptf));
    CPPUNIT_ASSERT(range.first != range.second);
    CPPUNIT_ASSERT_EQUAL(const_cast<const PaxTypeFare* const>(baseFare), range.first->first);
  }

  void testFindInOptions_notFoundBaseFare()
  {
    PaxTypeFare other, ptf;
    VoluntaryRefundsInfo r3;
    _trx->insertOption(&other, &r3);

    populateBaseFare(&ptf);

    Range range;
    CPPUNIT_ASSERT_THROW(range = _gen->findInOptions(_trx->refundOptions(), &ptf),
                         ErrorResponseException);
  }

  void testProcessForSMP_found_perms()
  {
    setUpForSMP();
    RefundPermutationGenerator::Permutations permutations;
    FarePath* farePath = _memH.create<FarePath>();
    prepareFarePath(*farePath, 2);
    RefundPermutationGenerator::FCtoSequence seqsByFc;
    _gen->processForSMP(permutations, *farePath, seqsByFc, smp::BOTH);
    CPPUNIT_ASSERT(!permutations.empty());
  }

  void testProcessForSMP_notFound_perms()
  {
    setUpForSMP();
    RefundPermutationGenerator::Permutations permutations;
    FarePath* farePath = _memH.create<FarePath>();
    prepareFarePath(*farePath, 0);
    RefundPermutationGenerator::FCtoSequence seqsByFc;
    _gen->processForSMP(permutations, *farePath, seqsByFc, smp::BOTH);
    CPPUNIT_ASSERT(permutations.empty());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RefundPermutationGeneratorTest);
}
