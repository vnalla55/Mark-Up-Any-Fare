#ifndef FARE_DISPLAY_TAX_TEST_H
#define FARE_DISPLAY_TAX_TEST_H

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
class FareDisplayTrx;
class FareMarket;

class FareDisplayTaxTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(FareDisplayTaxTest);
  CPPUNIT_TEST(testGetTaxCodeForFQ_fareTaxRequest);
  CPPUNIT_TEST(testGetTaxCodeForFQ_SingleUS);
  CPPUNIT_TEST(testGetTaxCodeForFQ_DoubleOrigUS);
  CPPUNIT_TEST(testGetTaxCodeForFQ_NoUSTaxForOrigUSTerr);
  CPPUNIT_TEST(testGetTaxCodeForFQ_OrigDestNotTerrUS);
  CPPUNIT_TEST(testGetTaxCodeForFQ_AgentLocAustraliaDifferentCityPair);
  CPPUNIT_SKIP_TEST(testGetTaxCodeForFQ_AgentLocAustraliaSameCityPair);
  CPPUNIT_TEST(testGetTaxCodeForFQ_AgentLocKiwi);
  CPPUNIT_TEST_SUITE_END();

private:
  FareDisplayTrx* _trx;
  std::set<TaxCode> _taxCode;
  GlobalDirection _globalDir;
  FareMarket* _fm;

public:
  void setUp();
  void tearDown();

  FareDisplayTrx* createFDTrx();
  void destroyFDTrx(FareDisplayTrx* trx);
  FareMarket* createFareMarket();
  void destroyFareMarket(FareMarket* trx);

  void testGetTaxCodeForFQ_fareTaxRequest();
  void testGetTaxCodeForFQ_SingleUS();
  void testGetTaxCodeForFQ_DoubleOrigUS();
  void testGetTaxCodeForFQ_NoUSTaxForOrigUSTerr();
  void testGetTaxCodeForFQ_OrigDestNotTerrUS();
  void testGetTaxCodeForFQ_AgentLocAustraliaDifferentCityPair();
  void testGetTaxCodeForFQ_AgentLocAustraliaSameCityPair();
  void testGetTaxCodeForFQ_AgentLocKiwi();

  void assertTaxCodes(int expected, TaxCode taxCode[]);
  void setLoc(const Loc* loc, const NationCode& nation, const StateCode& state);
  void setBoardOff(FareMarket* fm, const LocCode& board, const LocCode& off);
};

} // tse
#endif // FARE_DISPLAY_TAX_H
