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

#include "DBAccess/ConstructedFareInfoFactory.h"

#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/SITAConstructedFareInfo.h"

namespace tse
{
ConstructedFareInfo*
ConstructedFareInfoFactory::create(eConstructedFareInfoType type)
{
  ConstructedFareInfo* info(nullptr);

  switch (type)
  {
  case eConstructedFareInfo:
    info = new ConstructedFareInfo;
    break;

  case eSITAConstructedFareInfo:
    info = new SITAConstructedFareInfo;
    break;

  default:
    throw std::out_of_range("eConstructedFareInfo type invalid.");
    break;
  }

  return info;
}
}
