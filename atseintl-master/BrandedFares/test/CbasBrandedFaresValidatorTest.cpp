//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelectorTest.cpp
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "BrandedFares/test/S8BrandedFaresSelectorInitializer.h"
#include "BrandedFares/CbasBrandedFaresSelector.h"

using namespace std;
namespace tse
{
class CbasBrandedFaresValidatorTest : public CppUnit::TestFixture, S8BrandedFaresSelectorInitializer<CbasBrandedFareValidator>
{
  CPPUNIT_TEST_SUITE(CbasBrandedFaresValidatorTest);
  CPPUNIT_TEST(testisFamilyFareBasisCode);
  CPPUNIT_TEST(testisMatchingNonExactFareBasis);
  CPPUNIT_TEST(testisMatchedFareBasisCode);
  CPPUNIT_TEST(testhandleFareBasisCodeWorkaround);
  CPPUNIT_TEST(testmatchBasedOnFareBasisCode);
  CPPUNIT_TEST(testisBookingCodeMatched);
  CPPUNIT_TEST(testmatchBasedOnBookingCode);
  CPPUNIT_TEST(testvalidateFare);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    init();
    _s8BrandedFaresSelector = _memHandle.create<CbasBrandedFareValidator>(*_trx);
    _fdS8BrandedFaresSelector = _memHandle.create<CbasBrandedFareValidator>(*_fdTrx);
  }

  void tearDown() { _memHandle.clear(); }

