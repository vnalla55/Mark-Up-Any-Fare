#include <dlfcn.h>

#include <iostream>

#include <cppunit/plugin/DynamicLibraryManagerException.h>
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

bool
execute(CppUnit::PlugInManager& plugInManager, const CommandLineParser& parser)
{
  const std::vector<std::string>& list = parser.libNames();
  typedef std::vector<std::string>::const_iterator It;
  for (It i = list.begin(); i != list.end(); ++i)
    plugInManager.load(i->c_str());

  AtseTestRunner runner(plugInManager, parser.isJUnitFormat());

  if (parser.isList())
  {
    runner.printTestsList(std::cout);
    return true;
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

  return isSuccess;
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

  // gTest test cases are stored in a static field testing::UnitTest::instance,
  // so their destructors are called after this main. When PlugInManager
  // is destructed it unloads libraries that contain gTests destructors definitions.
  // If PlugInManager is destroyed before gTests segmentation fault occurs,
  // because we try to call a method under an address that is no longer available.
  // Making PlugInManager a pointer and not deleting it is to prevent calling
  // PlugInManager destructor before destructing gTests.
  CppUnit::PlugInManager* mgr = new CppUnit::PlugInManager();

  try { return execute(*mgr, parser) ? 0 : 1; }
  catch (CppUnit::DynamicLibraryManagerException& e)
  {
    // This exception's what() method doesn't provide enough information.
    // We assume it uses dlopen() under the hood and get more useful information
    // from dlerror() function.
    std::cerr << "Exception: " << e.what() << '\n';

    const char* error = dlerror();
    if (error)
      std::cerr << "Reason: " << error << '\n';
  }
  catch (std::exception& e) { std::cerr << "Exception: " << e.what() << '\n'; }
  return -2;
}
