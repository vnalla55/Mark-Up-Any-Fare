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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/TaxPointLoc1InternationalDomesticRule.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/PaymentDetail.h"

namespace tax
{

class TaxPointLoc1InternationalDomesticRule;
class LocService;
class PaymentDetail;

class TaxPointLoc1InternationalDomesticApplicator : public BusinessRuleApplicator
{
  typedef bool FlightTypeChecker(const type::Nation& nation1, const type::Nation& nation2);

public:
  TaxPointLoc1InternationalDomesticApplicator(const TaxPointLoc1InternationalDomesticRule& rule,
                                              const GeoPath& geoPath,
                                              const LocService& locService);

  ~TaxPointLoc1InternationalDomesticApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const GeoPath& _geoPath;
  const LocService& _locService;
  const TaxPointLoc1InternationalDomesticRule& _intlDomLoc1IndRule;

  bool applyAdjacent(PaymentDetail& paymentDetail, FlightTypeChecker& flightTypeChecker) const;
  bool applyAdjacentStopoverDomestic(PaymentDetail& paymentDetail) const;
  bool applyAdjacentStopoverInternational(PaymentDetail& paymentDetail) const;

  bool skipUnticketedPoint(const type::Index& id) const;
};

} // namespace tax
