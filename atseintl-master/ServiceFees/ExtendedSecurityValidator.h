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
#include "TicketingFee/SecurityValidator.h"


namespace tse
{
class PricingTrx;
class SvcFeesSecurityInfo;

class ExtendedSecurityValidator : public SecurityValidator
{
  friend class ExtendedSecurityValidatorTest;

public:
  ExtendedSecurityValidator(PricingTrx& trx,
                            const std::vector<TravelSeg*>::const_iterator segI,
                            const std::vector<TravelSeg*>::const_iterator segIE);
  virtual ~ExtendedSecurityValidator();

protected:
  StatusT183Security validateSequence(const SvcFeesSecurityInfo* secInfo, bool& i) const override;

  bool checkDutyCode(const SvcFeesSecurityInfo* secInfo) const override;
  bool checkViewIndicator(const SvcFeesSecurityInfo* secInfo) const;

private:
  ExtendedSecurityValidator(const ExtendedSecurityValidator& rhs);
  ExtendedSecurityValidator& operator=(const ExtendedSecurityValidator& rhs);
};
} // tse namespace

