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

void testGeneralFareRuleSimpleCache ()
{
  std::cout << __FUNCTION__ << std::endl;
  tse::GeneralFareRuleDAO dao;
  testTextFormat(dao, 5);
  testSimpleCache(CACHE_OPS, dao);
  testMultiThreadSimpleCache(CACHE_OPS, 50, dao);
  testMultiThreadSimpleCache(CACHE_OPS, 10, dao);
}