  void testisFamilyFareBasisCode()
  {
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isFamilyFareBasisCode(""));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isFamilyFareBasisCode("-"));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isFamilyFareBasisCode("-AA"));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isFamilyFareBasisCode("AAY-"));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isFamilyFareBasisCode("AA"));
  }

  void testisMatchingNonExactFareBasis()
  {
    Diag889Collector* diag = NULL;
    BrandedFareDiagnostics diagnostics(*_trx, diag);
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("HLE70", "H-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("BE70HNR", "H-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("BHE70", "B-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("BE70", "B-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("BHE70NR", "B-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("BXE170", "B-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("BE70", "-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("BHE70", "-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("E70", "-E70", diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("YM", "-M", diagnostics));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isMatchingNonExactFareBasis("M", "-M", diagnostics));
  }

  void testisMatchedFareBasisCode()
  {
    Diag889Collector* diag = NULL;
    BrandedFareDiagnostics diagnostics(*_trx, diag);
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC"), FareBasisCode("FBC"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC"), FareBasisCode("FBA"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC"), FareBasisCode("FBC/TKT"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC/TKT"), FareBasisCode("FBC"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC/TKT"), FareBasisCode("/TKT"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC/TKT"), FareBasisCode("FBC/TKT"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC/TKT"), FareBasisCode("/TKT"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC/TKT"), FareBasisCode("F-/TKT"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC/TKT"), FareBasisCode("E-/TKT"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC/TKT"), FareBasisCode("F-"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC"), FareBasisCode("F-"), diagnostics));
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isMatchedFareBasisCode(FareBasisCodeStructure("FBC"), FareBasisCode("E-"), diagnostics));
  }

  void testmatchBasedOnFareBasisCode()
  {
    FareBasisCodeStructure ptfFareBasisCode("FBC/TKT");
    Diag889Collector* diag = NULL;
    BrandedFareDiagnostics diagnostics(*_trx, diag);
    std::vector<FareBasisCode> includedFareBasisCodes;
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->matchBasedOnFareBasisCode(
                           ptfFareBasisCode, includedFareBasisCodes, diagnostics));

    includedFareBasisCodes = {"AAA/TKT", "FBC/TKT", "BBB/TKT"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->matchBasedOnFareBasisCode(
                           ptfFareBasisCode, includedFareBasisCodes, diagnostics));

    includedFareBasisCodes[1] = "EBC/TKT";
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_FAIL, _s8BrandedFaresSelector->matchBasedOnFareBasisCode(
                           ptfFareBasisCode, includedFareBasisCodes, diagnostics));
  }

  void testhandleFareBasisCodeWorkaround()
  {
    BrandInfo brand;
    brand.primaryBookingCode() = {"A"};
    brand.secondaryBookingCode() = {"C", "D"};
    brand.includedFareBasisCode() = {"EE", "FF", "GG"};

    std::vector<BookingCode> primaryBookingCodes, secondaryBookingCodes;
    std::vector<FareBasisCode> fareBasisCodes;

    _s8BrandedFaresSelector->handleFareBasisCodeWorkaround(brand, primaryBookingCodes, secondaryBookingCodes, fareBasisCodes);

    CPPUNIT_ASSERT(primaryBookingCodes.size() == 1);
    CPPUNIT_ASSERT(secondaryBookingCodes.size() == 2);
    CPPUNIT_ASSERT(fareBasisCodes.size() == 3);

    brand.primaryBookingCode().clear();
    primaryBookingCodes.clear();
    secondaryBookingCodes.clear();
    fareBasisCodes.clear();

    _s8BrandedFaresSelector->handleFareBasisCodeWorkaround(brand, primaryBookingCodes, secondaryBookingCodes, fareBasisCodes);

    CPPUNIT_ASSERT(primaryBookingCodes.size() == 3);
    CPPUNIT_ASSERT(secondaryBookingCodes.size() == 2);
    CPPUNIT_ASSERT(fareBasisCodes.size() == 0);

    primaryBookingCodes.clear();
    secondaryBookingCodes.clear();
    fareBasisCodes.clear();

    brand.includedFareBasisCode() = {"EE", "-FF", "GG"};

    _s8BrandedFaresSelector->handleFareBasisCodeWorkaround(brand, primaryBookingCodes, secondaryBookingCodes, fareBasisCodes);

    CPPUNIT_ASSERT(primaryBookingCodes.size() == 0);
    CPPUNIT_ASSERT(secondaryBookingCodes.size() == 0);
    CPPUNIT_ASSERT(fareBasisCodes.size() == 3);

    brand.includedFareBasisCode() = {"EE", "3FF", "GG"};
    primaryBookingCodes.clear();
    secondaryBookingCodes.clear();
    fareBasisCodes.clear();

    _s8BrandedFaresSelector->handleFareBasisCodeWorkaround(brand, primaryBookingCodes, secondaryBookingCodes, fareBasisCodes);

    CPPUNIT_ASSERT(primaryBookingCodes.size() == 0);
    CPPUNIT_ASSERT(secondaryBookingCodes.size() == 0);
    CPPUNIT_ASSERT(fareBasisCodes.size() == 3);
  }

  void testisBookingCodeMatched()
  {
    Diag889Collector* diag = NULL;
    BrandedFareDiagnostics diagnostics(*_trx, diag);
    std::vector<BookingCode> bookingCodes = {"A", "B", "C"};
    std::vector<BookingCode> matchingCodes;
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isBookingCodeMatched(bookingCodes, matchingCodes, diagnostics));

    matchingCodes = {"C", "D", "E"};
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isBookingCodeMatched(bookingCodes, matchingCodes, diagnostics));

    matchingCodes = {"", "D", "E"};
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isBookingCodeMatched(bookingCodes, matchingCodes, diagnostics));

    matchingCodes = {" ", "D", "E"};
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isBookingCodeMatched(bookingCodes, matchingCodes, diagnostics));

    matchingCodes = {"D", "E"};
    CPPUNIT_ASSERT_EQUAL(false, _s8BrandedFaresSelector->isBookingCodeMatched(bookingCodes, matchingCodes, diagnostics));

    matchingCodes = {"DG", "FH"};
    bookingCodes = {"EB", "DB"};
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isBookingCodeMatched(bookingCodes, matchingCodes, diagnostics));

    matchingCodes = {"XX", "*"};
    CPPUNIT_ASSERT_EQUAL(true, _s8BrandedFaresSelector->isBookingCodeMatched(bookingCodes, matchingCodes, diagnostics));
  }

  void testmatchBasedOnBookingCode()
  {
    std::vector<BookingCode> bookingCodes = {"A", "B", "C"};
    std::vector<BookingCode> primaryBookingCodes;
    std::vector<BookingCode> secondarybookingCodes;
    Diag889Collector* diag = NULL;
    BrandedFareDiagnostics diagnostics(*_trx, diag);
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->matchBasedOnBookingCode(bookingCodes, primaryBookingCodes, secondarybookingCodes, diagnostics));

    primaryBookingCodes = {"C", "D", "E"};
    secondarybookingCodes = {"F", "G", "H"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->matchBasedOnBookingCode(bookingCodes, primaryBookingCodes, secondarybookingCodes, diagnostics));

    primaryBookingCodes = {"D", "E"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_FAIL, _s8BrandedFaresSelector->matchBasedOnBookingCode(bookingCodes, primaryBookingCodes, secondarybookingCodes, diagnostics));

    secondarybookingCodes = {"C", "G", "H"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_SOFT_PASS, _s8BrandedFaresSelector->matchBasedOnBookingCode(bookingCodes, primaryBookingCodes, secondarybookingCodes, diagnostics));

    primaryBookingCodes = {"C", "G", "H"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->matchBasedOnBookingCode(bookingCodes, primaryBookingCodes, secondarybookingCodes, diagnostics));
  }

  void testvalidateFare()
  {
    BrandInfo brand;
    Diag889Collector* diag = NULL;
    BrandedFareDiagnostics diagnostics(*_trx, diag);
    FareBasisCodeStructure ptfFareBasisCode("FBC");
    std::vector<BookingCode> bookingCodes = {"A", "B", "C"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->validateFare(ptfFareBasisCode, bookingCodes, brand, diagnostics));

    brand.primaryBookingCode() = {"A"};

    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->validateFare(ptfFareBasisCode, bookingCodes, brand, diagnostics));

    brand.primaryBookingCode() = {"B"};
    brand.secondaryBookingCode() = {"R"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->validateFare(ptfFareBasisCode, bookingCodes, brand, diagnostics));

    bookingCodes = {"A", "C"};
    brand.excludedFareBasisCode() = {"FBC"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_FAIL, _s8BrandedFaresSelector->validateFare(ptfFareBasisCode, bookingCodes, brand, diagnostics));

    bookingCodes = {"A", "R"};
    brand.excludedFareBasisCode() = {"FBA"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_SOFT_PASS, _s8BrandedFaresSelector->validateFare(ptfFareBasisCode, bookingCodes, brand, diagnostics));

    brand.primaryBookingCode().clear();
    brand.secondaryBookingCode().clear();
    brand.includedFareBasisCode() = {"FBC"};

    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->validateFare(ptfFareBasisCode, bookingCodes, brand, diagnostics));

    brand.excludedFareBasisCode() = {"F-"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_FAIL, _s8BrandedFaresSelector->validateFare(ptfFareBasisCode, bookingCodes, brand, diagnostics));

    brand.excludedFareBasisCode() = {"G-"};
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_HARD_PASS, _s8BrandedFaresSelector->validateFare(ptfFareBasisCode, bookingCodes, brand, diagnostics));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CbasBrandedFaresValidatorTest);
}
