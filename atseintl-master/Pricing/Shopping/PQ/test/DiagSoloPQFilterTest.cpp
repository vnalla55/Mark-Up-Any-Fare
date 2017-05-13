// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         04-11-2011
//! \file         DiagSoloPQFilterTest.cpp
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
//
// -------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "Pricing/Shopping/PQ/test/TestPQItem.h"
#include "Pricing/Shopping/PQ/DiagSoloPQFilter.h"

namespace tse
{
namespace shpq
{

class DiagSoloPQFilterTest : public CppUnit::TestFixture
{
private:
  CPPUNIT_TEST_SUITE(DiagSoloPQFilterTest);

  CPPUNIT_TEST(testInitialValues);

  CPPUNIT_TEST(testByIDFiltering);
  CPPUNIT_TEST(testByIDFilteringRelationShipPositive);
  CPPUNIT_TEST(testByIDFilteringRelationShipNegative);

  CPPUNIT_TEST(testByLevelFiltering);
  CPPUNIT_TEST(testByLevelFilteringNegative);

  CPPUNIT_TEST(testBySPFiltering);
  CPPUNIT_TEST(testBySPFilteringNegative);
  CPPUNIT_TEST(testBySPGlobalFiltering);

  CPPUNIT_TEST_SUITE_END();

public:
  void testInitialValues()
  {
    DiagSoloPQFilter filter;
    test::TestPQItemPtr item(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL));
    filter.initialize(0);

