//----------------------------------------------------------------------------
//          File:           CxrScheduleSegmentLocs.h
//          Description:    CxrScheduleSegmentLocs
//          Created:        11/01/2010
//          Authors:
//
//          Updates:
//
//     © 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#ifndef CXRSCHEDULESEGMENTLOCS_H
#define CXRSCHEDULESEGMENTLOCS_H

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class CxrScheduleSegmentLocs
{
public:
  CxrScheduleSegmentLocs() {}
  virtual ~CxrScheduleSegmentLocs() {}

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  std::vector<LocCode>& locCodes() { return _locCodes; }
  const std::vector<LocCode>& locCodes() const { return _locCodes; }

  virtual bool operator==(const CxrScheduleSegmentLocs& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_locCodes == rhs._locCodes));
  }

  static void dummyData(CxrScheduleSegmentLocs& obj)
  {
    obj._carrier = "ZZZ";

    obj._locCodes.push_back("aaa");
    obj._locCodes.push_back("bbb");
  }

private:
  CarrierCode _carrier;
  std::vector<LocCode> _locCodes;

public:
  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _locCodes);
  }
};
}

#endif
