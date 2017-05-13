#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"

namespace tse
{

class HiddenStopDetails
{
public:
  HiddenStopDetails()
    : _airport(), _equipment(), _departureDate(), _arrivalDate()
  {};

  virtual ~HiddenStopDetails() {};

  LocCode& airport() { return _airport; }
  const LocCode& airport() const { return _airport; }

  EquipmentType& equipment() { return _equipment; }
  const EquipmentType& equipment() const { return _equipment; }

  DateTime& departureDate() { return _departureDate; }
  const DateTime& departureDate() const { return _departureDate; }

  DateTime& arrivalDate() { return _arrivalDate; }
  const DateTime& arrivalDate() const { return _arrivalDate; }

private:
  LocCode _airport;
  EquipmentType _equipment;
  DateTime _departureDate;
  DateTime _arrivalDate;
};

} // namespace tse
