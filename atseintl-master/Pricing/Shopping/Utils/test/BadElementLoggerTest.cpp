//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/include/GtestHelperMacros.h"
#include "Pricing/Shopping/Utils/BadElementLogger.h"

#include <memory>
#include <sstream>

using namespace ::testing;
using namespace boost;

namespace tse
{

namespace utils
{

template <typename T0>
class MockINamedPredicate : public INamedPredicate<T0> {
 public:
  virtual bool operator()(const T0& t){
    return RoundBracketOperator(t); }
  MOCK_METHOD1_T(RoundBracketOperator,
      bool(const T0& t));
  MOCK_CONST_METHOD0_T(getName,
      std::string());
};


class MockILogger : public ILogger {
 public:
  MOCK_METHOD2(message,
      void(LOGGER_LEVEL level, const std::string& msg));
  MOCK_METHOD1(debug,
      void(const std::string& msg));
  MOCK_METHOD1(debug,
      void(const boost::format& msg));
  MOCK_METHOD1(info,
      void(const std::string& msg));
  MOCK_METHOD1(info,
      void(const boost::format& msg));
  MOCK_METHOD1(error,
      void(const std::string& msg));
  MOCK_METHOD1(error,
      void(const boost::format& msg));
  MOCK_CONST_METHOD1(enabled,
      bool(LOGGER_LEVEL level));
};


class BadElementLoggerTest: public Test
{
public:
  typedef BadElementLogger<int> TestedBadElementLogger;
  typedef MockINamedPredicate<int> TestedMockINamedPredicate;

  void SetUp()
  {
    _mockILogger.reset(new StrictMock<MockILogger>());
    _badElemLogger.reset(new TestedBadElementLogger(*_mockILogger, DUMMY_LEVEL, DUMMY_METANAME));
    _mockINamedPredicate.reset(new StrictMock<TestedMockINamedPredicate>());
  }
  void TearDown(){}

  std::shared_ptr<MockILogger> _mockILogger;
  std::shared_ptr<TestedBadElementLogger> _badElemLogger;
  std::shared_ptr<TestedMockINamedPredicate> _mockINamedPredicate;

  static const utils::LOGGER_LEVEL DUMMY_LEVEL;
  static const std::string DUMMY_METANAME;
  static const int DUMMY_ELEMENT;
  static const std::string DUMMY_PREDICATE_NAME;
};

const utils::LOGGER_LEVEL BadElementLoggerTest::DUMMY_LEVEL = utils::LOGGER_LEVEL::DEBUG;
const std::string BadElementLoggerTest::DUMMY_METANAME = "DummyMetaname";
const int BadElementLoggerTest::DUMMY_ELEMENT = 17;
const std::string BadElementLoggerTest::DUMMY_PREDICATE_NAME = "--predicate--";

TEST_F(BadElementLoggerTest, testLogsInvalidElementEventIfLogEnabled)
{
  EXPECT_CALL(*_mockILogger, enabled(DUMMY_LEVEL))
        .WillOnce(Return(true));
  EXPECT_CALL(*_mockINamedPredicate, getName())
        .WillOnce(Return(DUMMY_PREDICATE_NAME));
  std::ostringstream expectedMsg;
  expectedMsg << DUMMY_METANAME << " " << DUMMY_ELEMENT
      << " failed. Reason: " << DUMMY_PREDICATE_NAME;
  EXPECT_CALL(*_mockILogger, message(DUMMY_LEVEL, expectedMsg.str()))
        .Times(1);
  _badElemLogger->elementInvalid(DUMMY_ELEMENT, *_mockINamedPredicate);
}

TEST_F(BadElementLoggerTest, testDoesNothingIfLogDisabled)
{
  EXPECT_CALL(*_mockILogger, enabled(DUMMY_LEVEL))
        .WillOnce(Return(false));
  _badElemLogger->elementInvalid(DUMMY_ELEMENT, *_mockINamedPredicate);
}

} // namespace utils

} // namespace tse

