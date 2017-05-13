// ---------------------------------------------------------------------------
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------
#ifndef APPLICATION_FEE_H
#define APPLICATION_FEE_H

#include <vector>
#include <list>
#include "Common/TseStringTypes.h"
#include "DBAccess/YQYRFees.h"

namespace tse
{
class AirSeg;

// ----------------------------------------------------------------------------
// <PRE>
//
// @class ApplicationFee
// Description: Work Class for ATPCO passses Fuel and Insurance Fees
//
// When yqyrFees() is NULL, denotes an unassigned portion of travel
// A sector of travel will only have one seg in the vector
// </PRE>
// ----------------------------------------------------------------------------
class ApplicationFee
{

public:
  ApplicationFee() : _yqyrFees(nullptr), _segmentOrderBegin(0), _segmentOrderEnd(0) {};

  virtual ~ApplicationFee() {};

  YQYRFees*& yqyrFees() { return _yqyrFees; }

  std::vector<AirSeg*>& segs() { return _segs; }
  std::vector<AirSeg*>::iterator begin() { return _segs.begin(); }
  std::vector<AirSeg*>::iterator end() { return _segs.end(); }

  uint16_t& segmentOrderBegin() { return _segmentOrderBegin; }
  const uint16_t& segmentOrderBegin() const { return _segmentOrderBegin; }
  uint16_t& segmentOrderEnd() { return _segmentOrderEnd; }
  const uint16_t& segmentOrderEnd() const { return _segmentOrderEnd; }

private:
  YQYRFees* _yqyrFees;
  std::vector<AirSeg*> _segs;
  uint16_t _segmentOrderBegin;
  uint16_t _segmentOrderEnd;

  ApplicationFee(const ApplicationFee& mi);
  ApplicationFee& operator=(const ApplicationFee& mi);
};
}
#endif
