#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "Fares/SalesRestrictionByNation.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

#include <set>
#include <vector>

using namespace std;

namespace tse
{

class SalesRestrictionByNationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SalesRestrictionByNationTest);
  CPPUNIT_TEST(testMatchCarrier_MatchButException);
  CPPUNIT_TEST(testMatchCarrier_MatchButNoException);
  CPPUNIT_TEST(testMatchCarrier_NoMatchButException);
  CPPUNIT_TEST(testMatchCarrier_NoMatchAndNoException);
  CPPUNIT_TEST_SUITE_END();


public:

#if 0
carrier: AS AM CM
cxr: AS
exceptCxr = Y
return: false

cxr: AS
exceptCxr = N
return: true

cxr: AA
execptCXR = Y
return: true

cxr: AA
exceptCxr = N
return: false
#endif


  void testMatchCarrier_MatchButException()
  {
    Itin itin;
    std::vector<CarrierCode> restrictedCxrsByRule = {"AS", "AM", "CM"};
    Indicator exceptCxr = 'Y';
    std::vector<CarrierCode> validatingCxrs = { "AS" };
    std::set<CarrierCode> restrictedCxrs;

    CPPUNIT_ASSERT(_saleRest.matchCarrier(itin, restrictedCxrsByRule,
          exceptCxr, validatingCxrs, restrictedCxrs)==false);
    CPPUNIT_ASSERT(_saleRest.matchCarrier(restrictedCxrsByRule,
          exceptCxr, validatingCxrs.front())==false);
  }

  void testMatchCarrier_MatchButNoException()
  {
    Itin itin;
    std::vector<CarrierCode> restrictedCxrsByRule = {"AS", "AM", "CM"};
    Indicator exceptCxr = 'N';
    std::vector<CarrierCode> validatingCxrs = { "AS" };
    std::set<CarrierCode> restrictedCxrs;

    CPPUNIT_ASSERT(_saleRest.matchCarrier(itin, restrictedCxrsByRule, exceptCxr,
          validatingCxrs, restrictedCxrs)==true);
    CPPUNIT_ASSERT(_saleRest.matchCarrier(restrictedCxrsByRule, exceptCxr,
          validatingCxrs.front())==true);

  }

  void testMatchCarrier_NoMatchButException()
  {
    Itin itin;
    std::vector<CarrierCode> restrictedCxrsByRule = {"AS", "AM", "CM"};
    Indicator exceptCxr = 'Y';
    std::vector<CarrierCode> validatingCxrs = { "AA" };
    std::set<CarrierCode> restrictedCxrs;

    CPPUNIT_ASSERT(_saleRest.matchCarrier(itin, restrictedCxrsByRule, exceptCxr,
          validatingCxrs, restrictedCxrs)==true);
    CPPUNIT_ASSERT(_saleRest.matchCarrier(restrictedCxrsByRule, exceptCxr,
          validatingCxrs.front())==true);
  }

  void testMatchCarrier_NoMatchAndNoException()
  {
    Itin itin;
    std::vector<CarrierCode> restrictedCxrsByRule = {"AS", "AM", "CM"};
    Indicator exceptCxr = 'N';
    std::vector<CarrierCode> validatingCxrs = { "AA" };
    std::set<CarrierCode> restrictedCxrs;

    CPPUNIT_ASSERT(_saleRest.matchCarrier(itin, restrictedCxrsByRule, exceptCxr,
          validatingCxrs, restrictedCxrs)==false);
    CPPUNIT_ASSERT(_saleRest.matchCarrier(restrictedCxrsByRule, exceptCxr,
          validatingCxrs.front())==false);
  }

private:
  SalesRestrictionByNation _saleRest;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SalesRestrictionByNationTest);
}
