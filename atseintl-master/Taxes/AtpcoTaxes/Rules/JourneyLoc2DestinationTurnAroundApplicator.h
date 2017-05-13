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
#include "Rules/JourneyLoc2DestinationTurnAroundRule.h"

namespace tax
{

class GeoPath;
class Itin;
class LocService;
class MileageService;
class PaymentDetail;

class JourneyLoc2DestinationTurnAroundApplicator : public BusinessRuleApplicator
{
public:
  JourneyLoc2DestinationTurnAroundApplicator(const JourneyLoc2DestinationTurnAroundRule& rule,
                                             const Itin& itin,
                                             const LocService& locService,
                                             const MileageService& mileageService);
  ~JourneyLoc2DestinationTurnAroundApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const JourneyLoc2DestinationTurnAroundRule& _rule;
  const GeoPath& _geoPath;
  const Itin& _itin;
  const LocService& _locService;
  const MileageService& _mileageService;
};
}
