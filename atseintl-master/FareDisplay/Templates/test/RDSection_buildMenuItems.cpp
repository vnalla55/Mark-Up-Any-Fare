
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Templates/RDSection.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{

using namespace boost::assign;

class RDSection_buildMenuItems : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RDSection_buildMenuItems);
  CPPUNIT_TEST(testBuildMenuItems_longItemSize);
  CPPUNIT_TEST(testBuildMenuItems_oneLine);
  CPPUNIT_TEST(testBuildMenuItems_twoLines);
  CPPUNIT_TEST(testBuildMenuItems_twoLinesFull);
  CPPUNIT_TEST(testBuildMenuItems_oneLineWithLongItemFirst);
  CPPUNIT_TEST(testBuildMenuItems_oneLineWithLongItemSecond);
  CPPUNIT_TEST(testBuildMenuItems_twoLinesWithLongItemFirst);
  CPPUNIT_TEST(testBuildMenuItems_twoLinesWithLongItemSecond);
  CPPUNIT_TEST(testBuildMenuItems_twoLinesWithLongItemThird);
  CPPUNIT_TEST(testBuildMenuItems_twoLinesWithLongItemThirdMoreItems);
  CPPUNIT_TEST(testBuildMenuItems_threeLinesWithLongItemThird);
  CPPUNIT_TEST(testBuildMenuItems_threeLinesWithLongItemThirdMoreItems);
  CPPUNIT_TEST(testBuildMenuItems_twoLinesWithLongItemLast);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memHandle;
  FareDisplayTrx* _fdTrx = nullptr;
  RDSection* _section = nullptr;
  FareDisplayInfo fdi;
  bool duty;
  const std::string itemA, itemB, itemC, itemD, item_long;
  const unsigned item_width;

  std::ostringstream stream;

  typedef RDSection::MenuItem MenuItem;
  std::vector<MenuItem> items;

  RDSection_buildMenuItems()
    : duty(false),
      itemA("itemA"),
      itemB("itemB"),
      itemC("itemC"),
      itemD("itemD"),
      item_long("very long long long item"),
      item_width(MAX_PSS_LINE_SIZE / 3)
  {
  }

  std::string formatItem(const MenuItem& item, int width)
  {
    std::ostringstream str;
    str << std::setw(width) << std::left << _section->formatMenuItem(fdi, duty, item);
    return str.str();
  }

  std::string formatLine(const MenuItem& item1, const MenuItem& item2, const MenuItem& item3)
  {
    std::ostringstream str;
    str << formatItem(item1, item_width + 1) << formatItem(item2, item_width - 1)
        << formatItem(item3, item_width) << "\n";
    return str.str();
  }

  void assertMenu(const std::string& expect, const std::string& output)
  {
    std::istringstream is(output);
    std::string line;
    while (std::getline(is, line))
      CPPUNIT_ASSERT(line.size() < static_cast<unsigned>(MAX_PSS_LINE_SIZE));

    CPPUNIT_ASSERT_EQUAL(expect, output);
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdTrx = _memHandle.create<FareDisplayTrx>();
    _section = _memHandle.create<RDSection>(*_fdTrx);

    items.clear();
    stream.str("");
  }

  void tearDown() { _memHandle.clear(); }

  void testBuildMenuItems_oneLine()
  {
    items += std::make_pair(50, &itemA), std::make_pair(2, &itemB), std::make_pair(33, &itemC);

    stream << formatLine(items[0], items[1], items[2]);

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_twoLines()
  {
    items += std::make_pair(50, &itemA), std::make_pair(2, &itemB), std::make_pair(33, &itemC),
        std::make_pair(99, &itemD);

    stream << formatLine(items[0], items[1], items[2]) << formatItem(items[3], item_width + 1)
           << "\n";

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_twoLinesFull()
  {
    items += std::make_pair(50, &itemA), std::make_pair(2, &itemB), std::make_pair(33, &itemC),
        std::make_pair(99, &itemD), std::make_pair(10, &itemA), std::make_pair(11, &itemB);

    stream << formatLine(items[0], items[1], items[2]) << formatLine(items[3], items[4], items[5]);

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_longItemSize()
  {
    CPPUNIT_ASSERT(item_long.size() > item_width - 4);
    CPPUNIT_ASSERT(item_long.size() < 2 * item_width - 4);
  }

  void testBuildMenuItems_oneLineWithLongItemFirst()
  {
    items += std::make_pair(2, &item_long), std::make_pair(50, &itemA);

    stream << formatItem(items[0], 2 * item_width) << formatItem(items[1], item_width) << "\n";

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_oneLineWithLongItemSecond()
  {
    items += std::make_pair(50, &itemA), std::make_pair(2, &item_long);

    stream << formatItem(items[0], item_width + 1) << formatItem(items[1], 2 * item_width - 1)
           << "\n";

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_twoLinesWithLongItemFirst()
  {
    items += std::make_pair(2, &item_long), std::make_pair(50, &itemA), std::make_pair(3, &itemB);

    stream << formatItem(items[0], 2 * item_width) << formatItem(items[1], item_width) << "\n"
           << formatItem(items[2], item_width + 1) << "\n";

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_twoLinesWithLongItemSecond()
  {
    items += std::make_pair(50, &itemA), std::make_pair(2, &item_long), std::make_pair(3, &itemB);

    stream << formatItem(items[0], item_width + 1) << formatItem(items[1], 2 * item_width - 1)
           << "\n" << formatItem(items[2], item_width + 1) << "\n";

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_twoLinesWithLongItemThird()
  {
    items += std::make_pair(50, &itemA), std::make_pair(3, &itemB), std::make_pair(2, &item_long);

    stream << formatItem(items[0], item_width + 1) << formatItem(items[1], item_width - 1) << "\n"
           << formatItem(items[2], 2 * item_width) << "\n";

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_twoLinesWithLongItemThirdMoreItems()
  {
    items += std::make_pair(50, &itemA), std::make_pair(3, &itemB), std::make_pair(2, &item_long),
        std::make_pair(5, &itemC);

    stream << formatItem(items[0], item_width + 1) << formatItem(items[1], item_width - 1) << '\n'
           << formatItem(items[2], 2 * item_width) << formatItem(items[3], item_width) << '\n';

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_threeLinesWithLongItemThird()
  {
    items += std::make_pair(50, &itemA), std::make_pair(3, &itemB), std::make_pair(2, &item_long),
        std::make_pair(5, &itemC), std::make_pair(7, &itemD);

    stream << formatItem(items[0], item_width + 1) << formatItem(items[1], item_width - 1) << '\n'
           << formatItem(items[2], 2 * item_width) << formatItem(items[3], item_width) << '\n'
           << formatItem(items[4], item_width + 1) << '\n';

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_threeLinesWithLongItemThirdMoreItems()
  {
    items += std::make_pair(50, &itemA), std::make_pair(3, &itemB), std::make_pair(2, &item_long),
        std::make_pair(5, &itemC), std::make_pair(7, &itemD), std::make_pair(88, &itemA),
        std::make_pair(99, &itemB);

    stream << formatItem(items[0], item_width + 1) << formatItem(items[1], item_width - 1) << '\n'
           << formatItem(items[2], 2 * item_width) << formatItem(items[3], item_width) << '\n'
           << formatLine(items[4], items[5], items[6]);

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }

  void testBuildMenuItems_twoLinesWithLongItemLast()
  {
    items += std::make_pair(50, &itemA), std::make_pair(1, &itemB), std::make_pair(2, &itemC),
        std::make_pair(3, &itemD), std::make_pair(4, &item_long);

    stream << formatLine(items[0], items[1], items[2]) << formatItem(items[3], item_width + 1)
           << formatItem(items[4], 2 * item_width - 1) << '\n';

    assertMenu(stream.str(), _section->buildMenuItems(fdi, duty, items));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RDSection_buildMenuItems);
}
