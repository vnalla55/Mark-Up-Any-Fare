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

// #include "Common/ErrorResponseException.h"

#include "Pricing/Shopping/Utils/StreamFormat.h"
#include <sstream>

using namespace std;

class Bulwa
{
public:
  Bulwa(int x) : _x(x) {}
  int getX() const { return _x; }

private:
  int _x;
};

class BulwaFormatter
{
public:
  void operator()(std::ostream& out, const Bulwa& b) const { out << "Bulwa = " << b.getX(); }
};

void
formatBulwa(std::ostream& out, const Bulwa& b)
{
  out << "Bulwa = " << b.getX();
}

namespace tse
{

using namespace utils;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class StreamFormatTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StreamFormatTest);
  CPPUNIT_TEST(formatWithFunctor);
  CPPUNIT_TEST(formatWithFunction);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    out.str("");
  }

  void tearDown() { _memHandle.clear(); }

  void formatWithFunctor()
  {
    Bulwa b1(77);
    out << tse::utils::format(b1, BulwaFormatter());
    CPPUNIT_ASSERT_EQUAL(string("Bulwa = 77"), out.str());
  }

  void formatWithFunction()
  {
    Bulwa b1(77);
    out << tse::utils::format(b1, formatBulwa);
    CPPUNIT_ASSERT_EQUAL(string("Bulwa = 77"), out.str());
  }

private:
  TestMemHandle _memHandle;
  ostringstream out;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StreamFormatTest);

} // namespace tse
