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

#include "Common/Tariff.h"
#include "DataModel/Common/Types.h"

namespace tax
{

class Fare
{
public:
  type::FareRuleCode const& rule() const { return _rule; }

  type::FareRuleCode& rule() { return _rule; }

  type::FareBasisCode const& basis() const { return _basis; }

  type::FareBasisCode& basis() { return _basis; }

  type::FareTypeCode const& type() const { return _type; }

  type::FareTypeCode& type() { return _type; }

  type::OneWayRoundTrip& oneWayRoundTrip() { return _oneWayRoundTrip; }

  const type::OneWayRoundTrip& oneWayRoundTrip() const { return _oneWayRoundTrip; }

  type::Directionality& directionality() { return _directionality; }

  const type::Directionality& directionality() const { return _directionality; }

  type::MoneyAmount& amount() { return _amount; }
  const type::MoneyAmount& amount() const { return _amount; }

  type::MoneyAmount& markupAmount() { return _markupAmount; }
  const type::MoneyAmount& markupAmount() const { return _markupAmount; }

  type::MoneyAmount& sellAmount() { return _sellAmount; }

  const type::MoneyAmount& sellAmount() const { return _sellAmount; }

  bool& isNetRemitAvailable() { return _isNetRemitAvailable; }

  const bool& isNetRemitAvailable() const { return _isNetRemitAvailable; }

  Tariff& tariff() { return _tariff; }

  const Tariff& tariff() const { return _tariff; }

  type::PassengerCode& outputPtc() { return _outputPtc; }
  type::PassengerCode const& outputPtc() const { return _outputPtc; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::FareBasisCode _basis;
  type::FareTypeCode _type;
  type::OneWayRoundTrip _oneWayRoundTrip {' '};
  type::Directionality _directionality;
  type::MoneyAmount _amount {0};
  type::MoneyAmount _markupAmount {0};
  type::MoneyAmount _sellAmount {0};
  bool _isNetRemitAvailable {false};
  type::FareRuleCode _rule {};
  Tariff _tariff ;
  type::PassengerCode _outputPtc;
};

} // namespace tax

