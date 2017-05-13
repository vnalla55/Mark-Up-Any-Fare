//----------------------------------------------------------------------------
//
// Copyright Sabre 2014
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseEnums.h"

#include <boost/noncopyable.hpp>

namespace tse
{

class Diag852Collector;
class TravelSeg;

class BTAValidationProcessObserver : public boost::noncopyable
{
public:
  BTAValidationProcessObserver(Diag852Collector* diag) : _diag(diag) {}

  bool notifyCabinValidationFinished(bool cabinPassed) const;

  bool notifyRBDValidationFinished(bool rbdPassed) const;

  void notifySegmentValidationStarted(const TravelSeg& segment) const;

  StatusS7Validation notifySegmentValidationFinished(StatusS7Validation validationStatus,
                                                     const TravelSeg& segment) const;

  bool notifyTable171ValidationFinished(bool resultingFareClassPasses) const;

  bool notifyOutputTicketDesignatorValidationFinished(bool passed) const;

  bool notifyRuleValidationFinished(bool rulePassed) const;

  bool notifyRuleTariffValidationFinished(bool ruleTariffPassed) const;

  bool notifyFareIndValidationFinished(bool rulePassed) const;

  bool notifyCarrierFlightApplT186Finished(bool t186Passed) const;

private:
  mutable Diag852Collector* _diag;
};
}

