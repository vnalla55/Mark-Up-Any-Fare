//----------------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "DataModel/PaxTypeFare.h"

namespace tse
{
using Ts2ss = std::map<TravelSeg*, std::pair<PaxTypeFare::SegmentStatus*, PaxTypeFare*>>;
}
