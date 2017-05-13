#ifndef APP_FEE_TEST_H
#define APP_FEE_TEST_H

#include "Taxes/LegacyTaxes/ApplicationFee.h"
#include "DataModel/AirSeg.h"
#include "test/include/CppUnitHelperMacros.h"

class ApplicationFeeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ApplicationFeeTest);
  CPPUNIT_TEST(testOneWay);
  CPPUNIT_SKIP_TEST(testPortion);
  CPPUNIT_TEST_SUITE_END();

public:
  void init();
  // void dump(tse::AllAppFees &f);
  void testOneWay();
  void testPortion();

  // tse::AllAppFees              _allAppFees;
  std::vector<tse::AirSeg*> _itinSegs;
};

#endif // APP_FEE_TEST_H
