//-------------------------------------------------------------------
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DBAccess/FareInfoFactory.h"

#include "DBAccess/FareInfo.h"
#include "DBAccess/SITAFareInfo.h"

namespace tse
{
FareInfo*
FareInfoFactory::create(eFareInfoType type)
{
  FareInfo* info(nullptr);

  switch (type)
  {
  case eFareInfo:
    info = new FareInfo;
    break;

  case eSITAFareInfo:
    info = new SITAFareInfo;
    break;

  default:
    throw std::out_of_range("eFareInfo type invalid.");
    break;
  }

  return info;
}
}
