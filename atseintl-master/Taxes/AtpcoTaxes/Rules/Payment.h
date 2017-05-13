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

#include "Common/TaxName.h"
#include "Rules/PaymentDetail.h"

#include <vector>

namespace tax
{
class Payment
{
  typedef std::vector<const PaymentDetail*> PaymentDetailVec;

public:
  Payment(TaxName const& newTaxName);
  ~Payment();

  const PaymentDetailVec& paymentDetail() const
  {
    return _PaymentDetail;
  }

  PaymentDetailVec& paymentDetail() { return _PaymentDetail; }

  const TaxName& taxName() const { return _taxName; }

  type::MoneyAmount& totalityAmt() { return _totalityAmt; }

  type::MoneyAmount const& totalityAmt() const { return _totalityAmt; }

private:
  PaymentDetailVec _PaymentDetail;

  const TaxName& _taxName;
  type::MoneyAmount _totalityAmt;
};
}

