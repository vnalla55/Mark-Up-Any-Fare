#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "RexPricing/FarePathChangeDetermination.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/list_of.hpp>

namespace tse
{

class FakeFarePathChangeDetermination : public FarePathChangeDetermination
{
public:
  FakeFarePathChangeDetermination() : FarePathChangeDetermination() {}

protected:
  virtual bool sameFareComponents(const PaxTypeFare& newPtf, const PaxTypeFare& excPtf) const
  {
    return excPtf.fareMarket()->boardMultiCity() == newPtf.fareMarket()->boardMultiCity();
  }
};

class FarePathChangeDeterminationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FarePathChangeDeterminationTest);
  CPPUNIT_TEST(test1a_1a);
  CPPUNIT_TEST(test1a_1b);
  CPPUNIT_TEST(test1a_1ab);
  CPPUNIT_TEST(test1b_1ab);
  CPPUNIT_TEST(test1ab_1a);
  CPPUNIT_TEST(test1ab_1b);
  CPPUNIT_TEST(test1ab_1ab);
  CPPUNIT_TEST(test1ab_1ac);
  CPPUNIT_TEST(test1ab_1cb);
  CPPUNIT_TEST(test1acb_1ab);
  CPPUNIT_TEST(test1ab_1acb);

  CPPUNIT_TEST(test1a2b_1ab);
  CPPUNIT_TEST(test1ab_1a2b3c);
  CPPUNIT_TEST(test1ab_1c2a3b);
  CPPUNIT_TEST(test1ab_1a2c3b);
  CPPUNIT_TEST(test1a2b_1acb);
  CPPUNIT_TEST(test1abc_1a2c);

  CPPUNIT_TEST(test1ab2cd_1ab2c);
  CPPUNIT_TEST(test1ab2cd_1ab2d);
  CPPUNIT_TEST(test1ab2cd_1a2cd);
  CPPUNIT_TEST(test1ab2cd_1b2cd);
  CPPUNIT_TEST(test1a2cd_1ab2cd);
  CPPUNIT_TEST(test1b2cd_1ab2cd);
  CPPUNIT_TEST(test1ab2c_1ab2cd);
  CPPUNIT_TEST(test1ab2d_1ab2cd);

  CPPUNIT_TEST(test1abc2d_1ab2cf);
  CPPUNIT_TEST(test1abc2d_1b2cd);

  CPPUNIT_TEST(testSameTravelSegments55);
  CPPUNIT_TEST(testSameTravelSegments65);
  CPPUNIT_TEST(testSameTravelSegments24);
  CPPUNIT_TEST(testSameTravelSegmentsOpen12);
  CPPUNIT_TEST(testSameTravelSegmentsOpen21);
  CPPUNIT_TEST(testSameTravelSegmentsOpen20);
  CPPUNIT_TEST(testSameTravelSegmentsOpen01);
  CPPUNIT_TEST(testSameTravelSegmentsOpenLast01newLonger);
  CPPUNIT_TEST(testSameTravelSegmentsOpenLast01newShorter);
  CPPUNIT_TEST(testSameTravelSegmentsOpenLast10newShorter);
  CPPUNIT_TEST(testSameTravelSegmentsOpenLast10newLonger);

  CPPUNIT_TEST_SUITE_END();

private:
  FarePath* _newFP;
  FarePath* _excFP;
  RexPricingTrx* _trx;
  FarePathChangeDetermination* _icd;
  TestMemHandle _memory;

