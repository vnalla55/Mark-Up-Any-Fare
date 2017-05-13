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

#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/MileageService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/RuleDescriptionFormater.h"
#include "Rules/YqYrAmountApplicator.h"
#include "Rules/YqYrAmountRule.h"

namespace tax
{
YqYrAmountRule::YqYrAmountRule()
{
}

YqYrAmountRule::ApplicatorType
YqYrAmountRule::createApplicator(const type::Index& itinIndex,
                                 const Request& request,
                                 Services& services,
                                 RawPayments& /*rawPayments*/) const
{

  Itin const& itin = request.getItinByIndex(itinIndex);
  const GeoPath* geoPath = itin.geoPath();
  const std::vector<FlightUsage>& flightUsages = itin.flightUsages();
  const type::Timestamp travelDate(itin.travelOriginDate(), type::Time(0, 0));
  const MileageGetter& mileageGetter =
      services.mileageService().getMileageGetter(*geoPath, flightUsages, travelDate);

  return ApplicatorType(*this, mileageGetter, *geoPath);
}

std::string
YqYrAmountRule::getDescription(Services& /*services*/) const
{
  return std::string("CALCULATE TOTAL TAX ON YQYRS");
}
}
