
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
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"
#include "Pricing/Shopping/Utils/StreamLogger.h"
#include "Pricing/Shopping/Utils/LoggerHandle.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"

#include <string>
#include <sstream>

namespace tse
{

namespace utils
{

using namespace std;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class StreamLoggerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StreamLoggerTest);
  CPPUNIT_TEST(enableTest);
  CPPUNIT_TEST(stringLoggingTest);
  CPPUNIT_TEST(formatterTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    out.str("");
    _log.install(new StreamLogger(&out));
  }

  void tearDown() { _log.clear(); }

  void enableTest()
  {
    CPPUNIT_ASSERT_EQUAL(true, _log->enabled(LOGGER_LEVEL::DEBUG));
    CPPUNIT_ASSERT_EQUAL(true, _log->enabled(LOGGER_LEVEL::INFO));
    CPPUNIT_ASSERT_EQUAL(true, _log->enabled(LOGGER_LEVEL::ERROR));
  }

  void stringLoggingTest()
  {
    _log->debug("Hello world");
    CPPUNIT_ASSERT_EQUAL(string("DEBUG: Hello world\n"), out.str());
    out.str("");
    _log->info("Hello world");
    CPPUNIT_ASSERT_EQUAL(string(" INFO: Hello world\n"), out.str());
    out.str("");
    _log->message(LOGGER_LEVEL::ERROR, "Hello world");
    CPPUNIT_ASSERT_EQUAL(string("ERROR: Hello world\n"), out.str());
    out.str("");
  }

  void formatterTest()
  {
    using namespace tse::swp;

    _log->debug(Fmt("Hello world %u") % 1);
    CPPUNIT_ASSERT_EQUAL(string("DEBUG: Hello world 1\n"), out.str());
    out.str("");
    _log->info(Fmt("Hello world %d") % -3);
    CPPUNIT_ASSERT_EQUAL(string(" INFO: Hello world -3\n"), out.str());
    out.str("");
    _log->error(Fmt("Hello world %s") % "four");
    CPPUNIT_ASSERT_EQUAL(string("ERROR: Hello world four\n"), out.str());
    out.str("");

    BasicAppraiserScore bas(BasicAppraiserScore::MUST_HAVE, 57);
    _log->error(Fmt("Hello world %s") % bas);
    CPPUNIT_ASSERT_EQUAL(string("ERROR: Hello world M(57)\n"), out.str());
    out.str("");
  }

private:
  LoggerHandle _log;
  ostringstream out;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StreamLoggerTest);

} // namespace utils

} // namespace tse
