//-------------------------------------------------------------------
//
//  File:        ArunkSeg.cpp
//  Created:
//  Authors:
//
//  Description: Itinerary surface segment
//
//  Updates:
//          05/06/04 - VN - file created.
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

#include "DataModel/ArunkSeg.h"

#include "DBAccess/DataHandle.h"

#include <iostream>

using namespace std;
using namespace tse;

ArunkSeg::ArunkSeg() : _rtwDynamicSupplementalArunk(false) { originalId() = ARUNK_SEG_DEFAULT_ID; }

ArunkSeg::~ArunkSeg() {}

ArunkSeg*
ArunkSeg::clone(DataHandle& dh) const
{
  ArunkSeg* as = dh.create<ArunkSeg>();
  *as = *this;
  return as;
}
