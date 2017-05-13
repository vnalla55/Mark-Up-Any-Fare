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
#include "ServiceFees/ExtendedSecurityValidator.h"


#include <vector>

namespace tse
{
class PricingTrx;
class SvcFeesSecurityInfo;
class TravelSeg;

class AncillarySecurityValidator : public ExtendedSecurityValidator
{
  friend class AncillarySecurityValidatorTest;

public:
  AncillarySecurityValidator(PricingTrx& trx,
                             const std::vector<TravelSeg*>::const_iterator segI,
                             const std::vector<TravelSeg*>::const_iterator segIE);
  virtual ~AncillarySecurityValidator();

protected:
  bool checkDutyCode(const SvcFeesSecurityInfo* secInfo) const override;
  bool processAirlineSpecificX(const SvcFeesSecurityInfo* secInfo) const override;

private:
  AncillarySecurityValidator(const AncillarySecurityValidator& rhs);
  AncillarySecurityValidator& operator=(const AncillarySecurityValidator& rhs);
};
} // tse namespace

