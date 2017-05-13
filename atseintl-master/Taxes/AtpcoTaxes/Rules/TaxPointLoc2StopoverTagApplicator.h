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

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/TaxPointLoc2StopoverTagRule.h"

#include <vector>

namespace tax
{

class BusinessRule;
class Itin;
class Services;
class PaymentDetail;

class TaxPointLoc2StopoverTagApplicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc2StopoverTagApplicator(const TaxPointLoc2StopoverTagRule& rule,
                                    const Itin& itin,
                                    const Services& services);
  ~TaxPointLoc2StopoverTagApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool applyForItinerary(PaymentDetail& paymentDetail,
                         bool matchTicketedOnly) const;
  bool applyForYqYr(PaymentDetail& paymentDetail,
                    const TaxPointsProperties& properties,
                    bool matchTicketedOnly,
                    TaxableYqYrs& taxableYqYrs) const;
  type::Index findFurthestIndex(const PaymentDetail& paymentDetail, bool matchTicketedOnly) const;
  type::Index findFurthestPointInFareComponent(const PaymentDetail& paymentDetail,
                                               bool matchTicketedOnly) const;
  type::Index findFurthestPointInDirection(const PaymentDetail& paymentDetail) const;
  type::Index findLastDomPointBeforeInt(const PaymentDetail& paymentDetail,
                                        bool matchTicketedOnly) const;
  type::Index findFurthestIntPointBeforeDomStop(const PaymentDetail& paymentDetail,
                                                bool matchTicketedOnly) const;

  const TaxPointLoc2StopoverTagRule& _rule;

  const GeoPath& _geoPath;
  const Itin& _itin;
  const Services& _services;
  int _direction;
  type::TaxPointTag _taxPointTagToMatch;
  bool failApplication;
};

} // namespace tax
