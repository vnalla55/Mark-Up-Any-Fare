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
#include "Rules/ReturnToOriginApplicator.h"

#include "DomainDataObjects/GeoPath.h"

namespace tax
{

ReturnToOriginApplicator::ReturnToOriginApplicator(BusinessRule const* parent,
                                                   GeoPath const& geoPath,
                                                   type::RtnToOrig const& rtnToOrig)
  : BusinessRuleApplicator(parent), _geoPath(geoPath), _rtnToOrig(rtnToOrig)
{
}

ReturnToOriginApplicator::~ReturnToOriginApplicator() {}

bool
ReturnToOriginApplicator::apply(PaymentDetail& /* paymentDetail */) const
{
  if (_rtnToOrig == type::RtnToOrig::ReturnToOrigin)
  {
    if (_geoPath.isJourneyDomestic())
      return _geoPath.getOriginCity() == _geoPath.getDestinationCity();
    else
      return _geoPath.getOriginNation() == _geoPath.getDestinationNation();
  }
  else //(_rtnToOrig == type::RtnToOrig::NotReturnToOrigin)
  {
    if (_geoPath.isJourneyDomestic())
      return _geoPath.getOriginCity() != _geoPath.getDestinationCity();
    else
      return _geoPath.getOriginNation() != _geoPath.getDestinationNation();
  }
}
}
