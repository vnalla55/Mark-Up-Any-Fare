// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "TaxDisplay/Response/LineParams.h"
#include "TaxDisplay/Response/ResponseFormatter.h"
#include "test/include/CppUnitHelperMacros.h"

#include <sstream>

namespace tax
{
namespace display
{

class ResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ResponseFormatterTest);

  CPPUNIT_TEST(testFormat);
  CPPUNIT_TEST(testLineBreak);
  CPPUNIT_TEST(testLineTruncate);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}

  void testFormat()
  {
    ResponseFormatter formatter;
    formatter.addLine("1234").addBlankLine().addLine("12345");

    std::ostringstream response;
    formatter.format(response);

    CPPUNIT_ASSERT(response.str().size() == 12); // 1234 + \n + \n + 12345 + \n
    CPPUNIT_ASSERT(formatter._linesList.size() == 0);
  }

  void testLineBreak()
  {
    using std::string;

    LineParams withIndent54, withNoIndent;
    withIndent54.setLeftMargin(54);

    ResponseFormatter formatter;
    formatter.addLine("salamander goo", withIndent54);         // should split into a two lines
    formatter.addLine("123456789 123456 1234", withIndent54);  // should split into a three lines
    formatter.addLine("axolotl", withNoIndent);

    std::vector<string> lines;
    for(ResponseFormatter::LinesList::iterator lineIt = formatter._linesList.begin();
        lineIt != formatter._linesList.end();
        ++lineIt)
    {
      string formatted;
      formatter.formatLine(lineIt, formatted);
      lines.push_back(formatted);

      CPPUNIT_ASSERT(lineIt->str().empty()); // line should be swapped with displayLine
    }

    CPPUNIT_ASSERT_EQUAL(ResponseFormatter::LinesList::size_type(6), lines.size());

    CPPUNIT_ASSERT_EQUAL(string::size_type(64), lines[0].length() );   // indent54 + salamander
    CPPUNIT_ASSERT_EQUAL(string::size_type(57), lines[1].length() );   // indent54 + goo
    CPPUNIT_ASSERT_EQUAL(string::size_type(63), lines[2].length() );   // indent54 + 123456789
    CPPUNIT_ASSERT_EQUAL(string::size_type(60), lines[3].length() );   // indent54 + 123456
    CPPUNIT_ASSERT_EQUAL(string::size_type(58), lines[4].length() );   // indent54 + 1234
    CPPUNIT_ASSERT_EQUAL(string::size_type(7),  lines[5].length() );   // axolotl
  }

  void testLineTruncate()
  {
    using std::string;

    LineParams withIndent54, withNoIndent;
    withIndent54.setLeftMargin(54);
    withIndent54.longLineFormatting() = LineParams::LongLineFormatting::TRUNCATE;
    withNoIndent.longLineFormatting() = LineParams::LongLineFormatting::TRUNCATE;

    ResponseFormatter formatter;
    formatter.addLine("salamander goo", withIndent54);
    formatter.addLine("123456789 123456 1234", withIndent54);
    formatter.addLine("axolotl", withNoIndent);

    std::vector<string> lines;
    for(ResponseFormatter::LinesList::iterator lineIt = formatter._linesList.begin();
        lineIt != formatter._linesList.end();
        ++lineIt)
    {
      string formatted;
      formatter.formatLine(lineIt, formatted);
      lines.push_back(formatted);

      CPPUNIT_ASSERT(lineIt->str().empty()); // line should be swapped with displayLine
    }

    CPPUNIT_ASSERT_EQUAL(ResponseFormatter::LinesList::size_type(3), lines.size());

    CPPUNIT_ASSERT_EQUAL(string::size_type(64), lines[0].length() );   // indent54 + salamander
    CPPUNIT_ASSERT_EQUAL(string::size_type(64), lines[1].length() );   // indent54 + 123456789
    CPPUNIT_ASSERT_EQUAL(string::size_type(7),  lines[2].length() );   // axolotl
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ResponseFormatterTest);

} /* namespace display */
} /* namespace tax */
