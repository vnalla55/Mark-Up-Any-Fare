#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include <boost/tokenizer.hpp>
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Common/TseSrvStats.h"
#include "Server/AppConsoleStats.h"

using namespace tse;
using namespace std;
using namespace boost;

namespace tse
{
class TseSrvStatsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TseSrvStatsTest);
  CPPUNIT_TEST(testElapsedTagAttributesUniqueness);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TseSrvStats::_acStats = _memHandle.create<AppConsoleStats>();
  }

  void tearDown()
  {
    _memHandle.clear();
    TseSrvStats::_acStats = 0;
  }

  void testElapsedTagAttributesUniqueness()
  {
    memset(TseSrvStats::_acStats, 0, sizeof(AppConsoleStats));
    ostringstream oss;
    TseSrvStats::dumpElapsed(oss);
    string elapsedTag = oss.str();

    char_separator<char> spaceSep(" ");
    char_separator<char> equalSep("=");

    tokenizer<char_separator<char> > tokens(elapsedTag, spaceSep);
    set<string> tokensSet;

    for (const string& t : tokens)
    {
      CPPUNIT_ASSERT_MESSAGE("Duplicated attribute: " + t, tokensSet.end() == tokensSet.find(t));
      tokensSet.insert(t);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TseSrvStatsTest);
}
