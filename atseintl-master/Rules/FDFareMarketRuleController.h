//----------------------------------------------------------------
//
//  File:              FDFareMarketRuleController.h
//
//  Author:      Daryl Champagne
//  Created:     April 5, 2006
//  Description: FDFareMarketRuleController class for Fare Display
//
//
//  Copyright Sabre 2006
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

#include "Rules/FareMarketRuleController.h"


namespace tse
{
class Itin;

class FDFareMarketRuleController : public FareMarketRuleController
{
public:
  FDFareMarketRuleController() : FareMarketRuleController() {};

  FDFareMarketRuleController(CategoryPhase phase) : FareMarketRuleController(phase) {};

  FDFareMarketRuleController(CategoryPhase phase, const std::vector<uint16_t>& categories)
    : FareMarketRuleController(phase, categories) {};

  virtual bool validate(PricingTrx& trx, Itin& itin, PaxTypeFare& paxTypeFare) override;
  using FareMarketRuleController::validate;
  //    bool validate(FareDisplayTrx& fdTrx, Itin& itin, PaxTypeFare& paxTypeFare);

  bool usedAcctOrCorp(PaxTypeFare& paxTypeFare);
  virtual bool isValidForCWTuser(PricingTrx& trx, PaxTypeFare& paxTypeFare);

};

} // namespace tse

