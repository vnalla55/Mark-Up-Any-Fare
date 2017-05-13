// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseConsts.h"
#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/FOS/AlreadyGeneratedSolutionPredicate.h"
#include "Pricing/Shopping/FOS/FosBoss.h"
#include "Pricing/Shopping/FOS/FosTaskScope.h"
#include "Pricing/Shopping/FOS/FosBaseGenerator.h"
#include "Pricing/Shopping/Utils/FosGenerator.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace fos
{

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 6, 1);
static DateTime ibDate = DateTime(2013, 6, 8);
static DateTime ibDate2 = DateTime(2013, 6, 9);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment Segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 2, "LH", "JFK", "EWR", "LH", DT(obDate, 10), DT(obDate, 11) },
  { 0, 2, "LH", "EWR", "DFW", "LH", DT(obDate, 11), DT(obDate, 12) },
  { 1, 0, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 1, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 2, "LH", "DFW", "EWR", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 2, "LH", "EWR", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }
};

const LegsBuilder::Segment SnowmanSegments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "EWR", "AA", DT(obDate, 10), DT(obDate, 11) },
  { 0, 0, "AA", "EWR", "DFW", "AA", DT(obDate, 12), DT(obDate, 13) },
  { 0, 1, "AA", "JFK", "EWR", "AA", DT(obDate, 10), DT(obDate, 11) },
  { 0, 1, "LH", "EWR", "DFW", "LH", DT(obDate, 13), DT(obDate, 14) },
  { 1, 0, "LH", "DFW", "EWR", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 0, "AA", "EWR", "JFK", "AA", DT(ibDate, 12), DT(ibDate, 13) },
  { 1, 1, "LH", "DFW", "EWR", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 1, "LH", "EWR", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }
};

const LegsBuilder::Segment DiamondSegments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "LHS", "AA", DT(obDate, 10), DT(obDate, 11) },
  { 0, 0, "LH", "LHS", "DFW", "LH", DT(obDate, 12), DT(obDate, 13) },
  { 0, 1, "AA", "JFK", "EWR", "AA", DT(obDate, 10), DT(obDate, 11) },
  { 0, 1, "LH", "EWR", "DFW", "LH", DT(obDate, 13), DT(obDate, 14) },
  { 1, 0, "LH", "DFW", "EWR", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 0, "AA", "EWR", "JFK", "AA", DT(ibDate, 12), DT(ibDate, 13) },
  { 1, 1, "AA", "DFW", "EWR", "AA", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 1, "AA", "EWR", "JFK", "AA", DT(ibDate, 12), DT(ibDate, 13) }
};

const LegsBuilder::Segment LongCnxSegments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "AA", DT(obDate, 7), DT(obDate, 8) }, // direct SOP
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 22), DT(obDate, 23) }, // direct SOP
  { 1, 0, "LH", "DFW", "EWR", "LH", DT(ibDate, 7), DT(ibDate, 8) },
  { 1, 0, "LH", "EWR", "JFK", "LH", DT(ibDate, 22), DT(ibDate, 23) }
};
#undef DT

// MOCKS
namespace
{
}
// ==================================
// TEST CLASS
// ==================================

class FosBossTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FosBossTest);

  CPPUNIT_TEST(testProcessOnlines);
  CPPUNIT_TEST(testProcessSnowman);
  CPPUNIT_TEST(testProcessDiamond);
  CPPUNIT_TEST(testProcessDiamondDeferred);
  CPPUNIT_TEST(testProcessCustom);
  //  CPPUNIT_TEST(testProcessLongCnx);
  CPPUNIT_TEST(testProcessDirects);

  CPPUNIT_TEST_SUITE_END();

