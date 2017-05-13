#pragma once
#include "Common/Code.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

/* Classes that represent FlightCount Information . */

class FlightCount
{
public:
  FlightCount() = default;
  FlightCount(CarrierCode cxr) : _carrier(cxr) {}
  uint16_t _nonStop = 0;
  uint16_t _direct = 0;
  uint16_t _onlineConnection = 0;
  bool _isDirectCarrier = false;
  bool _interLineServiceExist = false;
  CarrierCode _carrier;
  bool equalCxr(CarrierCode cxr) const { return _carrier == cxr; }
};
}
