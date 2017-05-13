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

#include "Rules/PointOfSaleApplicator.h"
#include "Rules/PointOfSaleRule.h"
#include "ServiceInterfaces/LocService.h"
#include "Util/BranchPrediction.h"

namespace tax
{

PointOfSaleApplicator::PointOfSaleApplicator(const PointOfSaleRule& rule,
                                             const type::AirportCode& salePoint,
                                             const LocService& locService)
  : BusinessRuleApplicator(&rule),
    _pointOfSaleRule(rule),
    _salePoint(salePoint),
    _locService(locService)
{
}

PointOfSaleApplicator::~PointOfSaleApplicator() {}

bool
PointOfSaleApplicator::apply(PaymentDetail& /*paymentDetail*/) const
{
  if (UNLIKELY(_salePoint.empty()))
    return true;
  return _locService.isInLoc(
      _salePoint, _pointOfSaleRule.getLocZone(), _pointOfSaleRule.getVendor());
}
}
