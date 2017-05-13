// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------


#include "Diagnostic/DiagnosticAutoIndent.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace tse
{

TEST(DiagnosticAutoIndentTest, EmptyIndent)
{
  DiagnosticAutoIndent testedClass;
  ASSERT_STREQ("", testedClass.toString().c_str());
}

TEST(DiagnosticAutoIndentTest, OneIncrease)
{
  DiagnosticAutoIndent testedClass;
  ASSERT_STREQ("  ", (++testedClass).toString().c_str());
}

TEST(DiagnosticAutoIndentTest, DoubleIncrease)
{
  DiagnosticAutoIndent testedClass;
  ASSERT_STREQ("    ", (++(++testedClass)).toString().c_str());
}

TEST(DiagnosticAutoIndentTest, IncreaseDecrease)
{
  DiagnosticAutoIndent testedClass;
  ASSERT_STREQ("", (--(++testedClass)).toString().c_str());
}

TEST(DiagnosticAutoIndentTest, DoubleIncreaseDecrease)
{
  DiagnosticAutoIndent testedClass;
  ASSERT_STREQ("  ", (--(++(++testedClass))).toString().c_str());
}

TEST(DiagnosticAutoIndentTest, DecreaseBelowSize)
{
  DiagnosticAutoIndent testedClass;
  ASSERT_STREQ("", (--(--(++testedClass))).toString().c_str());
}

TEST(DiagnosticAutoIndentTest, CopyConstruction)
{
  DiagnosticAutoIndent testedClass;
  DiagnosticAutoIndent copyConstructedTestedClass{testedClass};
  ASSERT_STREQ("  ", copyConstructedTestedClass.toString().c_str());
}

TEST(DiagnosticAutoIndentTest, AddToString)
{
  DiagnosticAutoIndent testedClass;
  std::string dummy = "Dummy";
  ASSERT_STREQ("  Dummy", ((++testedClass) + dummy).c_str());
}

TEST(DiagnosticAutoIndentTest, AddToCharString)
{
  DiagnosticAutoIndent testedClass;
  ASSERT_STREQ("  AnotherDummy", ((++testedClass) + "AnotherDummy").c_str());
}

} //tse