protected:
  class FosTaskScopeMock : public FosTaskScope
  {
  public:
    void setNumDirectFos(uint32_t num) { _numDirectFos = num; }
    void setNumDirectFosPerCarrier(const CarrierCode& cxr, uint32_t num)
    {
      _numDirectFosPerCarrier[cxr] = num;
    }
    void setNumOnlineFos(uint32_t num) { _numOnlineFos = num; }
    void setNumOnlineFosPerCarrier(const CarrierCode& cxr, uint32_t num)
    {
      _numFosPerCarrier[cxr] = num;
    }
    void setNumSnowmanFos(uint32_t num) { _numSnowmanFos = num; }
    void setNumDiamondFos(uint32_t num) { _numDiamondFos = num; }
    void setNumCustomFos(uint32_t num) { _numCustomFos = num; }
    void setNumLongConxFos(uint32_t num) { _numLongConxFos = num; }
  };

  class MockFosGenerator : public FosBaseGenerator
  {
  public:
    MockFosGenerator(ShoppingTrx& trx,
                     FosFilterComposite& fosFilterComposite,
                     Diag910Collector* dc = 0)
      : FosBaseGenerator(trx, fosFilterComposite, dc)
    {
      _fosGenerator = createFosGenerator();
    }

    void initGenerators()
    {
      for (LegId legIdx = 0; legIdx < _trx.legs().size(); ++legIdx)
      {
        for (uint32_t sopIdx = 0; sopIdx < _trx.legs()[legIdx].sop().size(); ++sopIdx)
        {
          _fosGenerator->addSop(legIdx, sopIdx);
        }
      }

      _fosGenerators.push_back(_fosGenerator);
    }

    void addPredicates()
    {
      AlreadyGeneratedSolutionPredicate* alreadyGen =
          &_trx.dataHandle().safe_create<AlreadyGeneratedSolutionPredicate>(_trx);

      _fosGenerator->addPredicate(alreadyGen, "AlreadyGeneratedSolutionPredicate");
    }

  protected:
    utils::FosGenerator* _fosGenerator;
  };

private:
  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
  ItinStatistic* _itinStatistic;
  FosTaskScopeMock* _fosTaskScope;
  FosBoss* _boss;

protected:
  void initSegments()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();
  }

  void initSnowmanSegments()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(SnowmanSegments, boost::size(SnowmanSegments));
    builder.endBuilding();
  }

  void initDiamondSegments()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(DiamondSegments, boost::size(DiamondSegments));
    builder.endBuilding();
  }

  void initLongCnxSegments()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(LongCnxSegments, boost::size(LongCnxSegments));
    builder.endBuilding();
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    _itinStatistic = _memHandle.create<ItinStatistic>(*_trx);

    _fosTaskScope = _memHandle.create<FosTaskScopeMock>();

    _boss = _memHandle.create<FosBoss>(*_trx, _itinStatistic);
  }

  void tearDown() { _memHandle.clear(); }

  void testProcessOnlines()
  {
    initSegments();

    _fosTaskScope->setNumOnlineFos(2);
    _fosTaskScope->setNumOnlineFosPerCarrier("AA", 1);
    _fosTaskScope->setNumOnlineFosPerCarrier("LH", 1);

    _boss->process<MockFosGenerator>(*_fosTaskScope);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->flightMatrix().size());
  }

  void testProcessDirects()
  {
    initSegments();

    _fosTaskScope->setNumDirectFos(3);
    _fosTaskScope->setNumDirectFosPerCarrier("AA", 1);
    _fosTaskScope->setNumDirectFosPerCarrier("LH", 1);
    _fosTaskScope->setNumDirectFosPerCarrier(Diversity::INTERLINE_CARRIER, 1);

    _boss->process<MockFosGenerator>(*_fosTaskScope);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _trx->flightMatrix().size());
  }

  void testProcessSnowman()
  {
    initSnowmanSegments();

    _fosTaskScope->setNumSnowmanFos(3);

    _boss->process<MockFosGenerator>(*_fosTaskScope);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->flightMatrix().size());
  }

  void testProcessDiamond()
  {
    initDiamondSegments();

    _fosTaskScope->setNumDiamondFos(3);

    _boss->process<MockFosGenerator>(*_fosTaskScope);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _trx->flightMatrix().size());
  }

  void testProcessDiamondDeferred()
  {
    initDiamondSegments();

    _fosTaskScope->setNumOnlineFos(1);
    _fosTaskScope->setNumOnlineFosPerCarrier("AA", 1);
    _fosTaskScope->setNumDiamondFos(3);

    _boss->process<MockFosGenerator>(*_fosTaskScope);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _trx->flightMatrix().size());
  }

  void testProcessCustom()
  {
    initSegments();

    _trx->legs()[0].setCustomLeg(true);
    _trx->legs()[0].sop()[0].setCustomSop(true);

    _trx->legs()[1].setCustomLeg(true);
    _trx->legs()[1].sop()[0].setCustomSop(true);
    _trx->legs()[1].sop()[1].setCustomSop(true);

    _fosTaskScope->setNumCustomFos(4);

    _boss->process<MockFosGenerator>(*_fosTaskScope);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->flightMatrix().size());
  }

  void testProcessLongCnx()
  {
    initLongCnxSegments();

    _trx->getRequest()->percentOfLngCnxSolutions() = 100.0;
    _trx->legs()[1].sop()[2].isLngCnxSop() = true;

    _fosTaskScope->setNumLongConxFos(2);

    _boss->process<MockFosGenerator>(*_fosTaskScope);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->flightMatrix().size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FosBossTest);

} // fos
} // tse
