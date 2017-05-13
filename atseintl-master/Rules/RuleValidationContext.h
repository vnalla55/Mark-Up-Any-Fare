//----------------------------------------------------------------------------
//
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
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/FlexFares/Types.h"

namespace tse
{

class PricingTrx;
class PaxTypeFare;
class FareUsage;

struct RuleValidationContext
{
  PricingTrx* _trx;
  PaxTypeFare* _paxTypeFare;
  FareUsage* _fareUsage;

  flexFares::GroupId _groupId;

  enum ContextType
  {
    FARE_MARKET = 0,
    PU_FP
  };
  ContextType _contextType;

  RuleValidationContext()
    : _trx(nullptr), _paxTypeFare(nullptr), _fareUsage(nullptr), _groupId(0u), _contextType(FARE_MARKET)
  {
  }
};
}

