//-------------------------------------------------------------------
//
//  File:        BrandProgramTest.cpp
//  Created:     2015
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------


#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

#include "BrandedFares/BrandProgram.h"

#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"

#include <string>

namespace tse
{

class BrandProgramTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandProgramTest);
  CPPUNIT_TEST(testDirectionalityBothwaysFmSLProgNB);
  CPPUNIT_TEST(testDirectionalityBothwaysFmSLProgBN);
  CPPUNIT_TEST(testDirectionalityBothwaysFmSLProgBoth);
  CPPUNIT_TEST(testDirectionalityOriginalFmSLProgNB);
  CPPUNIT_TEST(testDirectionalityOriginalFmSLProgBN);
  CPPUNIT_TEST(testDirectionalityOriginalFmSLProgBoth);
  CPPUNIT_TEST(testDirectionalityReversedFmSLProgNB);
  CPPUNIT_TEST(testDirectionalityReversedFmSLProgBN);
  CPPUNIT_TEST(testDirectionalityReversedFmSLProgBoth);
  CPPUNIT_TEST(testDirectionalityNoBrands);
  CPPUNIT_TEST_SUITE_END();

private:
  Loc* _locNyc;
  Loc* _locLax;
  Loc* _locSyd;
  Loc* _locBri;
  FareMarket* _fm;
  TestMemHandle _memHandle;
  BrandProgram* _bProgram;


