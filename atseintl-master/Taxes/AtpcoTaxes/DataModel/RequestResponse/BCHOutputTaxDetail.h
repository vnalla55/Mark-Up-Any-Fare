// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

class BCHOutputTaxDetail
{
public:
  BCHOutputTaxDetail() {}

  BCHOutputTaxDetail& operator=(const BCHOutputTaxDetail&) = delete;

  void setId(type::Index id) { _id = id; }
  const type::Index& getId() const { return _id; }

  void setSabreCode(const type::SabreTaxCode sabreCode) { _sabreCode = sabreCode; }
  const type::SabreTaxCode& getSabreCode() const { return _sabreCode; }

  void setPaymentAmt(const type::MoneyAmount& paymentAmt) { _paymentAmt = paymentAmt; }
  const type::MoneyAmount& getPaymentAmt() const { return _paymentAmt; }

  void setName(const type::TaxLabel& name) { _name = name; }
  const type::TaxLabel& getName() const { return _name; }

private:
  type::Index _id{0};
  type::SabreTaxCode _sabreCode;
  type::MoneyAmount _paymentAmt;
  type::TaxLabel _name;
};

} // namespace tax
