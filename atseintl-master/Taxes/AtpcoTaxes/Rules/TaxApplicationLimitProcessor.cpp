// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "DomainDataObjects/YqYrPath.h"
#include "Processor/BusinessRulesProcessor.h"
#include "Rules/LimitGroup.h"
#include "Rules/RawPayments.h"
#include "Rules/SpecialTrips.h"
#include "Rules/TaxApplicationLimitProcessor.h"
#include "Rules/TaxLimiter.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <iterator>

namespace tax
{

template<>
std::vector<Trip>
TaxApplicationLimitProcessor::find<ContinuousJourneyInfo>(PaymentDetail& paymentDetail)
{
  std::vector<Trip> journeys;
  SpecialTrips::findContinuousJourneys(paymentDetail, _geoPath, journeys);
  return journeys;
}

template<>
std::vector<Trip>
TaxApplicationLimitProcessor::find<SingleJourneyInfo>(PaymentDetail& paymentDetail)
{
  std::vector<Trip> journeys;
  SpecialTrips::findSingleJourneys(paymentDetail, _geoPath, journeys);
  return journeys;
}

template<>
std::vector<Trip>
TaxApplicationLimitProcessor::find<UsRoundTripInfo>(PaymentDetail& paymentDetail)
{
  std::vector<Trip> journeys;
  SpecialTrips::findRoundTrips(paymentDetail, _geoPath, _flightUsages, journeys);
  return journeys;
}

std::set<type::TaxApplicationLimit>
TaxApplicationLimitProcessor::createLimits(const std::vector<PaymentWithRules>& paymentWithRulesVector)
{
  std::set<type::TaxApplicationLimit> result;

  for (PaymentWithRules each: paymentWithRulesVector)
  {
    result.insert(each.paymentDetail->getLimitType());
  }

  return result;
}

std::vector<std::shared_ptr<LimitedJourneyInfo>>
TaxApplicationLimitProcessor::determineLimitedJourneys(
    std::vector<PaymentWithRules>& paymentWithRulesVector, std::vector<TaxLimitInfo>& taxLimitInfos)
{
  std::vector<std::shared_ptr<LimitedJourneyInfo>> result;
  std::set<type::TaxApplicationLimit> limits = createLimits(paymentWithRulesVector);
  TaxPointMap taxPointMap;
  std::shared_ptr<LimitedJourneyInfo> itineraryInfo;

  for (type::TaxApplicationLimit each: limits)
  {
    switch (each)
    {
    case type::TaxApplicationLimit::OnceForItin :
    case type::TaxApplicationLimit::Unlimited :
      if (!itineraryInfo)
      {
        itineraryInfo = LimitedJourneyInfo::create<ItineraryInfo>(taxLimitInfos.begin(), taxLimitInfos.end(),
            taxLimitInfos.begin(), taxLimitInfos.end());
        result.push_back(itineraryInfo);
      }
      break;
    case type::TaxApplicationLimit::FirstTwoPerContinuousJourney :
      addJourneys<ContinuousJourneyInfo>(paymentWithRulesVector.front().getPaymentDetail(),
          result, taxPointMap, taxLimitInfos);
      break;
    case type::TaxApplicationLimit::OncePerSingleJourney :
      addJourneys<SingleJourneyInfo>(paymentWithRulesVector.front().getPaymentDetail(),
          result, taxPointMap, taxLimitInfos);
      break;
    case type::TaxApplicationLimit::FirstTwoPerUSRoundTrip :
      addJourneys<UsRoundTripInfo>(paymentWithRulesVector.front().getPaymentDetail(),
          result, taxPointMap, taxLimitInfos);
      break;
    default:
      break;
    }
  }

  if (itineraryInfo)
    itineraryInfo->setTaxPointMap(taxPointMap);

  return result;
}

std::set<type::Index>
TaxApplicationLimitProcessor::selectGeoOfPassesPayments(
    const std::vector<std::shared_ptr<LimitedJourneyInfo>>& limitedJourneys)
{
  std::set<type::Index> result;

  for (const std::shared_ptr<tax::LimitedJourneyInfo>& each : limitedJourneys)
  {
    std::vector<type::Index> geoIds = each->findPassedRules();
    result.insert(geoIds.begin(), geoIds.end());
  }

  return result;
}

std::vector<TaxLimitInfo>
TaxApplicationLimitProcessor::createTaxLimitInfos(
    const std::vector<PaymentWithRules>& paymentWithRulesVector)
{
  std::vector<TaxLimitInfo> result;

  for (const PaymentWithRules& each: paymentWithRulesVector)
  {
    result.push_back(TaxLimitInfo::create(*each.paymentDetail));
  }

  return result;
}

void
TaxApplicationLimitProcessor::run(std::vector<PaymentWithRules>& paymentWithRulesVector)
{
  std::vector<TaxLimitInfo> taxLimitInfos = createTaxLimitInfos(paymentWithRulesVector);
  std::vector<std::shared_ptr<LimitedJourneyInfo>> limitedJourneys =
      determineLimitedJourneys(paymentWithRulesVector, taxLimitInfos);

  std::set<type::Index> geoOfPassesPayments = selectGeoOfPassesPayments(limitedJourneys);

  for (const PaymentWithRules& each: paymentWithRulesVector)
  {
    if (std::find(geoOfPassesPayments.begin(), geoOfPassesPayments.end(),
        each.getGeoId()) == geoOfPassesPayments.end())
    {
      each.paymentDetail->getMutableItineraryDetail().setFailedRule(each.paymentDetail->getLimitRule());
    }
  }
}

}
