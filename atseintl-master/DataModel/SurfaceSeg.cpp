//-------------------------------------------------------------------
//
//  File:        SurfaceSeg.cpp
//  Created:
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

#include "DataModel/SurfaceSeg.h"

#include "DBAccess/DataHandle.h"

#include <iostream>

using namespace std;
using namespace tse;

SurfaceSeg::SurfaceSeg() {}

SurfaceSeg::~SurfaceSeg() {}

SurfaceSeg*
SurfaceSeg::clone(DataHandle& dh) const
{
  SurfaceSeg* ss = dh.create<SurfaceSeg>();
  *ss = *this;
  return ss;
}
