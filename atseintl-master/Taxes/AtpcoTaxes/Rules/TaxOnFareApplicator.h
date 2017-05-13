// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/ProrateCalculator.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/SectorDetailMatcher.h"

#include <boost/optional/optional_fwd.hpp>
#include <vector>

namespace tax
{
class FarePath;
class GeoPathMapping;
class PaymentDetail;
class Services;
class SectorDetail;
class TaxOnFareRule;
class MileageGetter;

class TaxOnFareApplicator : public BusinessRuleApplicator
{
public:
  TaxOnFareApplicator(TaxOnFareRule const& parent,
                      Services const& services,
                      std::shared_ptr<SectorDetail const> sectorDetail,
                      std::vector<Fare> const& fares,
                      FarePath const& farePath,
                      std::vector<Flight> const& flights,
                      std::vector<FlightUsage> const& flightUsages,
                      GeoPath const& geoPath,
                      GeoPathMapping const& geoPathMapping,
                      type::Index const& itinId,
                      bool useRepricing,
                      bool isRtw,
                      MileageGetter const& mileageGetter);

  ~TaxOnFareApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool applyForRtw(PaymentDetail& paymentDetail) const;
  bool applyForNonRtw(PaymentDetail& paymentDetail) const;

  boost::optional<type::MoneyAmount>
  calculateFareAmount(PaymentDetail const& paymentDetail, bool withMarkup = false) const;

  boost::optional<type::MoneyAmount>
  getFareAmount(PaymentDetail const& paymentDetail, bool withMarkup = false) const;

  type::MoneyAmount getNetRemitFareAmount() const;
  boost::optional<type::MoneyAmount>
  getFareAmountForLowestFromFareList(PaymentDetail const& paymentDetail) const;
  boost::optional<type::MoneyAmount>
  getFareAmountForLowestFromFareListBahamasLogic(PaymentDetail const& paymentDetail) const;
  boost::optional<type::MoneyAmount>
  getFareAmountForLowestFromFareListStandardLogic(PaymentDetail const& paymentDetail) const;
  boost::optional<type::MoneyAmount>
  getFareAmountForFareBreaks(PaymentDetail const& paymentDetail) const;
  type::MoneyAmount getFareAmountForSectorDetail() const;

  boost::optional<type::MoneyAmount>
  getFareAmountWithUSDeductMethod(PaymentDetail const& paymentDetail, bool withMarkup = false) const;

  type::MoneyAmount
  getFareAmountForFullyApplicableRoundTrip(PaymentDetail const& paymentDetail, bool withMarkup = false) const;

  type::MoneyAmount getFareAmountByProration(const PaymentDetail& paymentDetail) const;
  type::MoneyAmount collectFareBetweenFareBreaks(PaymentDetail const& paymentDetail) const;

  TaxOnFareRule const& _taxOnFareRule;
  Services const& _services;
  std::shared_ptr<SectorDetail const> _sectorDetail;
  std::vector<Fare> const& _fares;
  FarePath const& _farePath;
  std::vector<Flight> const& _flights;
  std::vector<FlightUsage> const& _flightUsages;
  GeoPath const& _geoPath;
  GeoPathMapping const& _geoPathMapping;
  type::Index const& _itinId;
  const bool _useRepricing;
  const bool _isRtw;
  SectorDetailMatcher _matcher;
  ProrateCalculator _prorateCalculator;
};

} // namespace tax

