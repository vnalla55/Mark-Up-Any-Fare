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
#pragma once

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/FlightUsage.h"
#include "Rules/LimitedJourneyInfo.h"
#include "Rules/SpecialTrips.h"
#include "Rules/TaxLimitInfo.h"

#include <memory>
#include <set>
#include <vector>

namespace tax
{
struct PaymentWithRules;
class GeoPath;

class TaxApplicationLimitProcessor
{
  friend class TaxApplicationLimitProcessorTest;

  const GeoPath& _geoPath;
  const std::vector<FlightUsage>& _flightUsages;

  std::vector<std::shared_ptr<LimitedJourneyInfo>>
  determineLimitedJourneys(std::vector<PaymentWithRules>& paymentWithRulesVector,
                           std::vector<TaxLimitInfo>& taxLimitInfos);

  std::set<type::Index> selectGeoOfPassesPayments(
      const std::vector<std::shared_ptr<LimitedJourneyInfo>>& limitedJourneys);

  std::set<type::TaxApplicationLimit>
  createLimits(const std::vector<PaymentWithRules>& paymentWithRulesVector);

  std::vector<TaxLimitInfo>
  createTaxLimitInfos(const std::vector<PaymentWithRules>& paymentWithRulesVector);

  template <typename T>
  void addJourneys(PaymentDetail& paymentDetail,
                   std::vector<std::shared_ptr<LimitedJourneyInfo>>& result,
                   TaxPointMap& taxPointMap,
                   std::vector<TaxLimitInfo>& taxLimitInfos)
  {
    std::vector<Trip> journeys = find<T>(paymentDetail);
    for (const Trip& trip : journeys)
    {
      result.push_back(LimitedJourneyInfo::create<T>(taxLimitInfos.begin(), taxLimitInfos.end(), trip));
    }
    taxPointMap.addJourney(journeys);
  }

  template<typename T>
  std::vector<Trip> find(PaymentDetail& paymentDetail);

public:
  explicit TaxApplicationLimitProcessor(const GeoPath& geoPath,
                                        const std::vector<FlightUsage>& flightUsages)
  : _geoPath(geoPath), _flightUsages(flightUsages) {}

  void run(std::vector<PaymentWithRules>& paymentWithRulesVector);
};
}
