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

#include "GeneralFareRuleInfo.h"
#include "GeneralFareRuleDAO.h"
#include "CompressionTestCommon.h"

void testGeneralFareRuleCompressedCache()
{
  std::cout << __FUNCTION__ << std::endl;
  tse::GeneralFareRuleDAO dao;
  testTextFormat(dao, 3);
  //testCache(1, 0, dao);
  testCache(CACHE_OPS, 0, dao);
  testCache(CACHE_OPS * 100, 0, dao);
  //testCache(1, 0, dao);
  testMultiThreadCache(CACHE_OPS, 0, 50, dao);
  //testMultiThreadCache(0, CACHE_OPS * 6, 10, dao);
  //testMultiThreadCache(1, 0, 50, dao);
}
