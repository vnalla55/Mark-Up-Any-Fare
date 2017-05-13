// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         18-10-2011
//! \file         SoloPQTest.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
// -------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/test/TestPQItem.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace shpq
{

class MockTravelSeg : public TravelSeg
{
public:
  MockTravelSeg* clone(DataHandle& dh) const
  {
    TSE_ASSERT(false);
    return 0;
  }
};

class SoloPQTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloPQTest);

  CPPUNIT_TEST(testPQOperations);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

public:
  void testPQOperations()
  {
    ShoppingTrx* trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(trx);
    trx->journeyItin() = _memHandle.create<Itin>();
    // Add 2 travel segs
    trx->journeyItin()->travelSeg().push_back(_memHandle.create<MockTravelSeg>());
    trx->journeyItin()->travelSeg().push_back(_memHandle.create<MockTravelSeg>());

    // Attach Fake origin/destination
    trx->journeyItin()->travelSeg().front()->origin() = createLoc(trx, "LAX");
    trx->journeyItin()->travelSeg().front()->destination() = createLoc(trx, "DFW");

    // DiversityModel instance should be created and passed to constructor if we add calls to
    // getNextFarepath
    ItinStatistic stats(*trx);
    SoloPQ pq(*trx, stats, 0);

    CPPUNIT_ASSERT_EQUAL(pq.empty(), true);

    // insert 4 new items into pq
    pq.enqueue(test::TestPQItem::create(20.0, SoloPQItem::SP_LEVEL));
    pq.enqueue(test::TestPQItem::create(20.0, SoloPQItem::CR_LEVEL));
    pq.enqueue(test::TestPQItem::create(50.0, SoloPQItem::CRC_LEVEL));
    pq.enqueue(test::TestPQItem::create(10.0, SoloPQItem::SP_LEVEL));

    CPPUNIT_ASSERT_EQUAL(pq.empty(), false);

    CPPUNIT_ASSERT_EQUAL(4u, static_cast<unsigned>(pq.size()));

    // check the first item
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, pq.peek()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(SoloPQItem::SP_LEVEL, pq.peek()->getLevel());
    // get the first item
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, pq.dequeue()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(3u, static_cast<unsigned>(pq.size()));

    // check and get the next item
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, pq.peek()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(SoloPQItem::CR_LEVEL, pq.peek()->getLevel());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, pq.dequeue()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(2u, static_cast<unsigned>(pq.size()));

    // insert another item
    pq.enqueue(test::TestPQItem::create(25.0, SoloPQItem::SP_LEVEL));

    // check and get the next item
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, pq.peek()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(SoloPQItem::SP_LEVEL, pq.peek()->getLevel());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, pq.dequeue()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(2u, static_cast<unsigned>(pq.size()));

    // check and get the next item
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0, pq.peek()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(SoloPQItem::SP_LEVEL, pq.peek()->getLevel());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0, pq.dequeue()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned>(pq.size()));

    // check and get the next item
    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, pq.peek()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(SoloPQItem::CRC_LEVEL, pq.peek()->getLevel());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, pq.dequeue()->getScore(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned>(pq.size()));
  }

  Loc* createLoc(ShoppingTrx* trx, const LocCode& code)
  {
    Loc* loc = trx->dataHandle().create<Loc>();
    loc->loc() = code;
    return loc;
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SoloPQTest);
} /* namespace shpq */
} /* namespace tse */