public:
  void setUp()
  {
    _newFP = _memory(new FarePath);
    _excFP = _memory(new FarePath);
    _trx = _memory(new RexPricingTrx);
    _trx->exchangeItin().push_back(_memory(new ExcItin));
    _trx->exchangeItin().front()->farePath().push_back(_excFP);

    _icd = _memory(new FakeFarePathChangeDetermination());

    // for default we assume itin is changed and we will compute penaltys
    CPPUNIT_ASSERT(!_newFP->ignoreReissueCharges());
  }

  void tearDown() { _memory.clear(); }

  void addPricingUnit(FarePath& fp, const std::vector<std::string>& fareComponents)
  {
    fp.pricingUnit().push_back(_memory(new PricingUnit));

    std::vector<std::string>::const_iterator fci = fareComponents.begin();
    std::vector<std::string>::const_iterator fcie = fareComponents.end();
    for (; fci != fcie; ++fci)
    {
      fp.pricingUnit().back()->fareUsage().push_back(_memory(new FareUsage));
      fp.pricingUnit().back()->fareUsage().back()->paxTypeFare() = _memory(new PaxTypeFare);
      fp.pricingUnit().back()->fareUsage().back()->paxTypeFare()->fareMarket() =
          _memory(new FareMarket);
      fp.pricingUnit().back()->fareUsage().back()->paxTypeFare()->fareMarket()->boardMultiCity() =
          *fci;
    }
  }

  void changed(int puIndex, int fuIndex)
  {
    CPPUNIT_ASSERT(
        _icd->changedFC(*_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(_icd->insideChangedPU(
        *_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(!_icd->sameInsideExtendedPU(
        *_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(!_newFP->ignoreReissueCharges());
  }
  void unchangedInsideChangedPU(int puIndex, int fuIndex)
  {
    CPPUNIT_ASSERT(
        !_icd->changedFC(*_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(_icd->insideChangedPU(
        *_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(!_newFP->ignoreReissueCharges());
  }
  void unchangedInsideExtendedPU(int puIndex, int fuIndex)
  {
    CPPUNIT_ASSERT(
        !_icd->changedFC(*_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(_icd->sameInsideExtendedPU(
        *_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(!_newFP->ignoreReissueCharges());
  }
  void unchangedOnlyInExtendedPU(int puIndex, int fuIndex)
  {
    CPPUNIT_ASSERT(
        !_icd->changedFC(*_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(!_icd->insideChangedPU(
        *_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(_icd->sameInsideExtendedPU(
        *_excFP->pricingUnit()[puIndex]->fareUsage()[fuIndex]->paxTypeFare()));
    CPPUNIT_ASSERT(!_newFP->ignoreReissueCharges());
  }

  // test names points to FP construction
  // numbers - pricing units
  // letters - fare components
  //_ indicate exc/new FP

  void test1a_1a()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA"));

    _icd->determineChanges(*_newFP, *_trx);

    CPPUNIT_ASSERT(_newFP->ignoreReissueCharges());
  }

  void test1a_1b()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_newFP, boost::assign::list_of("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    changed(0, 0);
  }

  void test1a_1ab()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(0, 0);
  }

  void test1b_1ab()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(0, 0);
  }

  void test1ab_1a()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedInsideChangedPU(0, 0);
    changed(0, 1);
  }

  void test1ab_1b()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    changed(0, 0);
    unchangedInsideChangedPU(0, 1);
  }

  void test1ab_1ab()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    CPPUNIT_ASSERT(_newFP->ignoreReissueCharges());
  }

  void test1ab_1ac()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("CCC"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedInsideChangedPU(0, 0);
    unchangedInsideExtendedPU(0, 0);
    changed(0, 1);
  }

  void test1ab_1cb()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    changed(0, 0);
    unchangedInsideChangedPU(0, 1);
    unchangedInsideExtendedPU(0, 1);
  }

  void test1acb_1ab()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("CCC")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedInsideChangedPU(0, 0);
    changed(0, 1);
    unchangedInsideChangedPU(0, 2);
  }

  void test1ab_1acb()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("CCC")("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(0, 0);
    unchangedOnlyInExtendedPU(0, 1);
  }

  void test1a2b_1ab()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_excFP, boost::assign::list_of("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    CPPUNIT_ASSERT(_newFP->ignoreReissueCharges());
  }

  void test1ab_1a2b3c()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_newFP, boost::assign::list_of("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC"));

    _icd->determineChanges(*_newFP, *_trx);

    CPPUNIT_ASSERT(_newFP->ignoreReissueCharges());
  }

  void test1ab_1c2a3b()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_newFP, boost::assign::list_of("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    CPPUNIT_ASSERT(_newFP->ignoreReissueCharges());
  }

  void test1ab_1a2c3b()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC"));
    addPricingUnit(*_newFP, boost::assign::list_of("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    CPPUNIT_ASSERT(_newFP->ignoreReissueCharges());
  }

  void test1a2b_1acb()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_excFP, boost::assign::list_of("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("CCC")("BBB"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(0, 0);
    unchangedOnlyInExtendedPU(1, 0);
  }

  void test1abc_1a2c()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB")("CCC"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedInsideChangedPU(0, 0);
    changed(0, 1);
    unchangedInsideChangedPU(0, 2);
  }

  void test1ab2cd_1ab2c()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_excFP, boost::assign::list_of("CCC")("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedInsideChangedPU(1, 0);
    changed(1, 1);
  }
  void test1ab2cd_1ab2d()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_excFP, boost::assign::list_of("CCC")("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("DDD"));

    _icd->determineChanges(*_newFP, *_trx);

    changed(1, 0);
    unchangedInsideChangedPU(1, 1);
  }
  void test1ab2cd_1a2cd()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_excFP, boost::assign::list_of("CCC")("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("DDD"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedInsideChangedPU(0, 0);
    changed(0, 1);
  }
  void test1ab2cd_1b2cd()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_excFP, boost::assign::list_of("CCC")("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("DDD"));

    _icd->determineChanges(*_newFP, *_trx);

    changed(0, 0);
    unchangedInsideChangedPU(0, 1);
  }
  void test1a2cd_1ab2cd()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA"));
    addPricingUnit(*_excFP, boost::assign::list_of("CCC")("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("DDD"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(0, 0);
  }
  void test1b2cd_1ab2cd()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("BBB"));
    addPricingUnit(*_excFP, boost::assign::list_of("CCC")("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("DDD"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(0, 0);
  }
  void test1ab2c_1ab2cd()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_excFP, boost::assign::list_of("CCC"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("DDD"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(1, 0);
  }
  void test1ab2d_1ab2cd()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_excFP, boost::assign::list_of("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("DDD"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(1, 0);
  }

  void test1abc2d_1ab2cf()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB")("CCC"));
    addPricingUnit(*_excFP, boost::assign::list_of("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("AAA")("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("FFF"));

    _icd->determineChanges(*_newFP, *_trx);

    unchangedOnlyInExtendedPU(0, 2);
    changed(1, 0);
  }

  void test1abc2d_1b2cd()
  {
    addPricingUnit(*_excFP, boost::assign::list_of("AAA")("BBB")("CCC"));
    addPricingUnit(*_excFP, boost::assign::list_of("DDD"));
    addPricingUnit(*_newFP, boost::assign::list_of("BBB"));
    addPricingUnit(*_newFP, boost::assign::list_of("CCC")("DDD"));

    _icd->determineChanges(*_newFP, *_trx);

    changed(0, 0);
    unchangedInsideChangedPU(0, 1);
    unchangedInsideChangedPU(0, 2);
  }

  std::vector<TravelSeg*>& getTSs(int i)
  {
    std::vector<TravelSeg*>& tsVec = *_memory(new std::vector<TravelSeg*>);
    for (; i != 0; --i)
      tsVec.push_back(_memory(new AirSeg));

    return tsVec;
  }

  void testSameTravelSegments55()
  {
    CPPUNIT_ASSERT(_icd->sameTravelSegments(getTSs(5), getTSs(5)));
  }
  void testSameTravelSegments65()
  {
    CPPUNIT_ASSERT(!_icd->sameTravelSegments(getTSs(6), getTSs(5)));
  }
  void testSameTravelSegments24()
  {
    CPPUNIT_ASSERT(!_icd->sameTravelSegments(getTSs(2), getTSs(4)));
  }
  void testSameTravelSegmentsOpen12()
  {
    std::vector<TravelSeg*>& newSegs = getTSs(5);
    std::vector<TravelSeg*>& excSegs = getTSs(6);

    newSegs[1]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;
    excSegs[1]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;
    excSegs[2]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT(_icd->sameTravelSegments(newSegs, excSegs));
  }
  void testSameTravelSegmentsOpen21()
  {
    std::vector<TravelSeg*>& newSegs = getTSs(5);
    std::vector<TravelSeg*>& excSegs = getTSs(4);

    newSegs[2]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;
    newSegs[3]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;
    excSegs[2]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT(_icd->sameTravelSegments(newSegs, excSegs));
  }
  void testSameTravelSegmentsOpen20()
  {
    std::vector<TravelSeg*>& newSegs = getTSs(5);
    std::vector<TravelSeg*>& excSegs = getTSs(5);
    newSegs[2]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;
    newSegs[3]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT(!_icd->sameTravelSegments(newSegs, excSegs));
  }
  void testSameTravelSegmentsOpen01()
  {
    std::vector<TravelSeg*>& newSegs = getTSs(6);
    std::vector<TravelSeg*>& excSegs = getTSs(6);
    excSegs[2]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT(!_icd->sameTravelSegments(newSegs, excSegs));
  }
  void testSameTravelSegmentsOpenLast01newShorter()
  {
    std::vector<TravelSeg*>& newSegs = getTSs(2);
    std::vector<TravelSeg*>& excSegs = getTSs(7);
    excSegs[6]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT(!_icd->sameTravelSegments(newSegs, excSegs));
  }
  void testSameTravelSegmentsOpenLast01newLonger()
  {
    std::vector<TravelSeg*>& newSegs = getTSs(4);
    std::vector<TravelSeg*>& excSegs = getTSs(3);
    excSegs[2]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT(!_icd->sameTravelSegments(newSegs, excSegs));
  }
  void testSameTravelSegmentsOpenLast10newShorter()
  {
    std::vector<TravelSeg*>& newSegs = getTSs(2);
    std::vector<TravelSeg*>& excSegs = getTSs(3);
    newSegs[1]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT(!_icd->sameTravelSegments(newSegs, excSegs));
  }
  void testSameTravelSegmentsOpenLast10newLonger()
  {
    std::vector<TravelSeg*>& newSegs = getTSs(6);
    std::vector<TravelSeg*>& excSegs = getTSs(3);
    newSegs[5]->changeStatus() = TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT(!_icd->sameTravelSegments(newSegs, excSegs));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FarePathChangeDeterminationTest);
}
