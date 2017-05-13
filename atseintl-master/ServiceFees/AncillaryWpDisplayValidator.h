//-------------------------------------------------------------------
//  Copyright Sabre 2011
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

#include "ServiceFees/OptionalServicesValidator.h"

namespace tse
{
class AncillaryWpDisplayValidator : public OptionalServicesValidator
{
  friend class AncillaryWpDisplayValidatorTest;

public:
  using OptionalServicesValidator::OptionalServicesValidator;

private:
  bool setPBDate(const OptionalServicesInfo& optSrvInfo,
                 OCFees& ocFees,
                 const DateTime& pbDate) const override;
  bool checkInterlineIndicator(const OptionalServicesInfo& optSrvInfo) const override;
};
} // tse namespace

