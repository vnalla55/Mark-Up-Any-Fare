#include <vector>
#include "test/include/CppUnitHelperMacros.h"
#include "RexPricing/GenericRexMapper.h"
#include "Diagnostic/Diag689Collector.h"
#include "RexPricing/test/CommonRexPricing.h"

#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::list_of;

static const PaxTypeFare* _nullPtr = 0;

class GenericRexMapperTest : public CppUnit::TestFixture, public CommonRexPricing
{
  CPPUNIT_TEST_SUITE(GenericRexMapperTest);
  CPPUNIT_TEST(testDomesticWrongCityMapping);
  CPPUNIT_TEST(testDirectInternationalMapping);
  CPPUNIT_TEST(testInternationalNationMapping);
  CPPUNIT_TEST(testInternationalSubAreaMapping);
  CPPUNIT_TEST(testInternationalOppositeDoNotMatchMapping);
  CPPUNIT_TEST_SUITE_END();

public:
  GenericRexMapperTest() {}

  void setUp()
  {
    CommonRexPricing::setUp();

    _fbrm = _memHandle(new GenericRexMapper(*_trx, _allRepricePTFs));
    _fbrm->setDiag(_memHandle(new Diag689Collector));
  }

  void tearDown() { CommonRexPricing::tearDown(); }

  void assertAllMapped()
  {
    for (int i = 0; i != 3; ++i)
      CPPUNIT_ASSERT_EQUAL(_fbrm->_internationalMap.at(i), _fbrm->_allRepricePTFs->at(i));
  }

  void testDomesticWrongCityMapping()
  {
    _trx->exchangeItin().front()->geoTravelType() = GeoTravelType::Domestic;
    setUpDirectCityCityMapping();

    FareMarket* fm = getPTF(1);
    setUpLocation(fm->offMultiCity(), const_cast<Loc&>(*fm->destination()), "WAS", "US", "11");

    _fbrm->map();

    CPPUNIT_ASSERT_EQUAL(_allRepricePTFs->at(0), _fbrm->_domesticMap.at(0));
    CPPUNIT_ASSERT_EQUAL(_nullPtr, _fbrm->_domesticMap.at(1));
    CPPUNIT_ASSERT_EQUAL(_allRepricePTFs->at(2), _fbrm->_domesticMap.at(2));
  }

  void testDirectInternationalMapping()
  {
    setUpDirectCityCityMapping();

    _fbrm->map();

    assertAllMapped();
  }

  void testInternationalNationMapping()
  {
    setUpDirectCityCityMapping();

    FareMarket* fm = getPTF(0);
    setUpLocation(fm->offMultiCity(), const_cast<Loc&>(*fm->destination()), "WAW", "PL", "21");

    fm = getPTF(2);
    setUpLocation(fm->boardMultiCity(), const_cast<Loc&>(*fm->origin()), "WAS", "US", "11");
    setUpLocation(fm->offMultiCity(), const_cast<Loc&>(*fm->destination()), "LED", "RU", "32");

    _fbrm->map();

    assertAllMapped();
  }

  void testInternationalSubAreaMapping()
  {
    setUpDirectCityCityMapping();

    FareMarket* fm = getPTF(0);
    setUpLocation(fm->offMultiCity(), const_cast<Loc&>(*fm->destination()), "AGA", "MA", "21");

    fm = getPTF(2);
    setUpLocation(fm->boardMultiCity(), const_cast<Loc&>(*fm->origin()), "ARE", "PR", "11");
    setUpLocation(fm->offMultiCity(), const_cast<Loc&>(*fm->destination()), "BBM", "KH", "32");

    _fbrm->map();

    assertAllMapped();
  }

  void testInternationalOppositeDoNotMatchMapping()
  {
    setUpDirectCityCityMapping();

    FareMarket* fm = getPTF(0);
    setUpLocation(fm->boardMultiCity(), const_cast<Loc&>(*fm->origin()), "LED", "RU", "32");
    setUpLocation(fm->offMultiCity(), const_cast<Loc&>(*fm->destination()), "KRK", "XX", "XX");

    fm = getPTF(1);
    setUpLocation(fm->boardMultiCity(), const_cast<Loc&>(*fm->origin()), "AGA", "MA", "21");
    setUpLocation(fm->offMultiCity(), const_cast<Loc&>(*fm->destination()), "XX", "US", "XX");

    _fbrm->map();

    CPPUNIT_ASSERT_EQUAL(_nullPtr, _fbrm->_internationalMap.at(0));
    CPPUNIT_ASSERT_EQUAL(_nullPtr, _fbrm->_internationalMap.at(1));
    CPPUNIT_ASSERT_EQUAL(_allRepricePTFs->at(2), _fbrm->_internationalMap.at(2));
  }

private:
  GenericRexMapper* _fbrm;
};

CPPUNIT_TEST_SUITE_REGISTRATION(GenericRexMapperTest);
}
