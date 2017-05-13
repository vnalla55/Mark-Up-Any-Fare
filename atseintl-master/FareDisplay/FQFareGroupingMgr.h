//-------------------------------------------------------------------
//  File:        FQFareGroupingMgr.h
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseStringTypes.h"
#include "FareDisplay/FareGroupingMgr.h"
#include "Routing/RoutingConsts.h"


namespace tse
{

/**
*   @class FareGroupingMgr
*
*   Description:
*   FareGroupingMgr selects the appropriate Strategy to build the FareGroups.
*
*/
class FQFareGroupingMgr : public FareGroupingMgr
{
  friend class FareGroupingMgrTest;

public:
  bool groupFares(FareDisplayTrx& trx) override;
};
}
