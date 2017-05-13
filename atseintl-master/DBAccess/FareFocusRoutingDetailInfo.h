#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{


class FareFocusRoutingDetailInfo
{
 public:
  FareFocusRoutingDetailInfo()
    : _routingType(' ')
  {
  }

  RoutingNumber& routingNo() { return _routingNo; }
  const RoutingNumber& routingNo() const { return _routingNo; }

  Indicator& routingType() { return _routingType; }
  Indicator routingType() const { return _routingType; }

  bool operator==(const FareFocusRoutingDetailInfo& rhs) const
  {
    return _routingNo == rhs._routingNo
           && _routingType == rhs._routingType;
  }

  static void dummyData(FareFocusRoutingDetailInfo& obj)
  {
    obj._routingNo = "123456789012";
    obj._routingType = 'A';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _routingNo);
    FLATTENIZE(archive, _routingType);
  }

 private:
  RoutingNumber _routingNo;
  Indicator _routingType;
};

}// tse

