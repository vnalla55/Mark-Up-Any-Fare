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
#include "DataModel/Common/CodeOps.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxOnOptionalServiceApplicator.h"
#include "Rules/TaxOnOptionalServiceRule.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"

namespace tse
{
ATPCO_FALLBACK_DECL(ATPCO_TAX_AcceptOCTagPrePaid)
}

namespace tax
{
namespace
{

bool isFailedOc (const PaymentDetail& pd)
{
  return !pd.optionalServiceItems().empty() && // upon any OC assume an OC-only detail
         pd.areAllOptionalServicesFailed();
}

} // anonymous namespace

TaxOnOptionalServiceApplicator::TaxOnOptionalServiceApplicator(
    TaxOnOptionalServiceRule const& rule,
    std::shared_ptr<ServiceBaggage const> serviceBaggage,
    const FallbackService& fallbackService,
    bool restrictServiceBaggage)
  : BusinessRuleApplicator(&rule),
    _rule(rule),
    _serviceBaggage(serviceBaggage),
    _fallbackService(fallbackService),
    _restrictServiceBaggage(restrictServiceBaggage)
{
}

bool
TaxOnOptionalServiceApplicator::checkOptionalService(const ServiceBaggageEntry& entry,
                                                      const OptionalService& optionalService) const
{
  if(optionalService.type() != type::OptionalServiceTag::FlightRelated &&
     optionalService.type() != type::OptionalServiceTag::PrePaid &&
     optionalService.type() != type::OptionalServiceTag::BaggageCharge)
  {
    return false;
  }

  bool isPMatchedAsF = false;
  if(_fallbackService.isSet(tse::fallback::ATPCO_TAX_AcceptOCTagPrePaid))
  {
    isPMatchedAsF = (entry.optionalServiceTag == type::OptionalServiceTag::PrePaid &&
        optionalService.type() == type::OptionalServiceTag::FlightRelated);
  }

  const bool matchOCTagBlank = (entry.optionalServiceTag == type::OptionalServiceTag::Blank);

  return (optionalService.type() == entry.optionalServiceTag || matchOCTagBlank || isPMatchedAsF) &&
      (entry.taxCode == "OC") &&
      matches(entry.taxTypeSubcode, optionalService.subCode()) &&
      (entry.group.empty() || entry.group == optionalService.serviceGroup()) &&
      (entry.subGroup.empty() || entry.subGroup == optionalService.serviceSubGroup()) &&
      (entry.feeOwnerCarrier.empty() || entry.feeOwnerCarrier == optionalService.ownerCarrier());
}

bool
TaxOnOptionalServiceApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (!_serviceBaggage && _restrictServiceBaggage)
  {
    failAllOptionalServices(paymentDetail);
    return !isFailedOc(paymentDetail);
  }

  if (!_serviceBaggage || _serviceBaggage->entries.size() == 0)
  {
    return true;
  }

  for (OptionalService & optionalService: paymentDetail.optionalServiceItems())
  {
    if (optionalService.isFailed())
      continue;

    uint16_t matchLevel = 0;
    uint16_t matchingLevel = 0;
    uint16_t notMatchingLevel = 0;

    for (const ServiceBaggageEntry & entry: _serviceBaggage->entries)
    {
      matchLevel = 0;
      bool match = (entry.applTag == type::ServiceBaggageAppl::Positive);

      if (checkOptionalService(entry, optionalService))
      {
        if (!entry.taxCode.empty())
        {
          ++matchLevel;
          if (!entry.taxTypeSubcode.empty())
          {
            ++matchLevel;
          }
        }
        if (!entry.group.empty())
        {
          ++matchLevel;
          if (!entry.subGroup.empty())
          {
            ++matchLevel;
          }
        }
        if (!entry.feeOwnerCarrier.empty())
        {
          ++matchLevel;
        }

        if (match)
        {
          matchingLevel = std::max(matchLevel, matchingLevel);
        }
        else
        {
          notMatchingLevel = std::max(matchLevel, notMatchingLevel);
        }
      }
    }

    if (matchingLevel <= notMatchingLevel && _restrictServiceBaggage)
    {
      optionalService.setFailedRule(&_rule);
    }
  }

  return !isFailedOc(paymentDetail);
}

void
TaxOnOptionalServiceApplicator::failAllOptionalServices(PaymentDetail& paymentDetail) const
{
  for (OptionalService & optionalService: paymentDetail.optionalServiceItems())
  {
    if (optionalService.isFailed())
      continue;
    optionalService.setFailedRule(&_rule);
  }
}

} // namespace tax
