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

#include "SITAFareInfo.h"
#include "FareDAO.h"
#include "CompressionTestCommon.h"

void testFareCompressedCache()
{
  std::cout << __FUNCTION__ << std::endl;
  tse::FareDAO dao;
  testTextFormat(dao, 5);
  testCache(CACHE_OPS, CACHE_OPS * 6, dao);
  testCache(CACHE_OPS * 6, CACHE_OPS * 6, dao);
  //testCache(CACHE_OPS * 6, CACHE_OPS * 6, dao);
  //testMultiThreadCache(CACHE_OPS, CACHE_OPS * 6, 50, dao);
  //testMultiThreadCache(CACHE_OPS, CACHE_OPS * 6, 50, dao);
  testMultiThreadCache(CACHE_OPS, CACHE_OPS * 6, 10, dao);
}
