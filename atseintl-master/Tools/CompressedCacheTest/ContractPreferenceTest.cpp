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

#include "ContractPreference.h"
#include "ContractPreferenceDAO.h"
#include "CompressionTestCommon.h"

void testContractPreferenceCompressedCache ()
{
  std::cout << __FUNCTION__ << std::endl;
  tse::ContractPreferenceDAO dao;
  testTextFormat(dao, 5);
  testCache(CACHE_OPS, 0, dao);
  testCache(CACHE_OPS * 100, 0, dao);
  testMultiThreadCache(CACHE_OPS, 0, 50, dao);
  testMultiThreadCache(CACHE_OPS, 0, 10, dao);
}
