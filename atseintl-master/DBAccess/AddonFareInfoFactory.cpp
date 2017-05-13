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

#include "DBAccess/AddonFareInfoFactory.h"

#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "DBAccess/SITAAddonFareInfo.h"

namespace tse
{
AddonFareInfo*
AddonFareInfoFactory::create(eAddonFareInfoType type)
{
  AddonFareInfo* info(nullptr);

  switch (type)
  {
  case eAddonFareInfo:
    info = new AddonFareInfo;
    break;

  case eSITAAddonFareInfo:
    info = new SITAAddonFareInfo;
    break;

  case eFDAddonFareInfo:
    info = new FDAddOnFareInfo;
    break;

  default:
    throw std::out_of_range("eAddonFareInfo type invalid.");
    break;
  }

  return info;
}
}
