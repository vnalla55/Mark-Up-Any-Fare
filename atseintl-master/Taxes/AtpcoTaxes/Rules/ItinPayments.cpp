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

#include "Rules/ItinPayments.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"
#include "Util/BranchPrediction.h"

namespace tax
{
ItinPayments::ItinPayments(type::Index newItinId) : _itinId(newItinId)
{
}

ItinPayments::~ItinPayments()
{
}

void
ItinPayments::addTaxes(const type::ProcessingGroup processingGroup,
                       PaymentsMap const& taxes,
                       Services const& /*services*/)
{
  for (PaymentsMap::value_type const& paymentForLabel : taxes)
  {
    Payment* payment = new Payment(*paymentForLabel.first);
    payments(processingGroup).push_back(payment);

    for (const PaymentDetail* pd : paymentForLabel.second)
    {
      if (pd->isCommandExempt())
      {
        payment->totalityAmt() = 0;
        payment->paymentDetail().push_back(pd);
        continue;
      }

      payment->totalityAmt() += pd->taxEquivalentAmount();
      payment->totalityAmt() += pd->taxOnChangeFeeAmount();
      for (const OptionalService& oc : pd->optionalServiceItems())
      {
        payment->totalityAmt() += oc.getTaxEquivalentAmount();
      }
      for (const TicketingFee& ticketingFee : pd->ticketingFees())
      {
        payment->totalityAmt() += ticketingFee.taxAmount();
      }
      payment->paymentDetail().push_back(pd);
    }
  }
}

void
ItinPayments::addValidTaxes(const type::ProcessingGroup processingGroup,
                            RawPayments const& taxes,
                            Services const& services)
{
  PaymentsMap map;
  for (const RawPayments::value_type& rawPayment : taxes)
  {
    if (UNLIKELY(rawPayment.detail.isValidForGroup(processingGroup)))
    {
      map[rawPayment.taxName].push_back(&rawPayment.detail);
    }
  }
  addTaxes(processingGroup, map, services);
}

void
ItinPayments::addAllTaxes(const type::ProcessingGroup processingGroup,
                          RawPayments const& taxes,
                          Services const& services)
{
  PaymentsMap map;
  for (const RawPayments::value_type& rawPayment : taxes)
  {
    map[rawPayment.taxName].push_back(&rawPayment.detail);
  }
  addTaxes(processingGroup, map, services);
}
}
