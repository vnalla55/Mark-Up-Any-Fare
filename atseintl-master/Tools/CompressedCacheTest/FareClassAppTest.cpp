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

#include "FareClassAppInfo.h"
#include "FareClassAppDAO.h"
#include "CompressionTestCommon.h"

void disposeFCA()
{
  const tse::FareClassAppInfo& entry(tse::getRef<const tse::FareClassAppInfo>());
  const_cast<tse::FareClassAppInfo&>(entry).destroyChildren();
}

void testFCACompressedCache()
{
  //const tse::FareClassCodeC test("testABCD");
  //const tse::FareClassCodeC& cpy(test);
  std::cout << __FUNCTION__ << std::endl;
  tse::FareClassAppDAO dao;
  testTextFormat(dao, 5);
  testCache(CACHE_OPS, 0, dao);
  testCache(CACHE_OPS * 100, 0, dao);
  //testCache(CACHE_OPS * 6, CACHE_OPS * 6, dao);
  testMultiThreadCache(CACHE_OPS, 0, 50, dao);
  //testMultiThreadCache(CACHE_OPS, 0, 10, dao);
  //testMultiThreadCache(1, CACHE_OPS * 6, 10, dao);
}
