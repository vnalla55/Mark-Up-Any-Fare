// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include <memory>
#include <sstream>
#include <stdexcept>

#include "Rules/TimeStopoverChecker.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tax
{

class TimeStopoverCheckerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TimeStopoverCheckerTest);

  CPPUNIT_TEST(testSameDayStopoverChecker_conx);
  CPPUNIT_TEST(testSameDayStopoverChecker_sameDateStop);
  CPPUNIT_TEST(testSameDayStopoverChecker_differentDateStop);
  CPPUNIT_TEST(testDateStopoverChecker_zero_stop);
  CPPUNIT_TEST(testDateStopoverChecker_zero_conx);
  CPPUNIT_TEST(testDateStopoverChecker_2D_stop);
  CPPUNIT_TEST(testDateStopoverChecker_2D_conx);
  CPPUNIT_TEST(testSpecificTimeStopoverChecker);
  CPPUNIT_TEST(testMonthsStopoverChecker);
  CPPUNIT_TEST(testMonthsStopoverCheckerSnapToEndOfMonth);
  CPPUNIT_TEST(testMonthsStopoverCheckerShorterMonth);
  CPPUNIT_TEST(testDomesticTimeStopoverChecker_IntlTravel);
  CPPUNIT_TEST(testDomesticTimeStopoverChecker_USDomesticTravel);
  CPPUNIT_TEST(testDomesticTimeStopoverChecker_CADomesticTravel);
  CPPUNIT_TEST(testDomesticTimeStopoverChecker_OtherDomesticTravel);
  CPPUNIT_TEST(testSpecialDomesticTimeStopoverChecker_before17_lessorequal6hours_conx);
  CPPUNIT_TEST(testSpecialDomesticTimeStopoverChecker_before17_greaterthan6hours_stop);
  CPPUNIT_TEST(testSpecialDomesticTimeStopoverChecker_afterorequal17_sameDay_conx);
  CPPUNIT_TEST(testSpecialDomesticTimeStopoverChecker_afterorequal17_nextDay_beforeorequal10_conx);
  CPPUNIT_TEST(testSpecialDomesticTimeStopoverChecker_afterorequal17_nextDay_after10_stop);
  CPPUNIT_TEST(testSpecialDomesticTimeStopoverChecker_afterorequal17_twoDays_stop);

  CPPUNIT_TEST_SUITE_END();

private:
  std::unique_ptr<Flight> _fl1;
  std::unique_ptr<Flight> _fl2;
  std::unique_ptr<FlightUsage> _fu1;
  std::unique_ptr<FlightUsage> _fu2;
  std::unique_ptr<Itin> _itin;
  std::unique_ptr<GeoPath> _geoPath;

