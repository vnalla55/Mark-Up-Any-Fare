//
// Copyright Sabre 2011-12-05
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "Common/MessageWrapper.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

namespace tse
{
class MessageWrapperTest : public ::testing::Test
{
public:
  void setUp() {}
  void tearDown() {}

protected:
  std::string _str;
  std::vector<std::string> _result;
};

namespace
{
bool
basicPredicate(char c)
{
  return c == ' ' || c == '/';
}
}

TEST_F(MessageWrapperTest, testTokenizerEmptyString)
{
  _str = "";
  auto it = MessageWrapper::getTokenEnd(_str.cbegin(), _str.cend(), basicPredicate);
  ASSERT_EQ(_str.cend(), it);
}

TEST_F(MessageWrapperTest, testTokenizerSimpleString)
{
  _str = "test string";
  auto beg = _str.cbegin();
  auto it = MessageWrapper::getTokenEnd(_str.cbegin(), _str.cend(), basicPredicate);
  ASSERT_EQ("test", std::string(_str.cbegin(), it));

  beg = it;
  it = MessageWrapper::getTokenEnd(beg, _str.cend(), basicPredicate);
  ASSERT_EQ(" ", std::string(beg, it));

  beg = it;
  it = MessageWrapper::getTokenEnd(beg, _str.cend(), basicPredicate);
  ASSERT_EQ("string", std::string(beg, it));
  ASSERT_EQ(_str.cend(), it);
}

TEST_F(MessageWrapperTest, testTokenizerMultipleDelimiters)
{
  _str = "test / string";
  auto beg = _str.cbegin();
  auto it = MessageWrapper::getTokenEnd(_str.cbegin(), _str.cend(), basicPredicate);
  ASSERT_EQ("test", std::string(_str.cbegin(), it));

  beg = it;
  it = MessageWrapper::getTokenEnd(beg, _str.cend(), basicPredicate);
  ASSERT_EQ(" ", std::string(beg, it));

  beg = it;
  it = MessageWrapper::getTokenEnd(beg, _str.cend(), basicPredicate);
  ASSERT_EQ("/", std::string(beg, it));

  beg = it;
  it = MessageWrapper::getTokenEnd(beg, _str.cend(), basicPredicate);
  ASSERT_EQ(" ", std::string(beg, it));

  beg = it;
  it = MessageWrapper::getTokenEnd(beg, _str.cend(), basicPredicate);
  ASSERT_EQ("string", std::string(beg, it));
  ASSERT_EQ(_str.cend(), it);
}

TEST_F(MessageWrapperTest, testWrapperEmptyString)
{
  _str = "";
  _result = MessageWrapper::wrap(_str, basicPredicate);
  ASSERT_EQ(0, _result.size());
}

TEST_F(MessageWrapperTest, testWrapperSingleWordOverflow)
{
  _str = "123456789";
  _result = MessageWrapper::wrap(_str, basicPredicate, 5);
  ASSERT_EQ(2, _result.size());
  ASSERT_EQ("12345", _result[0]);
  ASSERT_EQ("6789", _result[1]);
}

TEST_F(MessageWrapperTest, testWrapperSimple)
{
  _str = "test str";
  _result = MessageWrapper::wrap(_str, basicPredicate, 5);
  ASSERT_EQ(2, _result.size());
  ASSERT_EQ("test ", _result[0]);
  ASSERT_EQ("str", _result[1]);
}

TEST_F(MessageWrapperTest, testWrapperWrapOverflow)
{
  _str = "test string";
  _result = MessageWrapper::wrap(_str, basicPredicate, 5);
  ASSERT_EQ(3, _result.size());
  ASSERT_EQ("test ", _result[0]);
  ASSERT_EQ("strin", _result[1]);
  ASSERT_EQ("g", _result[2]);
}
} // namespace tse
