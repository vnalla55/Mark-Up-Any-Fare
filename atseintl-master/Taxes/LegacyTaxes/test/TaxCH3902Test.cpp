#include "test/include/CppUnitHelperMacros.h"
#include "Taxes/LegacyTaxes/TaxCH3902.cpp"
#include "DataModel/AirSeg.h"

using namespace std;

namespace tse
{
class TaxCH3902Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxCH3902Test);
  CPPUNIT_TEST(testValidateSpecialFlightsTransitPassesWhenBothAreNotSpecialCase);
  CPPUNIT_TEST(testValidateSpecialFlightsTransitPassesWhenFirstIsSpecialCase);
  CPPUNIT_TEST(testValidateSpecialFlightsTransitPassesWhenSecondIsSpecialCase);
  CPPUNIT_TEST(testValidateSpecialFlightsTransitFailsWhenBothAreSpecialCase);
  CPPUNIT_TEST_SUITE_END();

  TaxCH3902 tax;

  AirSeg* _from;
  AirSeg* _to;

public:
  void setUp()
  {
    _from = new AirSeg();
    _to = new AirSeg();
  }

  void tearDown()
  {
    delete _from;
    delete _to;
  }

  void testValidateSpecialFlightsTransitPassesWhenBothAreNotSpecialCase()
  {
    _from->marketingFlightNumber() = 111;
    _from->setMarketingCarrierCode("AA");

    _to->marketingFlightNumber() = 222;
    _to->setMarketingCarrierCode("LH");

    CPPUNIT_ASSERT(tax.validateSpecialFlightsTransit(_from, _to));
  }

  void testValidateSpecialFlightsTransitPassesWhenFirstIsSpecialCase()
  {
    _from->marketingFlightNumber() = FROM_SPECIAL_FLIGHT;
    _from->setMarketingCarrierCode(FROM_TO_SPECIAL_CARRIER);

    _to->marketingFlightNumber() = 222;
    _to->setMarketingCarrierCode("LH");

    CPPUNIT_ASSERT(tax.validateSpecialFlightsTransit(_from, _to));
  }

  void testValidateSpecialFlightsTransitPassesWhenSecondIsSpecialCase()
  {
    _from->marketingFlightNumber() = 111;
    _from->setMarketingCarrierCode("LH");

    _to->marketingFlightNumber() = TO_SPECIAL_FLIGHT;
    _to->setMarketingCarrierCode(FROM_TO_SPECIAL_CARRIER);

    CPPUNIT_ASSERT(tax.validateSpecialFlightsTransit(_from, _to));
  }

  void testValidateSpecialFlightsTransitFailsWhenBothAreSpecialCase()
  {
    _from->marketingFlightNumber() = FROM_SPECIAL_FLIGHT;
    _from->setMarketingCarrierCode(FROM_TO_SPECIAL_CARRIER);

    _to->marketingFlightNumber() = TO_SPECIAL_FLIGHT;
    _to->setMarketingCarrierCode(FROM_TO_SPECIAL_CARRIER);

    CPPUNIT_ASSERT(!tax.validateSpecialFlightsTransit(_from, _to));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxCH3902Test);
};