public:
  void setUp()
  {
    _fl1.reset(new Flight());
    _fl2.reset(new Flight());
    _fu1.reset(new FlightUsage());
    _fu2.reset(new FlightUsage());
    _fu1->flight() = _fl1.get();
    _fu1->setId(0);
    _fu2->flight() = _fl2.get();
    _fu2->setId(1);
    _itin.reset(new Itin());
    _geoPath.reset(new GeoPath());
    _itin->geoPath() = _geoPath.get();
  }

  void tearDown()
  {
  }

  void addGeo(const type::Nation& nation)
  {
    Geo geo;
    geo.loc().nation() = nation;
    _geoPath->geos().push_back(geo);
  }

  void testSameDayStopoverChecker_conx()
  {
    SameDayStopoverChecker tc {6};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(5, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(8, 00);
    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));

    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
  }

  void testSameDayStopoverChecker_sameDateStop()
  {
    SameDayStopoverChecker tc {2};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(5, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(8, 00);
    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));

    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testSameDayStopoverChecker_differentDateStop()
  {
    SameDayStopoverChecker tc {6};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(5, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(8, 00);
    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 7));

    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testDateStopoverChecker_zero_stop()
  {
    DateStopoverChecker tc {0};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(5, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(8, 00);
    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 6));

    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testDateStopoverChecker_zero_conx()
  {
    DateStopoverChecker tc {0};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(5, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(8, 00);
    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));

    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
  }

  void testDateStopoverChecker_2D_stop()
  {
    DateStopoverChecker tc {2};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(5, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(8, 00);
    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 8));

    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testDateStopoverChecker_2D_conx()
  {
    DateStopoverChecker tc {2};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(5, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(8, 00);
    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 7));

    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
  }

  void testSpecificTimeStopoverChecker()
  {
    SpecificTimeStopoverChecker tc {200};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(5, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));
    _fl2->departureTime() = type::Time(6, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 20);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 21);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(9, 00);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(6, 00);
    _fu2->markDepartureDate(type::Date(2013, 6, 6));
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testMonthsStopoverChecker()
  {
    MonthsStopoverChecker tc {2};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(8, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 8, 5));
    _fl2->departureTime() = type::Time(6, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 01);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(22, 00);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(22, 00);
    _fu2->markDepartureDate(type::Date(2013, 8, 4));
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(4, 00);
    _fu2->markDepartureDate(type::Date(2013, 8, 6));
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testMonthsStopoverCheckerShorterMonth()
  {
    MonthsStopoverChecker tc {2};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(8, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2012, 12, 30));

    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 2, 28));
    _fl2->departureTime() = type::Time(6, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 01);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(22, 00);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(22, 00);
    _fu2->markDepartureDate(type::Date(2013, 2, 27));
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(4, 00);
    _fu2->markDepartureDate(type::Date(2013, 3, 1));
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testMonthsStopoverCheckerSnapToEndOfMonth()
  {
    MonthsStopoverChecker tc {2};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(8, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 2, 28));

    _fl2->arrivalTime() = type::Time(10, 00);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 4, 30));
    _fl2->departureTime() = type::Time(6, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(8, 01);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(22, 00);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(22, 00);
    _fu2->markDepartureDate(type::Date(2013, 4, 29));
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(4, 00);
    _fu2->markDepartureDate(type::Date(2013, 5, 1));
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  // DomesticTimeStopoverChecker tests

  void testDomesticTimeStopoverChecker_IntlTravel()
  {
    addGeo("US");
    addGeo("CA");

    DomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(16, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(21, 00);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(16, 50);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
  }

  void testDomesticTimeStopoverChecker_USDomesticTravel()
  {
    addGeo("US");
    addGeo("US");

    DomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(16, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(21, 00);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(16, 50);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
  }

  void testDomesticTimeStopoverChecker_CADomesticTravel()
  {
    addGeo("CA");
    addGeo("CA");

    DomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(16, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(20, 00);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));

    _fl2->departureTime() = type::Time(20, 01);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
    _fl2->departureTime() = type::Time(16, 50);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
    _fl2->departureTime() = type::Time(23, 50);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testDomesticTimeStopoverChecker_OtherDomesticTravel()
  {
    addGeo("DE");
    addGeo("DE");

    DomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(16, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(22, 00);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
    _fl2->departureTime() = type::Time(22, 01);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
    _fl2->departureTime() = type::Time(16, 50);
    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
    _fl2->departureTime() = type::Time(23, 50);
    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  // SpecialDomesticTimeStopoverChecker tests

  void testSpecialDomesticTimeStopoverChecker_before17_greaterthan6hours_stop()
  {
    addGeo("DE");
    addGeo("GB");
    addGeo("GB");
    addGeo("GB");

    SpecialDomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 0);
    _fl1->arrivalTime() = type::Time(16, 59);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(23, 0);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));

    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testSpecialDomesticTimeStopoverChecker_before17_lessorequal6hours_conx()
  {
    addGeo("DE");
    addGeo("GB");
    addGeo("GB");
    addGeo("GB");

    SpecialDomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 0);
    _fl1->arrivalTime() = type::Time(16, 59);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(22, 59);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));

    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
  }

  void testSpecialDomesticTimeStopoverChecker_afterorequal17_sameDay_conx()
  {
    addGeo("DE");
    addGeo("GB");
    addGeo("GB");
    addGeo("GB");

    SpecialDomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(17, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(23, 01);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 5));

    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
  }

  void testSpecialDomesticTimeStopoverChecker_afterorequal17_nextDay_beforeorequal10_conx()
  {
    addGeo("DE");
    addGeo("GB");
    addGeo("GB");
    addGeo("GB");

    SpecialDomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(17, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(10, 00);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 6));

    CPPUNIT_ASSERT_EQUAL(false, tc.isStopover(*_fu1, *_fu2));
  }

  void testSpecialDomesticTimeStopoverChecker_afterorequal17_nextDay_after10_stop()
  {
    addGeo("DE");
    addGeo("GB");
    addGeo("GB");
    addGeo("GB");

    SpecialDomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(17, 00);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(10, 01);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 6));

    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }

  void testSpecialDomesticTimeStopoverChecker_afterorequal17_twoDays_stop()
  {
    SpecialDomesticTimeStopoverChecker tc {*_itin};

    _fl1->departureTime() = type::Time(3, 00);
    _fl1->arrivalTime() = type::Time(17, 01);
    _fl1->arrivalDateShift() = 0;
    _fu1->markDepartureDate(type::Date(2013, 6, 5));

    _fl2->departureTime() = type::Time(9, 00);
    _fl2->arrivalTime() = type::Time(23, 40);
    _fl2->arrivalDateShift() = 0;
    _fu2->markDepartureDate(type::Date(2013, 6, 7));

    CPPUNIT_ASSERT_EQUAL(true, tc.isStopover(*_fu1, *_fu2));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TimeStopoverCheckerTest);

}
