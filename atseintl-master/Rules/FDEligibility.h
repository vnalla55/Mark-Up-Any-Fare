//----------------------------------------------------------------
//
//  File:	       FDEligibility.h
//
//  Authors:     Marco Cartolano
//  Created:     May 23, 2005
//  Description: FDEligibility class for Fare Display
//
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-----------------------------------------------------------------
#pragma once

#include "Rules/Eligibility.h"

namespace tse
{
class DiagCollector;

class FDEligibility : public Eligibility
{
public:
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      PaxTypeFare& fare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket,
                                      const bool& isQualifyingCat,
                                      const bool& isCat15Qualifying) override;

private:
  Record3ReturnTypes fdValidate(PricingTrx& trx,
                                PaxTypeFare& paxTypeFare,
                                const RuleItemInfo* rule,
                                const FareMarket& fareMarket,
                                const bool& isQualifyingCat,
                                const bool& isCat15Qualifying);

  void updateDiagnostic301(PricingTrx& trx,
                           PaxTypeFare& paxTypeFare,
                           const EligibilityInfo*& eligibilityInfo,
                           DCFactory*& factory,
                           DiagCollector*& diagPtr,
                           bool& diagEnabled);
};

} // namespace tse