    CPPUNIT_ASSERT_EQUAL(SoloPQItem::SVL_NORMAL, filter.getVerbosityLevel());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), filter.getNoOfExpansions());
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(item, 4u));
    CPPUNIT_ASSERT(!filter.isFMPFilteredOut(item.get(), 5u));
  }

  void testByIDFiltering()
  {
    DiagSoloPQFilter filter;
    test::TestPQItemPtr item1(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL));
    test::TestPQItemPtr item2(test::TestPQItem::create(33.0, SoloPQItem::CRC_LEVEL));
    test::TestPQItemPtr item3(test::TestPQItem::create(33.0, SoloPQItem::CRC_LEVEL));

    Diagnostic diag;
    diag.diagParamMap()[DiagSoloPQFilter::PARAM_SOLUTION_IDS] =
        "2*-5"; // solution 2 allowed, other (including 5) disallowed
    filter.initialize(&diag);

    CPPUNIT_ASSERT(!filter.isItemFilteredOut(item1, 2u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(item2, 4u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(item3, 5u));
  }

  void testByIDFilteringRelationShipPositive()
  {
    DiagSoloPQFilter filter;
    test::TestPQItemPtr item1(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL));

    Diagnostic diag;
    diag.diagParamMap()[DiagSoloPQFilter::PARAM_SOLUTION_IDS] =
        "2"; // only 2 and childs are allowed
    filter.initialize(&diag);

    CPPUNIT_ASSERT(!filter.isItemFilteredOut(item1, 2u));

    CPPUNIT_ASSERT(filter.isItemFilteredOut(item1, 4u));

    filter.addIdRelationShip(4u, 2u);

    CPPUNIT_ASSERT(!filter.isItemFilteredOut(item1, 4u));

    filter.addIdRelationShip(6u, 4u);
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(item1, 6u));

    CPPUNIT_ASSERT(filter.isItemFilteredOut(item1, 8u));
  }

  void testByIDFilteringRelationShipNegative()
  {
    DiagSoloPQFilter filter;
    test::TestPQItemPtr item1(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL));

    Diagnostic diag;
    diag.diagParamMap()[DiagSoloPQFilter::PARAM_SOLUTION_IDS] = "-2"; // 2 and child is not allowed
    filter.initialize(&diag);

    CPPUNIT_ASSERT(filter.isItemFilteredOut(item1, 2u));
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(item1, 4u));

    filter.addIdRelationShip(4u, 2u);

    CPPUNIT_ASSERT(filter.isItemFilteredOut(item1, 4u));

    filter.addIdRelationShip(6u, 4u);
    CPPUNIT_ASSERT(filter.isItemFilteredOut(item1, 6u));

    CPPUNIT_ASSERT(!filter.isItemFilteredOut(item1, 8u));
  }

  void testByLevelFiltering()
  {
    DiagSoloPQFilter filter;
    test::TestPQItemPtr crItem(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL));
    test::TestPQItemPtr spItem(test::TestPQItem::create(33.0, SoloPQItem::SP_LEVEL));
    test::TestPQItemPtr crcItem(test::TestPQItem::create(33.0, SoloPQItem::CRC_LEVEL));
    test::TestPQItemPtr crItem2(test::TestPQItem::create(40.0, SoloPQItem::CR_LEVEL));

    Diagnostic diag;
    diag.diagParamMap()[DiagSoloPQFilter::PARAM_LEVEL] = "CRC";
    filter.initialize(&diag);

    CPPUNIT_ASSERT(filter.isItemFilteredOut(crItem, 0u));
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(crcItem, 0u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(spItem, 0u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(crItem2, 3u));

    CPPUNIT_ASSERT(filter.isFMPFilteredOut(crItem.get(), 0u));
  }

  void testByLevelFilteringNegative()
  {
    DiagSoloPQFilter filter;
    test::TestPQItemPtr crItem(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL));
    test::TestPQItemPtr spItem(test::TestPQItem::create(33.0, SoloPQItem::SP_LEVEL));
    test::TestPQItemPtr crcItem(test::TestPQItem::create(33.0, SoloPQItem::CRC_LEVEL));
    test::TestPQItemPtr crItem2(test::TestPQItem::create(40.0, SoloPQItem::CR_LEVEL));

    Diagnostic diag;
    diag.diagParamMap()[DiagSoloPQFilter::PARAM_LEVEL] = "MCRCXMSP"; // i.e. -CRC*-SP
    filter.initialize(&diag);

    CPPUNIT_ASSERT(!filter.isItemFilteredOut(crItem, 0u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(crcItem, 0u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(spItem, 0u));
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(crItem2, 3u));

    CPPUNIT_ASSERT(!filter.isFMPFilteredOut(crItem.get(), 0u));
  }

  void testBySPFiltering()
  {
    DiagSoloPQFilter filter;

    const SolutionPattern& sp20(SolutionPatternStorage::instance().getSPByNumber(20));
    const SolutionPattern& sp30(SolutionPatternStorage::instance().getSPByNumber(30));

    test::TestPQItemPtr crItem30(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL, &sp30));
    test::TestPQItemPtr crItem20(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL, &sp20));
    test::TestPQItemPtr spItem30(test::TestPQItem::create(33.0, SoloPQItem::SP_LEVEL, &sp30));
    test::TestPQItemPtr crcItem30(test::TestPQItem::create(33.0, SoloPQItem::CRC_LEVEL, &sp30));
    test::TestPQItemPtr crItem20a(test::TestPQItem::create(40.0, SoloPQItem::CR_LEVEL, &sp20));

    Diagnostic diag;
    diag.diagParamMap()[DiagSoloPQFilter::PARAM_SOLUTION_PATTERNS] = "30"; // i.e. -CRC*-SP
    filter.initialize(&diag);

    CPPUNIT_ASSERT(!filter.isItemFilteredOut(crItem30, 0u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(crItem20, 0u));
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(spItem30, 0u));
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(crcItem30, 3u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(crItem20a, 3u));

    CPPUNIT_ASSERT(!filter.isFMPFilteredOut(crcItem30.get(), 0u));
    CPPUNIT_ASSERT(filter.isFMPFilteredOut(crItem20.get(), 0u));
  }

  void testBySPFilteringNegative()
  {
    DiagSoloPQFilter filter;

    const SolutionPattern& sp20(SolutionPatternStorage::instance().getSPByNumber(20));
    const SolutionPattern& sp30(SolutionPatternStorage::instance().getSPByNumber(30));

    test::TestPQItemPtr crItem30(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL, &sp30));
    test::TestPQItemPtr crItem20(test::TestPQItem::create(30.0, SoloPQItem::CR_LEVEL, &sp20));
    test::TestPQItemPtr spItem30(test::TestPQItem::create(33.0, SoloPQItem::SP_LEVEL, &sp30));
    test::TestPQItemPtr crcItem30(test::TestPQItem::create(33.0, SoloPQItem::CRC_LEVEL, &sp30));
    test::TestPQItemPtr crItem20a(test::TestPQItem::create(40.0, SoloPQItem::CR_LEVEL, &sp20));

    Diagnostic diag;
    diag.diagParamMap()[DiagSoloPQFilter::PARAM_SOLUTION_PATTERNS] = "-30"; // i.e. -CRC*-SP
    filter.initialize(&diag);

    CPPUNIT_ASSERT(filter.isItemFilteredOut(crItem30, 0u));
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(crItem20, 0u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(spItem30, 0u));
    CPPUNIT_ASSERT(filter.isItemFilteredOut(crcItem30, 3u));
    CPPUNIT_ASSERT(!filter.isItemFilteredOut(crItem20a, 3u));

    CPPUNIT_ASSERT(filter.isFMPFilteredOut(crcItem30.get(), 0u));
    CPPUNIT_ASSERT(!filter.isFMPFilteredOut(crItem20.get(), 0u));
  }

  void testBySPGlobalFiltering()
  {

    const SolutionPattern& sp20(SolutionPatternStorage::instance().getSPByNumber(20));
    const SolutionPattern& sp30(SolutionPatternStorage::instance().getSPByNumber(30));

    test::TestPQItemPtr spItem20(test::TestPQItem::create(30.0, SoloPQItem::SP_LEVEL, &sp20));
    test::TestPQItemPtr spItem30(test::TestPQItem::create(33.0, SoloPQItem::SP_LEVEL, &sp30));

    Diagnostic diag;
    diag.diagParamMap()[DiagSoloPQFilter::PARAM_ALLDIAG_SOLUTION_PATTERNS] = "30"; // i.e. -CRC*-SP

    {
      DiagSoloPQFilter filter;
      filter.initialize(&diag, true);

      CPPUNIT_ASSERT(!filter.isItemFilteredOut(spItem30, 0u));
      CPPUNIT_ASSERT(filter.isItemFilteredOut(spItem20, 0u));
    }

    {
      DiagSoloPQFilter filter;
      filter.initialize(&diag, false); // shouldn't take PARAM_ALLDIAG_SOLUTION_PATTERNS into
                                       // account

      CPPUNIT_ASSERT(!filter.isItemFilteredOut(spItem30, 0u));
      CPPUNIT_ASSERT(!filter.isItemFilteredOut(spItem20, 0u));
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DiagSoloPQFilterTest);
} /* namespace shpq */
} /* namespace tse */
