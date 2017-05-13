//-------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include "FreeBagService/BaggageSecurityValidator.h"

namespace tse
{
class AncillaryPricingTrx;

class BaggageAncillarySecurityValidator : public BaggageSecurityValidator
{
  friend class BaggageAncillarySecurityValidatorTest;

public:
  BaggageAncillarySecurityValidator(AncillaryPricingTrx& trx,
                                    const std::vector<TravelSeg*>::const_iterator segI,
                                    const std::vector<TravelSeg*>::const_iterator segIE,
                                    bool isCollectingT183Info,
                                    bool validateCharges);
  virtual ~BaggageAncillarySecurityValidator();

protected:
  virtual bool checkGds(const SvcFeesSecurityInfo* secInfo) const override;

private:
  BaggageAncillarySecurityValidator(const BaggageAncillarySecurityValidator& rhs);
  BaggageAncillarySecurityValidator& operator=(const BaggageAncillarySecurityValidator& rhs);

  bool _validateCharges;
};
} // tse namespace

