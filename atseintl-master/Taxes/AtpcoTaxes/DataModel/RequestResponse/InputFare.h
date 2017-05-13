// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "DataModel/Common/Types.h"

namespace tax
{

struct InputFare
{
  type::Index _id {0};
  type::FareBasisCode _basis {};
  type::FareTypeCode _type {};
  type::OneWayRoundTrip _oneWayRoundTrip {' '};
  type::Directionality _directionality {type::Directionality::Blank};
  type::MoneyAmount _amount {0};
  type::MoneyAmount _markupAmount {0};
  type::MoneyAmount _sellAmount {0};
  bool _isNetRemitAvailable {false};
  type::FareRuleCode _rule {};
  type::FareTariff _tariff {-1};
  type::FareTariffInd _tariffInd{UninitializedCode};
  type::PassengerCode _outputPtc{UninitializedCode};
};

} // namespace tax

