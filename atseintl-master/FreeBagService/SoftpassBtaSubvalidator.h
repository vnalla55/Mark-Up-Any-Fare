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

#include "FreeBagService/RegularBtaSubvalidator.h"

namespace tse
{

class SoftpassBtaSubvalidator final : public RegularBtaSubvalidator
{
public:
  using RegularBtaSubvalidator::RegularBtaSubvalidator;

  StatusS7Validation validate(const OptionalServicesInfo& s7, OCFees& ocFees) override;
};

}
