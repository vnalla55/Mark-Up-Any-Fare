// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "ItinAnalyzer/SoloJourney.h"
#include "ItinAnalyzer/SoloJourneySeg.h"

namespace tse
{
class Itin;
class SOPUsage;
class PricingTrx;

class SoloJourneyUtil
{
public:
  SoloJourneyUtil(const PricingTrx& trx, const Itin& itin);
  void fillAvailability(SOPUsage& sopUsage) const;

private:
  const PricingTrx& _trx;
  SoloJourneyVec _journeys;
  SoloJourneySegVec _segments;
};
} // namespace tse

