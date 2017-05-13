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
#pragma once

#include "Common/TseEnums.h"
#include "Taxes/AtpcoTaxes/Common/Timestamp.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/MileageGetter.h"

namespace tax
{
class FlightUsage;
class GeoPath;
} // namespace tax

namespace tse
{
class DataHandle;
class DateTime;
class PricingTrx;

class MileageGetterV2 : public tax::MileageGetter
{
  friend class MileageGetterV2Test;

public:
  MileageGetterV2(const tax::GeoPath& geoPath,
                  const std::vector<tax::FlightUsage>& flightUsages,
                  const tax::type::Timestamp& travelDate,
                  PricingTrx& trx,
                  bool isRtw);

  tax::type::Miles
  getSingleDistance(const tax::type::Index& from, const tax::type::Index& to) const override;

  tax::type::GlobalDirection
  getSingleGlobalDir(const tax::type::Index& from, const tax::type::Index& to) const override;

  tse::DateTime
  getOriginDate(unsigned int segmentIndex) const;

  tse::DateTime
  getDestinationDate(unsigned int segmentIndex) const;

private:
  tse::GlobalDirection
  getTseGlobalDir(const tax::type::Index& from, const tax::type::Index& to) const;

  const tse::Loc& getLoc(const tax::type::AirportCode& city, const DateTime& tseTravelDate) const;

  const tax::GeoPath& _geoPath;
  const std::vector<tax::FlightUsage>& _flightUsages;
  const tax::type::Timestamp _travelDate;
  PricingTrx& _trx;
  DataHandle& _dataHandle;
  bool _isRtw;
};

} // namespace tse

