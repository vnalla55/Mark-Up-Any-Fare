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
#ifndef MILEAGE_CACHE_IMPL_H
#define MILEAGE_CACHE_IMPL_H

#include <vector>
#include <boost/ptr_container/ptr_map.hpp>

#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/MileageService.h"

namespace tax
{
class FlightUsage;
class GeoPath;
class MileageGetter;
}

namespace tse
{
class PricingTrx;

class MileageServiceV2 : public tax::MileageService
{
  friend class MileageServiceV2Test;

  typedef std::pair<tax::type::Index, tax::type::Timestamp> GetterMapKey;
  typedef boost::ptr_map<GetterMapKey, tax::MileageGetter> GetterMap;

public:
  MileageServiceV2(PricingTrx&);

  const tax::MileageGetter& getMileageGetter(const tax::GeoPath& geoPath,
                                             const std::vector<tax::FlightUsage>& flightUsages,
                                             const tax::type::Timestamp& travelDate) const override;

  bool isRtw() const override;

private:
  PricingTrx& _trx;
  mutable GetterMap mileageGetterMap;
  const bool _isRtw;
};

} // namespace tax
#endif
