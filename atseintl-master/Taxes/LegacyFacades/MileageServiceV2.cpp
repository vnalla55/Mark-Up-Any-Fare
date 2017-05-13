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
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"

#include "Taxes/AtpcoTaxes/DomainDataObjects/FlightUsage.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/GeoPath.h"
#include "Taxes/LegacyFacades/MileageServiceV2.h"
#include "Taxes/LegacyFacades/MileageGetterV2.h"

namespace tse
{

MileageServiceV2::MileageServiceV2(PricingTrx& trx) :
    _trx(trx),
    mileageGetterMap(),
    _isRtw(_trx.getOptions() ? _trx.getOptions()->isRtw() : false)
{
}

const tax::MileageGetter&
MileageServiceV2::getMileageGetter(const tax::GeoPath& geoPath,
                                   const std::vector<tax::FlightUsage>& flightUsages,
                                   const tax::type::Timestamp& travelDate) const
{
  GetterMapKey key(geoPath.id(), travelDate);

  GetterMap::const_iterator result = mileageGetterMap.find(key);
  const tax::MileageGetter* mileageGetter = nullptr;
  if (result == mileageGetterMap.end())
  {
    mileageGetter = new MileageGetterV2(geoPath, flightUsages, travelDate, _trx, isRtw());
    mileageGetterMap.insert(key, const_cast<tax::MileageGetter*>(mileageGetter));
  }
  else
  {
    mileageGetter = result->second;
  }

  return *mileageGetter;
}

bool
MileageServiceV2::isRtw() const
{
  return _isRtw;
}

} // namespace tax
