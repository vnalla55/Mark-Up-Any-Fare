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

namespace tax
{

class GeoPath;
class LocService;
class PaymentDetail;
class TaxPointLoc3AsNextStopoverRule;

class TaxPointLoc3AsNextStopoverApplicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc3AsNextStopoverApplicator(const TaxPointLoc3AsNextStopoverRule& rule,
                                       const GeoPath& geoPath,
                                       const LocService& locService);

  ~TaxPointLoc3AsNextStopoverApplicator();

  bool apply(PaymentDetail& paymentDetail) const;
  bool matchLocs(const Geo& geo3, PaymentDetail& paymentDetail) const;
  void matchLocsForOC(const Geo& geo3, PaymentDetail& paymentDetail) const;

private:
  bool applyOnItinerary(PaymentDetail& paymentDetail) const;
  bool applyOnYqYr(const type::Index beginId,
                   const TaxPointsProperties& properties,
                   const bool matchTicketedOnly,
                   TaxableYqYrs& taxableYqYrs) const;
  const GeoPath& _geoPath;
  const LocService& _locService;
  const TaxPointLoc3AsNextStopoverRule& _rule;
};

} // namespace tax

