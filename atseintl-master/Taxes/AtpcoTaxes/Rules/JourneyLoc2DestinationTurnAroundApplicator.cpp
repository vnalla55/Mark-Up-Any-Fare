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
#include "Rules/JourneyLoc2DestinationTurnAroundApplicator.h"

#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/MileageService.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TurnaroundCalculator.h"

namespace tax
{
JourneyLoc2DestinationTurnAroundApplicator::JourneyLoc2DestinationTurnAroundApplicator(
    const JourneyLoc2DestinationTurnAroundRule& rule,
    const Itin& itin,
    const LocService& locService,
    const MileageService& mileageService)
  : BusinessRuleApplicator(&rule),
    _rule(rule),
    _geoPath(*itin.geoPath()),
    _itin(itin),
    _locService(locService),
    _mileageService(mileageService)
{
}

JourneyLoc2DestinationTurnAroundApplicator::~JourneyLoc2DestinationTurnAroundApplicator()
{
}

bool
JourneyLoc2DestinationTurnAroundApplicator::apply(PaymentDetail& paymentDetail) const
{
  if ((_rule.getJrnyInd() ==
       type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ) ||
      !paymentDetail.roundTripOrOpenJaw())
  {
    const Geo& geo = _geoPath.geos()[_geoPath.geos().size() - 1];
    paymentDetail.setJourneyLoc2(&geo);
    return _locService.isInLoc(geo.loc().code(), _rule.getLocZone(), _rule.getVendor());
  }
  else
  {
    TurnaroundCalculator turnaroundCalculator(_geoPath,
                                              _mileageService,
                                              paymentDetail.taxPointsProperties(),
                                              paymentDetail.specUS_RTOJLogic(),
                                              _rule.alternateTurnaroundDeterminationLogic());
    const Geo* turnAroundGeo = _itin.getTurnaround(turnaroundCalculator);
    if(turnAroundGeo)
    {
      paymentDetail.setJourneyLoc2(turnAroundGeo);
      return _locService.isInLoc(turnAroundGeo->loc().code(), _rule.getLocZone(), _rule.getVendor());
    }
  }

  return false;
}
}
