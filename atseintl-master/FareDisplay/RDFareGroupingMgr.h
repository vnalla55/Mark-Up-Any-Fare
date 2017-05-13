//-------------------------------------------------------------------
//  File:        RDFareGroupingMgr.h
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

#include "FareDisplay/FareGroupingMgr.h"


#include <string>
#include <vector>

namespace tse
{
class FareDisplayTrx;
class PaxTypeFare;
class FareDisplayOptions;

class RDFareGroupingMgr : public FareGroupingMgr
{
  friend class FareGroupingMgrTest;

public:
  /** implementation of groupFares() interface for RD type entries.*/
  bool groupFares(FareDisplayTrx& trx) override;

private:
  static bool isNotRequestedFBR(FareDisplayTrx&, std::string&);
  static void groupFaresByMultiAirport(std::vector<PaxTypeFare*>& faresToMerge,
                                       const FareDisplayOptions* fdo,
                                       FareDisplayTrx& trx);
};
}
