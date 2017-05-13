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

#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class BusinessRule;
class PaymentDetail;
class Geo;
class LocService;
class JourneyLoc1AsOriginRule;

class JourneyLoc1AsOriginApplicator : public BusinessRuleApplicator
{
public:
  JourneyLoc1AsOriginApplicator(const JourneyLoc1AsOriginRule& rule,
                                const Geo& startGeo,
                                const LocService& locService);

  bool apply(PaymentDetail& paymentDetail) const;
  void setSkipExempt(bool skipExempt)  { _isSkipExempt = skipExempt; };

private:
  const JourneyLoc1AsOriginRule& _loc1AsOriginRule;
  const Geo& _startGeo;
  const LocService& _locService;
  bool _isSkipExempt;
};
}