public:
  void setUp()
  {
    _locLax = createLoc("LAX");
    _locSyd = createLoc("SYD");
    _locNyc = createLoc("NYC");
    _locBri = createLoc("BRI");
    _bProgram = _memHandle.create<BrandProgram>();
  }

  Loc* createLoc(const std::string& location)
  {
    Loc* loc = _memHandle.create<Loc>();
    loc->loc() = location;
    return loc;
  }

  FareMarket* createFareMarket(const Loc* origin, const Loc* destination)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->origin() = origin;
    fm->destination() = destination;
    fm->brandProgramIndexVec().push_back(0);
    return fm;
  }

  // General concept of the tests
  // LEG   : NYC -- LAX -- SYD -- BRI
  // FM    :        LAX -- SYD
  // O&D   : NYC ---------------- BRI (or reversed, or both)

  void testDirectionalityBothwaysFmSLProgNB()
  {
    _fm = createFareMarket(_locSyd, _locLax);
    // FM brand/program request = BOTHWAYS
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addOriginRequestedForOriginalDirection(_locNyc->loc());
    _fm->addDestinationRequestedForReversedDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locNyc->loc());
    _bProgram->destinationsRequested().insert(_locBri->loc());

    Direction direction = Direction::BOTHWAYS;

    CPPUNIT_ASSERT(_bProgram->calculateDirectionality(*_fm, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::ORIGINAL, direction);
  }

  void testDirectionalityBothwaysFmSLProgBN()
  {
    _fm = createFareMarket(_locSyd, _locLax);
    // FM brand/program request = BOTHWAYS
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addOriginRequestedForOriginalDirection(_locNyc->loc());
    _fm->addDestinationRequestedForReversedDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locBri->loc());
    _bProgram->destinationsRequested().insert(_locNyc->loc());

    Direction direction = Direction::BOTHWAYS;

    CPPUNIT_ASSERT(_bProgram->calculateDirectionality(*_fm, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::REVERSED, direction);
  }

  void testDirectionalityBothwaysFmSLProgBoth()
  {
    _fm = createFareMarket(_locSyd, _locLax);
    // FM brand/program request = BOTHWAYS
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addOriginRequestedForOriginalDirection(_locNyc->loc());
    _fm->addDestinationRequestedForReversedDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locBri->loc());
    _bProgram->originsRequested().insert(_locNyc->loc());
    _bProgram->destinationsRequested().insert(_locBri->loc());
    _bProgram->destinationsRequested().insert(_locNyc->loc());

    Direction direction = Direction::ORIGINAL;

    CPPUNIT_ASSERT(_bProgram->calculateDirectionality(*_fm, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testDirectionalityOriginalFmSLProgNB()
  {
    _fm = createFareMarket(_locLax, _locSyd);
    // FM brand/program request = ORIGINAL
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addOriginRequestedForOriginalDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locNyc->loc());
    _bProgram->destinationsRequested().insert(_locBri->loc());;

    Direction direction = Direction::BOTHWAYS;

    CPPUNIT_ASSERT(_bProgram->calculateDirectionality(*_fm, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::ORIGINAL, direction);
  }

  void testDirectionalityOriginalFmSLProgBN()
  {
    _fm = createFareMarket(_locLax, _locSyd);
    // FM brand/program request = ORIGINAL
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addOriginRequestedForOriginalDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locBri->loc());
    _bProgram->destinationsRequested().insert(_locNyc->loc());

    Direction direction = Direction::BOTHWAYS;
    // improper case, we ask NYC-BRI and receive program for BRI-NYC
    CPPUNIT_ASSERT(!_bProgram->calculateDirectionality(*_fm, direction));
  }

  void testDirectionalityOriginalFmSLProgBoth()
  {
    _fm = createFareMarket(_locLax, _locSyd);
    // FM brand/program request = ORIGINAL
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addOriginRequestedForOriginalDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locBri->loc());
    _bProgram->originsRequested().insert(_locNyc->loc());
    _bProgram->destinationsRequested().insert(_locBri->loc());
    _bProgram->destinationsRequested().insert(_locNyc->loc());

    Direction direction = Direction::BOTHWAYS;

    CPPUNIT_ASSERT(_bProgram->calculateDirectionality(*_fm, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::ORIGINAL, direction);
  }

  void testDirectionalityReversedFmSLProgNB()
  {
    _fm = createFareMarket(_locLax, _locSyd);
    // FM brand/program request = REVERSED
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addDestinationRequestedForReversedDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locNyc->loc());
    _bProgram->destinationsRequested().insert(_locBri->loc());;

    Direction direction = Direction::BOTHWAYS;
    // improper case, we ask BRI-NYC and receive program for NYC-BRI
    CPPUNIT_ASSERT(!_bProgram->calculateDirectionality(*_fm, direction));
  }

  void testDirectionalityReversedFmSLProgBN()
  {
    _fm = createFareMarket(_locLax, _locSyd);
    // FM brand/program request = REVERSED
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addDestinationRequestedForReversedDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locBri->loc());
    _bProgram->destinationsRequested().insert(_locNyc->loc());

    Direction direction = Direction::BOTHWAYS;

    CPPUNIT_ASSERT(_bProgram->calculateDirectionality(*_fm, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::REVERSED, direction);
  }

  void testDirectionalityReversedFmSLProgBoth()
  {
    _fm = createFareMarket(_locLax, _locSyd);
    // FM brand/program request = REVERSED
    // O&D for original NYC - BRI, for reversed BRI - NYC
    _fm->addDestinationRequestedForReversedDirection(_locNyc->loc());

    _bProgram->originsRequested().clear();
    _bProgram->destinationsRequested().clear();
    _bProgram->originsRequested().insert(_locBri->loc());
    _bProgram->originsRequested().insert(_locNyc->loc());
    _bProgram->destinationsRequested().insert(_locBri->loc());
    _bProgram->destinationsRequested().insert(_locNyc->loc());

    Direction direction = Direction::BOTHWAYS;

    CPPUNIT_ASSERT(_bProgram->calculateDirectionality(*_fm, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::REVERSED, direction);
  }

  void testDirectionalityNoBrands()
  {
    _fm = createFareMarket(_locLax, _locSyd);
    _fm->brandProgramIndexVec().clear();

    Direction direction = Direction::ORIGINAL;

    CPPUNIT_ASSERT(_bProgram->calculateDirectionality(*_fm, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(BrandProgramTest);
}
