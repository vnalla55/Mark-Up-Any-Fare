//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"
#include "Xform/BrandingResponseBuilder.h"

#include <string>
#include <vector>
#include <set>
#include <sstream>

using namespace std;

namespace tse
{

void
verifyResponse(string expected, const BrandingResponseType& actual)
{
  ostringstream out;
  out << actual;
  CPPUNIT_ASSERT_EQUAL(expected, out.str());
}

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class BrandingResponseBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandingResponseBuilderTest);
  CPPUNIT_TEST(emptyBuilderTest);
  CPPUNIT_TEST(allDifferent);
  CPPUNIT_TEST(simpleRepetitionTest);
  CPPUNIT_TEST(someBrandsAndProgramsRepeat);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
    _brb = _memHandle.create<BrandingResponseBuilder>();
    for (int i = 0; i < 3; ++i)
      _itins.push_back(_memHandle.create<Itin>());
  }

  void tearDown() {
    _memHandle.clear();
    _itins.clear();
  }

  void emptyBuilderTest()
  {
    BrandingResponseType empty;
    CPPUNIT_ASSERT(empty == _brb->getBrandingResponse());
  }

  void allDifferent()
  {
    _itins[0]->itinNum() = 3;
    _itins[1]->itinNum() = 13;
    _brb->addBrandForItin(_itins[0], "123", "AB");
    _brb->addBrandForItin(_itins[0], "456", "BC");
    _brb->addBrandForItin(_itins[1], "2222", "WWWW");

    string exp = "Itin 3\n"
                 "    Brand AB\n"
                 "        Program 123\n"
                 "    Brand BC\n"
                 "        Program 456\n"
                 "Itin 13\n"
                 "    Brand WWWW\n"
                 "        Program 2222\n";

    verifyResponse(exp, _brb->getBrandingResponse());
  }

  void simpleRepetitionTest()
  {
    _itins[0]->itinNum() = 5;
    _brb->addBrandForItin(_itins[0], "555", "GGG");
    _brb->addBrandForItin(_itins[0], "555", "GGG");

    string exp = "Itin 5\n"
                 "    Brand GGG\n"
                 "        Program 555\n";

    verifyResponse(exp, _brb->getBrandingResponse());
  }

  void someBrandsAndProgramsRepeat()
  {
    _itins[0]->itinNum() = 0;
    _itins[1]->itinNum() = 1;
    _itins[2]->itinNum() = 3;
    _brb->addBrandForItin(_itins[0], "123", "AB");
    _brb->addBrandForItin(_itins[2], "77", "QTH");
    _brb->addBrandForItin(_itins[0], "77", "XY");
    _brb->addBrandForItin(_itins[1], "421", "NICE");
    _brb->addBrandForItin(_itins[1], "123", "AB");
    _brb->addBrandForItin(_itins[2], "1117111", "AB");

    string exp = "Itin 0\n"
                 "    Brand AB\n"
                 "        Program 123\n"
                 "    Brand XY\n"
                 "        Program 77\n"
                 "Itin 1\n"
                 "    Brand AB\n"
                 "        Program 123\n"
                 "    Brand NICE\n"
                 "        Program 421\n"
                 "Itin 3\n"
                 "    Brand AB\n"
                 "        Program 1117111\n"
                 "    Brand QTH\n"
                 "        Program 77\n";

    verifyResponse(exp, _brb->getBrandingResponse());
  }

private:
  TestMemHandle _memHandle;
  BrandingResponseBuilder* _brb;
  std::vector<Itin*> _itins;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BrandingResponseBuilderTest);

} // namespace tse
