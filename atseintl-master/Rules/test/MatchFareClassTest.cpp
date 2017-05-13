#include "test/include/CppUnitHelperMacros.h"
#include "Common/MatchFareClass.h"

namespace tse
{
class MatchFareClassTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(MatchFareClassTest);
  CPPUNIT_TEST(testMatchFareClassHyphenAtTheBeginning);
  CPPUNIT_TEST(testMatchFareClassHyphenInTheMiddle);
  CPPUNIT_TEST(testMatchFareClassHyphenAtTheEnd);
  CPPUNIT_TEST(testMatchFareClassEmptyRule);
  CPPUNIT_TEST(testMatchFareClassNoHyphenMatch);
  CPPUNIT_TEST(testMatchFareClassNoHyphenNoMatch);
  CPPUNIT_TEST_SUITE_END();

  void testMatchFareClassHyphenAtTheBeginning()
  {
    //-E70 matches BHE70NR but does not match BHE701
    CPPUNIT_ASSERT(matchFareClassN("-E70", "BHE70NR"));
    CPPUNIT_ASSERT(!matchFareClassN("-E70", "BHE701"));
    //-E70 will match to BE70 but not to E70
    CPPUNIT_ASSERT(matchFareClassN("-E70", "BE70"));
    CPPUNIT_ASSERT(!matchFareClassN("-E70", "E70"));
    //-E7K matches BHE7KNR and BHE7K1
    CPPUNIT_ASSERT(matchFareClassN("-E7K", "BHE7KNR"));
    CPPUNIT_ASSERT(matchFareClassN("-E7K", "BHE7K1"));
    //-YE will match to AYE, B7YE, and ABCYE, but does not match YE
    CPPUNIT_ASSERT(matchFareClassN("-YE", "AYE"));
    CPPUNIT_ASSERT(matchFareClassN("-YE", "B7YE"));
    CPPUNIT_ASSERT(matchFareClassN("-YE", "ABCYE"));
    CPPUNIT_ASSERT(!matchFareClassN("-YE", "YE"));
    CPPUNIT_ASSERT(matchFareClassN("-1", "SLX13IQ1"));
    CPPUNIT_ASSERT(matchFareClassN("-1", "DNE01AQ1"));
    CPPUNIT_ASSERT(!matchFareClassN("-1", "DNE01A21"));
    CPPUNIT_ASSERT(matchFareClassN("-E70", "BE70"));
    CPPUNIT_ASSERT(matchFareClassN("-E70", "BHE70"));
    CPPUNIT_ASSERT(!matchFareClassN("-E70", "E70"));
    CPPUNIT_ASSERT(matchFareClassN("-M", "YM"));
    CPPUNIT_ASSERT(!matchFareClassN("-M", "M"));
    CPPUNIT_ASSERT(matchFareClassN("-E70", "BE70"));
    CPPUNIT_ASSERT(matchFareClassN("-E70", "YNWE70"));
    CPPUNIT_ASSERT(matchFareClassN("-E70", "Q123E70R"));
    CPPUNIT_ASSERT(!matchFareClassN("-HE70", "HE70NR"));
    CPPUNIT_ASSERT(matchFareClassN("-E70", "BE70NR"));
  }

  void testMatchFareClassHyphenInTheMiddle()
  {
    // Y-E matches YHE, YHXE, Y8E, YLXE1A, and YE
    CPPUNIT_ASSERT(matchFareClassN("Y-E", "YHE"));
    CPPUNIT_ASSERT(matchFareClassN("Y-E", "YHXE"));
    CPPUNIT_ASSERT(matchFareClassN("Y-E", "Y8E"));
    CPPUNIT_ASSERT(matchFareClassN("Y-E", "YLXE1A"));
    CPPUNIT_ASSERT(matchFareClassN("Y-E", "YE"));
    // Y-8 matches YH8. It does not match Y18 or YH89
    CPPUNIT_ASSERT(matchFareClassN("Y-8", "YH8"));
    CPPUNIT_ASSERT(!matchFareClassN("Y-8", "Y18"));
    CPPUNIT_ASSERT(!matchFareClassN("Y-8", "YH89"));
    // fare family H-EE matches to HLEE1M, HKEE, and HAPEE3M
    CPPUNIT_ASSERT(matchFareClassN("H-EE", "HLEE1M"));
    CPPUNIT_ASSERT(matchFareClassN("H-EE", "HKEE"));
    CPPUNIT_ASSERT(matchFareClassN("H-EE", "HAPEE3M"));
    CPPUNIT_ASSERT(matchFareClassN("H-E70", "HLE70"));
    CPPUNIT_ASSERT(!matchFareClassN("H-E70", "BE70HNR"));
    CPPUNIT_ASSERT(matchFareClassN("B-E70", "BHE70"));
    CPPUNIT_ASSERT(matchFareClassN("B-E70", "BE70"));
    CPPUNIT_ASSERT(matchFareClassN("B-E70", "BHE70NR"));
    CPPUNIT_ASSERT(!matchFareClassN("B-E70", "BXE170"));
    CPPUNIT_ASSERT(matchFareClassN("GL-3", "GL3BRTH"));
    CPPUNIT_ASSERT(!matchFareClassN("S0MS-BR", "QW0ACA"));
    CPPUNIT_ASSERT(matchFareClassN("BE-70", "BEX70"));
    CPPUNIT_ASSERT(matchFareClassN("BE-70", "BE1X70"));
    CPPUNIT_ASSERT(!matchFareClassN("BE-70", "BEX170"));
    CPPUNIT_ASSERT(!matchFareClassN("B-E70", "BE701"));
    CPPUNIT_ASSERT(!matchFareClassN("B-E70", "BE710"));
    CPPUNIT_ASSERT(!matchFareClassN("B-E70", "BE170"));
  }

  void testMatchFareClassHyphenAtTheEnd()
  {
    CPPUNIT_ASSERT(matchFareClassN("M-", "M"));
    // Y8- matches Y8E, but does not match Y89, or Y80E
    CPPUNIT_ASSERT(matchFareClassN("Y8-", "Y8E"));
    CPPUNIT_ASSERT(!matchFareClassN("Y8-", "Y89"));
    CPPUNIT_ASSERT(!matchFareClassN("Y8-", "Y80E"));
    CPPUNIT_ASSERT(matchFareClassN("YA7-", "YA7"));
    CPPUNIT_ASSERT(matchFareClassN("YA7-", "YA7WA"));
  }

  void testMatchFareClassEmptyRule() { CPPUNIT_ASSERT(matchFareClassN("", "Y26")); }

  void testMatchFareClassNoHyphenMatch() { CPPUNIT_ASSERT(matchFareClassN("Y26", "Y26")); }

  void testMatchFareClassNoHyphenNoMatch() { CPPUNIT_ASSERT(!matchFareClassN("Y26", "Y26HX0")); }
};
CPPUNIT_TEST_SUITE_REGISTRATION(MatchFareClassTest);

} // tse
