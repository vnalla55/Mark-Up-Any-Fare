//-------------------------------------------------------------------
//  Copyright Sabre 2010
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

#include "Common/OcTypes.h"
#include "DataModel/BaggageTravel.h"

namespace tse
{
class Diag877Collector;

struct BagValidationOpt
{
  BagValidationOpt(BaggageTravel& bt,
                   const CheckedPoint& fcp,
                   const Ts2ss& ts2ss,
                   const bool isInternational,
                   Diag877Collector* diag,
                   const CarrierCode deferTarget = CarrierCode())
    : _bt(bt),
      _fcp(fcp),
      _ts2ss(ts2ss),
      _diag(diag),
      _deferTargetCxr(deferTarget),
      _isInternational(isInternational)
  {}

  BaggageTravel& _bt;
  const CheckedPoint& _fcp;
  const Ts2ss& _ts2ss;
  Diag877Collector* _diag;
  CarrierCode _deferTargetCxr; // if empty then defer not allowed
  const bool _isInternational;
};
}
