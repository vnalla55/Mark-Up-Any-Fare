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
#include "DataModel/Services/PreviousTicketTaxInfo.h"

#include <set>
#include <algorithm>

namespace tax
{
class ItinsPayments;
class PaymentDetail;
class Request;

class ExcItinTaxesSelector
{
  std::set<PreviousTicketTaxInfo> _excItinTaxInfo;

public:
  ExcItinTaxesSelector(const ItinsPayments& itinsPayments, const Request& request);
  bool check(const PaymentDetail& paymentDetail) const;
  const std::set<PreviousTicketTaxInfo>& get() const;
};
}
