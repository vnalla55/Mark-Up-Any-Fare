//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/test/prg_exec_monitor.hpp>
#include <boost/test/execution_monitor.hpp>
#include <boost/test/utils/basic_cstring/io.hpp>
#include <boost/test/debug.hpp>

#include <chrono>

void testSpinlock();
void matchFareClass();
void boostString();
void testAddonCombFareClassHistoricalCache();
void testBaseFareRuleCompressedCache();
void testCombinabilityRuleCompressedCache();
void testContractPreferenceCompressedCache();
void testFCACompressedCache();
void testFareCompressedCache();
void testFootNoteCtrlCompressedCache();
void testGeneralFareRuleCompressedCache();
void testGeneralFareRuleSimpleCache();
void testMUCCompressedCache();
void testNegFareRestCompressedCache();
void testOptionalServicesCompressedCache();
void testAddonFareCompressedCache();
void disposeFCA();

uint64_t getCurrentMillis()
{
  static std::chrono::steady_clock::time_point start(std::chrono::steady_clock::now());
  std::chrono::steady_clock::time_point now(std::chrono::steady_clock::now());
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

struct program
{
  int operator()()
  {
    //testSpinlock();
    matchFareClass();
    boostString();
    testCombinabilityRuleCompressedCache();
    testAddonFareCompressedCache();
    testAddonCombFareClassHistoricalCache();
    testBaseFareRuleCompressedCache();
    testContractPreferenceCompressedCache();
    testFCACompressedCache();
    testFareCompressedCache();
    testFootNoteCtrlCompressedCache();
    testGeneralFareRuleCompressedCache();
    testGeneralFareRuleSimpleCache();
    testMUCCompressedCache();
    testNegFareRestCompressedCache();
    testOptionalServicesCompressedCache();
    return 1;
  }
};

int main()
{
  boost::uint64_t start(getCurrentMillis());
  boost::debug::detect_memory_leaks(true);
  boost::execution_monitor monitor;
  try
  {
    for (int i = 0; i < 1; ++i)
    {
      monitor.execute(program());
    }
  }
  catch (boost::execution_exception const& e)
  {
    std::cout << "caught exception: " << e.what() << std::endl;
  }
  disposeFCA();
  //new int[1000];
  //boost::debug::break_memory_alloc(0);
  std::cout << __FUNCTION__ << " time:" << (getCurrentMillis() - start) << std::endl;
  return 0;
}
