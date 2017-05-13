//-------------------------------------------------------------------
//  Copyright Sabre 2013
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

namespace tse
{
class BaggageSecurityValidator : public ExtendedSecurityValidator
{
  friend class BaggageSecurityValidatorTest;

public:
  BaggageSecurityValidator(PricingTrx& trx,
                           const std::vector<TravelSeg*>::const_iterator segI,
                           const std::vector<TravelSeg*>::const_iterator segIE,
                           bool isCollectingT183Info);
  virtual ~BaggageSecurityValidator();

protected:
  virtual const Loc* getLocation() const override;

  virtual bool shouldCreateDiag() const override;
  virtual bool shouldCollectDiag() const override;

private:
  BaggageSecurityValidator(const BaggageSecurityValidator& rhs);
  BaggageSecurityValidator& operator=(const BaggageSecurityValidator& rhs);

  bool _isCollectingT183Info;
};
} // tse namespace

