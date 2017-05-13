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
//
//-------------------------------------------------------------------

#pragma once

#include "FreeBagService/BaggageS7SubvalidatorInterfaces.h"

namespace tse
{
class AllowanceSoftPassCollector : public IAllowanceCollector
{
public:
  AllowanceSoftPassCollector(std::vector<OCFees*>& ocFees) : _ocFees(ocFees) {}

  bool collect(OCFees& ocFees) override
  {
    _ocFees.push_back(&ocFees);
    return ocFees.bagSoftPass().isNull();
  }

private:
  std::vector<OCFees*>& _ocFees;
};
}
