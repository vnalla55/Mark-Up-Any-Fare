//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
struct FreqFlyerStatus
{
  CarrierCode _carrier;
  FreqFlyerTierLevel _level = 0;
  DateTime _createDate;
  DateTime _effectiveDate;
  DateTime _expireDate;
  DateTime _discDate;
  std::string _statusLevel;
  uint16_t _maxPassengersSamePNR = 0;
  uint16_t _maxPassengersDiffPNR = 0;

  // getters for IsEffective* predicates
  const DateTime& createDate() const { return _createDate; }
  const DateTime& effDate() const { return _effectiveDate; }
  const DateTime& expireDate() const { return _expireDate; }
  const DateTime& discDate() const { return _discDate; }

  bool operator==(const FreqFlyerStatus& other) const
  {
    return other._carrier == _carrier && other._level == _level &&
           _createDate == other._createDate && _effectiveDate == other._effectiveDate &&
           _expireDate == other._expireDate && _discDate == other._discDate &&
           _statusLevel == other._statusLevel &&
           _maxPassengersSamePNR == other._maxPassengersSamePNR &&
           _maxPassengersDiffPNR == other._maxPassengersDiffPNR;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _level);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effectiveDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _statusLevel);
    FLATTENIZE(archive, _maxPassengersSamePNR);
    FLATTENIZE(archive, _maxPassengersDiffPNR);
  }
};
}
