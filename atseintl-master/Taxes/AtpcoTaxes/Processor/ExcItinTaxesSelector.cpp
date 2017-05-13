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

#include "Common/MoneyUtil.h"
#include "DomainDataObjects/ItinsPayments.h"
#include "DomainDataObjects/Request.h"
#include "Processor/ExcItinTaxesSelector.h"
#include "Processor/IsOrigInCADestInCAOrUS.h"

namespace tax
{
bool
ExcItinTaxesSelector::check(const PaymentDetail& paymentDetail) const
{
  return paymentDetail.isValidForGroup(type::ProcessingGroup::Itinerary) &&
         paymentDetail.taxAmt() > doubleToAmount(0.0);
}

ExcItinTaxesSelector::ExcItinTaxesSelector(const ItinsPayments& itinsPayments,
                                           const Request& request)
{
  for (const ItinPayments& itinPayments : itinsPayments._itinPayments)
  {
    IsOrigInCADestInCAOrUS isCanadaExc(request, itinPayments.itinId());
    for (const Payment& payment : itinPayments.payments(type::ProcessingGroup::Itinerary))
    {
      for (const PaymentDetail* paymentDetail : payment.paymentDetail())
      {
        if (!check(*paymentDetail))
          continue;

        _excItinTaxInfo.emplace(paymentDetail->sabreTaxCode(),
                                tax::amountToDouble(paymentDetail->taxAmt()),
                                isCanadaExc.check());
      }
    }
  }
}

const std::set<PreviousTicketTaxInfo>&
ExcItinTaxesSelector::get() const
{
  return _excItinTaxInfo;
}
}
