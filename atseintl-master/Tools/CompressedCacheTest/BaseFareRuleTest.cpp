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

#include "BaseFareRule.h"
#include "BaseFareRuleDAO.h"
#include "CompressionTestCommon.h"

void testBaseFareRuleCompressedCache ()
{
  std::cout << __FUNCTION__ << std::endl;
  tse::BaseFareRuleDAO dao;
  testTextFormat(dao, 5);
  testCache(CACHE_OPS, 0, dao);
  testCache(CACHE_OPS * 100, 0, dao);
  testMultiThreadCache(CACHE_OPS, 0, 25, dao);
  testMultiThreadCache(CACHE_OPS, 0, 50, dao);
  testMultiThreadCache(1, 0, 10, dao);
}
