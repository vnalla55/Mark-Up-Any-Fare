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

#include "DomainDataObjects/YqYr.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/ServiceBaggageService.h"
#include "Rules/TaxOnYqYrApplicator.h"
#include "Rules/TaxOnYqYrRule.h"

namespace tax
{
namespace
{
bool
match(const boost::ptr_vector<ServiceBaggageEntry>& entries, const TaxableYqYr& yqYr)
{
  for (const ServiceBaggageEntry& entry : entries)
  {
    if ((entry.taxCode.empty() || entry.taxCode == yqYr._code) &&
        (entry.taxTypeSubcode.empty() || compareYqYrType(entry.taxTypeSubcode, yqYr._type)))
    {
      return entry.applTag == type::ServiceBaggageAppl::Positive;
    }
  }

  return false;
}

void
failUnmatchedYqYrs(const TaxOnYqYrRule& rule,
                   const bool checkServiceBaggage,
                   const ServiceBaggage* const serviceBaggage,
                   TaxableYqYrs& yqYrDetails,
                   type::Index paymentBegin,
                   type::Index paymentEnd,
                   bool isVatTax)
{
  const std::vector<TaxableYqYr>& yqYrs = yqYrDetails._subject;
  for (size_t id = 0; id < yqYrs.size(); ++id)
  {
    if (yqYrDetails.isFailedRule(id))
      continue;

    const TaxableYqYr& yqYr = yqYrs[id];

    if (isVatTax && yqYr._taxIncluded)
    {
      yqYrDetails.setFailedRule(id, rule);
      continue;
    }

    Range yqYrRange(yqYrDetails._ranges[id].first, yqYrDetails._ranges[id].second);
    ProperRange calcRange(paymentBegin, paymentEnd);

    bool exactMatch = (rule.taxAppliesToTagInd() == type::TaxAppliesToTagInd::LowestFromFareList) ||
                      (rule.taxAppliesToTagInd() == type::TaxAppliesToTagInd::BetweenFareBreaks);

    if (exactMatch && !(yqYrRange <= calcRange))
    {
      yqYrDetails.setFailedRule(id, rule);
      continue;
    }

    if (checkServiceBaggage && !match(serviceBaggage->entries, yqYr))
      yqYrDetails.setFailedRule(id, rule);
  }
}
}

TaxOnYqYrApplicator::TaxOnYqYrApplicator(TaxOnYqYrRule const& rule,
                                         std::shared_ptr<ServiceBaggage const> serviceBaggage,
                                         bool restrictServiceBaggage)
  : BusinessRuleApplicator(&rule),
    _rule(rule),
    _serviceBaggage(serviceBaggage),
    _restrictServiceBaggage(restrictServiceBaggage)
{
}

bool
TaxOnYqYrApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (!_serviceBaggage && _restrictServiceBaggage)
    return false;

  failUnmatchedYqYrs(_rule,
                     _restrictServiceBaggage && _serviceBaggage,
                     _serviceBaggage.get(),
                     paymentDetail.getMutableYqYrDetails(),
                     paymentDetail.getTaxPointBegin().id(),
                     paymentDetail.getTaxPointEnd().id(),
                     paymentDetail.isVatTax());

  return !paymentDetail.isFailed();
}
} // namespace tax
