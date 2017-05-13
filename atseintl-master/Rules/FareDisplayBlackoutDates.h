//-------------------------------------------------------------------
//
//  File:        FareDisplayBlackoutDates.h
//  Authors:     Lipika Bardalai
//  Created:     March 2005
//  Description: This class contains blackout dates validation
//               for Fare Display, reusing existing functionality.
//               Method - process is overidden to reuse functionality
//               from base class and alse adds new semantics for
//               Fare Display.
//
//  Copyright Sabre 2001
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//---------------------------------------------------------------------
#pragma once

#include "Rules/BlackoutDates.h"

namespace tse
{

class FareDisplayBlackoutDates : public BlackoutDates
{
public:
  virtual IsDateBetween* createDateBetween() override;
  virtual Record3ReturnTypes
  process(PaxTypeFare& paxTypeFare, PricingTrx& trx, bool isInbound) override;
  virtual Predicate* getDateBetweenPredicate(const BlackoutInfo& info) override;
};
}
