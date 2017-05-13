// ----------------------------------------------------------------------------

//  Copyright Sabre 2013

//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.

// ----------------------------------------------------------------------------
#pragma once

#include "Rules/BusinessRuleApplicator.h"
#include "Rules/TimeStopoverChecker.h"

#include <boost/variant/variant.hpp>

namespace tax
{
class FillTimeStopoversRule;
class Itin;
class FlightUsage;
class PaymentDetail;

class FillTimeStopoversApplicator : public BusinessRuleApplicator
{
public:
  typedef boost::variant<DomesticTimeStopoverChecker,
                         SpecialDomesticTimeStopoverChecker,
                         SpecificTimeStopoverChecker,
                         DateStopoverChecker,
                         MonthsStopoverChecker,
                         SameDayStopoverChecker> VariantStopoverChecker;

  FillTimeStopoversApplicator(const FillTimeStopoversRule* businessRule, const Itin& itin);
  ~FillTimeStopoversApplicator();

  bool isStopover(const FlightUsage& prev, const FlightUsage& next) const;
  bool apply(PaymentDetail& paymentDetail) const;

private:
  const FillTimeStopoversRule* _fillTimeStopoversRule;
  const Itin& _itin;
  VariantStopoverChecker _timeStopoverChecker;
};

} // namespace tax

