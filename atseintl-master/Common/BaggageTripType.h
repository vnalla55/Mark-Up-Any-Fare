//-------------------------------------------------------------------
//  Copyright Sabre 2014
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

#include <stdint.h>

namespace tse
{
class Itin;
class BaggageTripType
{
public:
  enum Enum
  {
    TO_FROM_US = 1,
    TO_FROM_CA,
    WHOLLY_WITHIN_US,
    WHOLLY_WITHIN_CA,
    BETWEEN_US_CA,
    OTHER
  };

  BaggageTripType() : _btt(OTHER) {}
  BaggageTripType(const Enum& btt) : _btt(btt) {}

  bool operator==(const Enum& btt) const { return _btt == btt; }
  Enum getRawValue() const { return static_cast<Enum>(_btt); }

  bool isUsDot() const { return _btt != OTHER; }
  bool isWhollyWithinUsOrCa() const { return _btt == WHOLLY_WITHIN_US || _btt == WHOLLY_WITHIN_CA; }
  bool usDotCarrierApplicable() const { return _btt == TO_FROM_US || _btt == BETWEEN_US_CA; }
  bool ctaCarrierApplicable() const { return _btt == TO_FROM_CA || _btt == BETWEEN_US_CA; }

  char getIndicator() const { return static_cast<char>('0' + _btt); }
  const char* getJourneyName(const Itin& itin) const;

private:
  uint8_t _btt;
};
}

