#include "FareCalc/PercentageComputator.h"
#include "test/include/CppUnitHelperMacros.h"
#include <math.h>

namespace tse
{
  double round( double value )
  {
      return floor( value*100 + 0.5 )/100;
  }

  class PercentageComputatorTest : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(PercentageComputatorTest);
    CPPUNIT_TEST(testWithDouble);
    CPPUNIT_TEST(testWithInts);
    CPPUNIT_TEST(testWithInts_roundingoff);
    CPPUNIT_TEST(equalSplitWithZeros);
    CPPUNIT_TEST_SUITE_END();

    public:
    void testWithInts()
    {
      PercentageComputator<std::string, int> computator(1500);
      computator.addToKey("A",300);
      computator.addToKey("B",200);
      computator.addToKey("C",500);

      std::map<std::string, int> outputMap = computator.getOutput();
      CPPUNIT_ASSERT(outputMap["A"] == 450);
      CPPUNIT_ASSERT(outputMap["B"] == 300);
      CPPUNIT_ASSERT(outputMap["C"] == 750);
      CPPUNIT_ASSERT(outputMap.size() == 3);
      // inexistent key should yield 0
      CPPUNIT_ASSERT(outputMap["D"] == 0);
    }
    void testWithInts_roundingoff()
    {
      PercentageComputator<std::string, int> computator(15);
      computator.addToKey("A",5);
      computator.addToKey("B",5);

      std::map<std::string, int> outputMap = computator.getOutput();
      CPPUNIT_ASSERT(outputMap["A"] == 8);
      CPPUNIT_ASSERT(outputMap["B"] == 7);
    }
    void testWithDouble()
    {
      PercentageComputator<std::string, double> computator(350.68);
      computator.addToKey("A",5.67);
      computator.addToKey("B",19.86);
      computator.addToKey("C",29.99);
      computator.addToKey("D",0);

      std::map<std::string, double> outputMap = computator.getOutput();
      CPPUNIT_ASSERT_EQUAL(round(outputMap["A"]), round(35.81));
      CPPUNIT_ASSERT_EQUAL(round(outputMap["B"]), round(125.44));
      CPPUNIT_ASSERT_EQUAL(round(outputMap["C"]), round(189.43));
      CPPUNIT_ASSERT_EQUAL(outputMap["D"], double(0));
    }

    void equalSplitWithZeros()
    {
      PercentageComputator<std::string, int> computator(1500);
      computator.addToKey("A",0);
      computator.addToKey("B",0);
      computator.addToKey("C",0);

      std::map<std::string, int> outputMap = computator.getOutput();
      CPPUNIT_ASSERT(outputMap["A"] == 500);
      CPPUNIT_ASSERT(outputMap["B"] == 500);
      CPPUNIT_ASSERT(outputMap["C"] == 500);
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION(PercentageComputatorTest);
}
