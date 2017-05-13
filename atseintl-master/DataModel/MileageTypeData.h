//-------------------------------------------------------------------
//  Copyright Sabre 2007
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
#include "Common/TseCodeTypes.h"

namespace tse
{
class TravelSeg;

class MileageTypeData
{
public:
  enum MileageType : uint8_t
  {
    UNKNOWN = 0,
    EQUALIZATION = 2,
    TICKETEDPOINT = 3
  };

  MileageTypeData() : _travelSeg(nullptr), _type(UNKNOWN) {}

  MileageType& type() { return _type; }
  const MileageType& type() const { return _type; }

  LocCode& city() { return _city; }
  const LocCode& city() const { return _city; }

  TravelSeg*& travelSeg() { return _travelSeg; }
  const TravelSeg* travelSeg() const { return _travelSeg; }

private:
  TravelSeg* _travelSeg;
  LocCode _city;
  MileageType _type;
};
} // tse namespace
