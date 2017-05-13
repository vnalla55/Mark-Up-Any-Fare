//-------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/BaggageTravel.h"
#include "FreeBagService/AllowanceStepProcessor.h"
#include "FreeBagService/ChargeStepProcessor.h"

namespace tse
{
class AllowanceStepProcessor;

class AllowanceFinder
{
public:
  AllowanceFinder(AllowanceStepProcessor& allowanceProc,
                  ChargeStepProcessor& chargeProc,
                  BaggageTravel& bt,
                  const CheckedPoint& furthestCp);

  void findAllowanceAndCharges() const;

private:
  void processWhollyWithin() const;
  void processUsDot() const;
  void processNonUsDot() const;
  void processCharges() const;

private:
  AllowanceStepProcessor& _allowanceProc;
  ChargeStepProcessor& _chargeProc;
  BaggageTravel& _bt;
  const CheckedPoint& _fcp;
  bool _isInternational = false;
};
}
