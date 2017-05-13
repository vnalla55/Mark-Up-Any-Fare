//-------------------------------------------------------------------
//  Copyright Sabre 2004
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

#ifndef MockTravelSeg_H
#define MockTravelSeg_H


#include "DataModel/TravelSeg.h"

namespace tse
{
class DataHandle;

class MockTravelSeg : public TravelSeg
{
public:
  MockTravelSeg* clone(DataHandle& dh) const { return 0; }
};
} // tse namespace

#endif // ifndef MockTravelSeg_H
