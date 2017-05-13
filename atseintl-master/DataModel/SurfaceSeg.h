//-------------------------------------------------------------------
//
//  File:        SurfaceSeg.h
//  Created:     March 8, 2004
//  Authors:
//
//  Description: Itinerary surface segment
//
//  Updates:
//          03/08/04 - VN - file created.
//
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

#pragma once

#include "Common/TseStringTypes.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
class DataHandle;

class SurfaceSeg : public TravelSeg
{
public:
  SurfaceSeg();
  virtual ~SurfaceSeg();
  virtual SurfaceSeg* clone(DataHandle& dh) const override;
};

} // tse namespace

