#include "Common/ObFeeDescriptors.h"
#include "test/include/CppUnitHelperMacros.h"

#include <iostream>
#include <string>

namespace tse
{
class ObFeeDescriptorsTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ObFeeDescriptorsTest);
  CPPUNIT_TEST(testFTypeFees);
  CPPUNIT_TEST(testRTypeFees);
  CPPUNIT_TEST(testTTypeFees);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void testFTypeFees()
  {
    ObFeeDescriptors descriptor;
    std::string outputStr;

    outputStr = descriptor.getDescription("FCA");
    CPPUNIT_ASSERT_EQUAL(std::string(""), outputStr);

    outputStr = descriptor.getDescription("FDA");
    CPPUNIT_ASSERT_EQUAL(std::string(""), outputStr);
  }

  void testRTypeFees()
  {
    ObFeeDescriptors descriptor;
    std::string outputStr;

    outputStr = descriptor.getDescription("R02");
    CPPUNIT_ASSERT_EQUAL(std::string("COURIER CHARGE"), outputStr);

    outputStr = descriptor.getDescription("R05");
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET"), outputStr);

    outputStr = descriptor.getDescription("R11");
    CPPUNIT_ASSERT_EQUAL(std::string("NON CREDIT CARD PAYMENT FEE"), outputStr);

    outputStr = descriptor.getDescription("R15");
    CPPUNIT_ASSERT_EQUAL(std::string("OPTIONAL SERVICE FEE15"), outputStr);
  }

  void testTTypeFees()
  {
    ObFeeDescriptors descriptor;
    std::string outputStr;

    outputStr = descriptor.getDescription("T02");
    CPPUNIT_ASSERT_EQUAL(std::string("CARRIER TICKETING FEE02"), outputStr);

    outputStr = descriptor.getDescription("T20");
    CPPUNIT_ASSERT_EQUAL(std::string("CARRIER TICKETING FEE20"), outputStr);

    outputStr = descriptor.getDescription("T55");
    CPPUNIT_ASSERT_EQUAL(std::string("CARRIER TICKETING FEE55"), outputStr);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ObFeeDescriptorsTest);

}
