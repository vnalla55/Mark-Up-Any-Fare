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

#include <vector>

#include "DomainDataObjects/FareUsage.h"
#include "DataModel/Services/PassengerTypeCode.h"

namespace tax
{

class FarePath
{
public:
  FarePath(void);
  ~FarePath(void);

  std::vector<FareUsage>& fareUsages() { return _fareUsages; }
  std::vector<FareUsage> const& fareUsages() const { return _fareUsages; }

  type::MoneyAmount& totalAmount() { return _totalAmount; }
  type::MoneyAmount const& totalAmount() const { return _totalAmount; }

  type::MoneyAmount& totalMarkupAmount() { return _totalMarkupAmount; }
  type::MoneyAmount const& totalMarkupAmount() const { return _totalMarkupAmount; }

  type::MoneyAmount& totalAmountBeforeDiscount() { return _totalAmountBeforeDiscount; }
  type::MoneyAmount const& totalAmountBeforeDiscount() const { return _totalAmountBeforeDiscount; }

  type::CarrierCode& validatingCarrier() { return _validatingCarrier; }
  type::CarrierCode const& validatingCarrier() const { return _validatingCarrier; }

  type::PassengerCode& outputPtc() { return _outputPtc; }
  type::PassengerCode const& outputPtc() const { return _outputPtc; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  std::vector<FareUsage> _fareUsages;
  type::MoneyAmount _totalAmount;
  type::MoneyAmount _totalMarkupAmount;
  type::MoneyAmount _totalAmountBeforeDiscount;
  type::CarrierCode _validatingCarrier;
  type::PassengerCode _outputPtc;
};
}
