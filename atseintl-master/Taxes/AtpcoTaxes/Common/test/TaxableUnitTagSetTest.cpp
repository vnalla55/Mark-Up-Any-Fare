// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TaxableUnitTagSet.h"

namespace tax
{

class TaxableUnitTagSetTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxableUnitTagSetTest);
  CPPUNIT_TEST(noFlags);
  CPPUNIT_TEST(allFlags);
  CPPUNIT_TEST(someFlags);
  CPPUNIT_TEST(overlapping);
  CPPUNIT_TEST(overlapping2);
  CPPUNIT_TEST_SUITE_END();

public:
  void noFlags()
  {
    TaxableUnitTagSet s = TaxableUnitTagSet::none();
    CPPUNIT_ASSERT(!s.hasTag(type::TaxableUnit::YqYr));
    CPPUNIT_ASSERT(!s.hasTag(type::TaxableUnit::Itinerary));
    CPPUNIT_ASSERT(!s.hasTag(type::TaxableUnit::BaggageCharge));
    CPPUNIT_ASSERT(!s.hasTag(type::TaxableUnit::TaxOnTax));
    CPPUNIT_ASSERT(s.isEmpty());
    CPPUNIT_ASSERT(!s.hasBit(1));
    CPPUNIT_ASSERT(!s.hasBit(9));
    CPPUNIT_ASSERT(!s.hasBit(7));
    CPPUNIT_ASSERT(!s.hasBit(8));
  }

  void allFlags()
  {
    TaxableUnitTagSet s = TaxableUnitTagSet::all();
    CPPUNIT_ASSERT(s.hasTag(type::TaxableUnit::YqYr));
    CPPUNIT_ASSERT(s.hasTag(type::TaxableUnit::Itinerary));
    CPPUNIT_ASSERT(s.hasTag(type::TaxableUnit::BaggageCharge));
    CPPUNIT_ASSERT(s.hasTag(type::TaxableUnit::TaxOnTax));
    CPPUNIT_ASSERT(!s.isEmpty());
    CPPUNIT_ASSERT(s.hasBit(1));
    CPPUNIT_ASSERT(s.hasBit(9));
    CPPUNIT_ASSERT(s.hasBit(7));
    CPPUNIT_ASSERT(s.hasBit(8));
  }

  void someFlags()
  {
    TaxableUnitTagSet s = TaxableUnitTagSet::none();
    s.setTag(type::TaxableUnit::BaggageCharge);
    s.setTag(type::TaxableUnit::OCFlightRelated);

    CPPUNIT_ASSERT(!s.hasTag(type::TaxableUnit::YqYr));
    CPPUNIT_ASSERT(!s.hasTag(type::TaxableUnit::Itinerary));
    CPPUNIT_ASSERT(s.hasTag(type::TaxableUnit::BaggageCharge));
    CPPUNIT_ASSERT(!s.hasTag(type::TaxableUnit::TaxOnTax));
    CPPUNIT_ASSERT(s.hasTag(type::TaxableUnit::OCFlightRelated));
    CPPUNIT_ASSERT(!s.isEmpty());
    CPPUNIT_ASSERT(!s.hasBit(1));
    CPPUNIT_ASSERT(!s.hasBit(9));
    CPPUNIT_ASSERT(s.hasBit(7));
    CPPUNIT_ASSERT(!s.hasBit(8));
  }

  void overlapping()
  {
    TaxableUnitTagSet sNone = TaxableUnitTagSet::none();

    TaxableUnitTagSet sFare = TaxableUnitTagSet::none();
    sFare.setTag(type::TaxableUnit::Itinerary);

    TaxableUnitTagSet sOC = TaxableUnitTagSet::none();
    sOC.setTag(type::TaxableUnit::OCFlightRelated);
    sOC.setTag(type::TaxableUnit::OCTicketRelated);
    sOC.setTag(type::TaxableUnit::OCFareRelated);
    sOC.setTag(type::TaxableUnit::BaggageCharge);

    TaxableUnitTagSet sAll = TaxableUnitTagSet::all();

    CPPUNIT_ASSERT(!overlap(sNone, sNone));
    CPPUNIT_ASSERT(!overlap(sNone, sFare));
    CPPUNIT_ASSERT(!overlap(sNone, sOC));
    CPPUNIT_ASSERT(!overlap(sNone, sAll));
    CPPUNIT_ASSERT(!overlap(sFare, sNone));
    CPPUNIT_ASSERT(!overlap(sOC, sNone));
    CPPUNIT_ASSERT(!overlap(sAll, sNone));

    CPPUNIT_ASSERT( overlap(sAll, sAll));
    CPPUNIT_ASSERT( overlap(sAll, sFare));
    CPPUNIT_ASSERT( overlap(sAll, sOC));
    CPPUNIT_ASSERT( overlap(sFare, sAll));
    CPPUNIT_ASSERT( overlap(sOC, sAll));

    CPPUNIT_ASSERT( overlap(sOC, sOC));
    CPPUNIT_ASSERT( overlap(sFare, sFare));

    CPPUNIT_ASSERT(!overlap(sOC, sFare));
    CPPUNIT_ASSERT(!overlap(sFare, sOC));
  }

  void overlapping2()
  {
    TaxableUnitTagSet s12 = TaxableUnitTagSet::none();
    TaxableUnitTagSet s23 = TaxableUnitTagSet::none();
    TaxableUnitTagSet s34 = TaxableUnitTagSet::none();

    s12.setTag(type::TaxableUnit::YqYr);
    s12.setTag(type::TaxableUnit::TicketingFee);
    s23.setTag(type::TaxableUnit::TicketingFee);
    s23.setTag(type::TaxableUnit::OCFlightRelated);
    s34.setTag(type::TaxableUnit::OCFlightRelated);
    s34.setTag(type::TaxableUnit::OCTicketRelated);

    CPPUNIT_ASSERT( overlap(s12, s23));
    CPPUNIT_ASSERT( overlap(s23, s12));

    CPPUNIT_ASSERT( overlap(s34, s23));
    CPPUNIT_ASSERT( overlap(s23, s34));

    CPPUNIT_ASSERT(!overlap(s12, s34));
    CPPUNIT_ASSERT(!overlap(s34, s12));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxableUnitTagSetTest);

} // namespace tax

