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

#include "Common/RangeUtils.h"
#include "DataModel/Common/CodeOps.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/YqYr.h"
#include "DomainDataObjects/YqYrPath.h"
#include "Rules/PaymentDetail.h"
#include "Rules/ServiceBaggageApplicator.h"
#include "Rules/ServiceBaggageRule.h"

namespace tax
{
ServiceBaggageApplicator::ServiceBaggageApplicator(
    ServiceBaggageRule const* serviceBaggageRule,
    std::vector<YqYr> const* yqYrs,
    YqYrPath const* yqYrPath,
    GeoPathMapping const* yqYrMappings,
    std::shared_ptr<ServiceBaggage const> serviceBaggage,
    RawPayments& rawPayments)
  : BusinessRuleApplicator(serviceBaggageRule),
    _yqYrs(yqYrs),
    _yqYrPath(yqYrPath),
    _yqYrMappings(yqYrMappings),
    _serviceBaggage(serviceBaggage),
    _rawPayments(rawPayments),
    _rule(serviceBaggageRule)
{
}

bool
ServiceBaggageApplicator::matchTaxRange(PaymentDetail const& paymentDetail,
                                        RawPayment const& rawPayment)
{
  if (rawPayment.taxName->taxPointTag() == type::TaxPointTag::Sale)
    return true;

  if (rawPayment.taxName->taxPointTag() == type::TaxPointTag::Delivery)
    return false;

  ProperRange paymentRange(paymentDetail.getTaxPointBegin().id(),
                           paymentDetail.getTaxPointEnd().id());

  ProperRange rawRange(rawPayment.detail.getTaxPointBegin().id(),
                       rawPayment.detail.getTaxPointEnd().id());

  return paymentRange <= rawRange;
}

bool
ServiceBaggageApplicator::match(PaymentDetail const& paymentDetail, RawPayment const& rawPayment)
{
  if (&paymentDetail == &rawPayment.detail) // self-comparison: must never match
    return false;

  if (rawPayment.detail.isFailed()) // failed taxs do not apply
    return false;

  if (rawPayment.detail.isExempt()) // treat exempt as failed
    return false;

  if (!matchTaxRange(paymentDetail, rawPayment)) // paymentDetail must be contained in rawDetail
    return false;

  return true;
}

bool
ServiceBaggageApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (!_serviceBaggage && std::numeric_limits<type::Index>::max() != _rule->itemNo())
  {
    return false;
  }

  if (!_serviceBaggage || _serviceBaggage->entries.size() == 0)
  {
    return false;
  }

  if (_yqYrPath && processYqYrs(paymentDetail))
  {
    return true;
  }

  if (!_rawPayments.empty())
  {
    std::vector<const RawPayment*> matchingRawPayments;
    for (const RawPayment& rawPayment : _rawPayments)
    {
      if (match(paymentDetail, rawPayment))
      {
        matchingRawPayments.push_back(&rawPayment);
      }
    }
    if (matchingRawPayments.size() == 0)
    {
      paymentDetail.getMutableItineraryDetail().setFailedRule(_rule);
      return !paymentDetail.areAllOptionalServicesFailed();
    }

    for (ServiceBaggageEntry const& entry : _serviceBaggage->entries)
    {
      for (RawPayment const* rawPayment : matchingRawPayments)
      {
        if (matches(entry.taxCode, rawPayment->taxName->taxCode()) &&
            matches(entry.taxTypeSubcode, rawPayment->taxName->taxType()))
        {
          if (entry.applTag == type::ServiceBaggageAppl::Positive)
          {
            return true;
          }
          else if (entry.applTag == type::ServiceBaggageAppl::Negative)
          {
            paymentDetail.getMutableItineraryDetail().setFailedRule(_rule);
            return !paymentDetail.areAllOptionalServicesFailed();
          }
        }
      }
    }
  }

  paymentDetail.getMutableItineraryDetail().setFailedRule(_rule);
  return !paymentDetail.areAllOptionalServicesFailed();
}

bool
ServiceBaggageApplicator::processYqYrs(PaymentDetail& paymentDetail) const
{
  std::vector<YqYr> const& yqYrs = *_yqYrs;

  for (size_t usageIdx = 0; usageIdx < _yqYrPath->yqYrUsages().size(); ++usageIdx)
  {
    YqYrUsage const& yqYrUsage = _yqYrPath->yqYrUsages()[usageIdx];
    Mapping const& mapping = _yqYrMappings->mappings()[usageIdx];
    type::Index yqYrStartIndex = mapping.maps().front().index();
    type::Index yqYrEndIndex = mapping.maps().back().index();

    if ((paymentDetail.getTaxPointBegin().loc().tag() == type::TaxPointTag::Departure &&
         (yqYrEndIndex > paymentDetail.getTaxPointEnd().id() ||
          yqYrStartIndex < paymentDetail.getTaxPointBegin().id())) ||
        (paymentDetail.getTaxPointBegin().loc().tag() == type::TaxPointTag::Arrival &&
         (yqYrEndIndex > paymentDetail.getTaxPointBegin().id() ||
          yqYrStartIndex < paymentDetail.getTaxPointEnd().id())))
    {
      continue;
    }

    YqYr const& yqYr = yqYrs[yqYrUsage.index()];

    int16_t matchLevel = 0;

    for (const ServiceBaggageEntry& entry : _serviceBaggage->entries)
    {
      bool match = (entry.applTag == type::ServiceBaggageAppl::Positive);
      if ((entry.taxCode.empty() || entry.taxCode == yqYr.code()) &&
          (entry.taxTypeSubcode.empty() || compareYqYrType(entry.taxTypeSubcode, yqYr.type())))
      {
        matchLevel = match ? 1 : -1;
        if (!entry.taxCode.empty())
        {
          matchLevel = match ? 2 : -2;
          if (!entry.taxTypeSubcode.empty())
          {
            matchLevel = match ? 3 : -3;
            break;
          }
        }
      }
    }

    if (matchLevel > 0)
    {
      return true;
    }
  }

  return false;
}

}
