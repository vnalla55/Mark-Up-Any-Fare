#include <string.h>
#include "test/include/CppUnitHelperMacros.h"
#include "test/XmlAndDiagnosticFixture.h"
#include "test/mainPath.h"

using namespace tax;
using namespace std;

namespace tse
{
class Loc2IntlDomTest : public XmlAndDiagnosticFixture
{
  CPPUNIT_TEST_SUITE(Loc2IntlDomTest);

  // CPPUNIT_TEST(test1);

  CPPUNIT_TEST_SUITE_END();

public:
  void test1()
  {
    string str;
    int numOfLines = runTest(mainPath + "Rules/test/data/loc2IntlDom.xml", 802, str);
    CPPUNIT_ASSERT_EQUAL(20, numOfLines);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Loc2IntlDomTest);
}
