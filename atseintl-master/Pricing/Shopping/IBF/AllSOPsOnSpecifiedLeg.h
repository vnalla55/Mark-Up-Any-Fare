/*
 * AllSOPsFromFirstLegAfterFixedLeg.h
 *
 *  Created on: Aug 17, 2015
 *      Author: SG0217429
 */

#pragma once

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/IBF/AllSopsRepresented.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

namespace tse {

class AllSOPsOnSpecifiedLeg
    : public AllSopsRepresented
{
public:
  AllSOPsOnSpecifiedLeg(ISopUsageTracker& tracker)
      : AllSopsRepresented(tracker), _legIdToTrack(-1)
  {
  }

  bool isSatisfied() const override;

  std::string toString() const override;

  void setLegIdToTrack(uint16_t legIdToTrack)
  {
    _legIdToTrack = legIdToTrack;
  }

private:
  uint16_t _legIdToTrack;

};

} /* namespace tse */
