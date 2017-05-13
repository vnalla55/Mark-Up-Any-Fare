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

#include "DataModel/AncRequest.h"
#include "FreeBagService/BaggageSecurityValidator.h"

namespace tse
{
class AncillaryPricingTrx;
class Itin;

class BaggageAncillarySecurityValidatorAB240 : public BaggageSecurityValidator
{
  friend class BaggageAncillarySecurityValidatorAB240Test;

public:
  BaggageAncillarySecurityValidatorAB240(AncillaryPricingTrx& trx,
                                    const std::vector<TravelSeg*>::const_iterator segI,
                                    const std::vector<TravelSeg*>::const_iterator segIE,
                                    bool isCollectingT183Info,
                                    const Indicator typeOfServiceToValidate,
                                    const Itin *itin = nullptr);

protected:
  virtual bool checkGds(const SvcFeesSecurityInfo* secInfo) const override;

private:
  BaggageAncillarySecurityValidatorAB240(const BaggageAncillarySecurityValidatorAB240& rhs);
  BaggageAncillarySecurityValidatorAB240& operator=(const BaggageAncillarySecurityValidatorAB240& rhs);
  std::string getGdsCodeFromRequest(const SvcFeesSecurityInfo* secInfo) const;
  std::string getGdsCodeFromRequestForAllowanceTypes() const;
  std::string getGdsCodeFromRequestForEmbargoes() const;
  std::string getGdsCodeFromRequestForCharges(const SvcFeesSecurityInfo* secInfo) const;
  std::string getDefaultGdsCodeFromRequest(const SvcFeesSecurityInfo* secInfo) const;

  Indicator _typeOfServiceToValidate;
  const Itin* _itin;
};
} // tse namespace

