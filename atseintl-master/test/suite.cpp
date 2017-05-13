//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include <iostream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/Runner/SegFaultGuard.h"
#include "test/Runner/CommandLineParser.h"
#include "test/Runner/AtseTestRunner.h"

bool
runTests(AtseTestRunner& runner, const CommandLineParser& parser)
{
  // Run CppUnit tests
  bool isSuccess = runner.run(parser.xmlFilename(), parser.testName());

  // Run Google Test suites
  testing::GTEST_FLAG(throw_on_failure) = false;
  return testing::UnitTest::GetInstance()->total_test_case_count()
             ? isSuccess && (RUN_ALL_TESTS() == 0)
             : isSuccess;
}

int
main(int argc, char* argv[])
{
  // The following line causes Google Mock to throw an exception on failure,
  // which will be interpreted by your testing framework as a test failure.
  ::testing::InitGoogleMock(&argc, argv);
  ::testing::GTEST_FLAG(throw_on_failure) = true;

  CommandLineParser parser;
  if (!parser.parse(argc, argv))
    return -1;

  AtseTestRunner runner(parser.isJUnitFormat());

  if (parser.isList())
  {
    runner.printTestsList(std::cout);
    return 0;
  }

  bool isSuccess = false;

  if (parser.isGuard())
  {
    SegFaultGuard g;
    isSuccess = runTests(runner, parser);
  }
  else
    isSuccess = runTests(runner, parser);

  std::cout.flush();

  return isSuccess ? 0 : 1;
}
