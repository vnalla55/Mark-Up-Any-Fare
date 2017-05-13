//-------------------------------------------------------------------
//
//  File:        FDIndustryFareController.cpp
//  Created:     Jan. 13th, 2006
//  Authors:     Daryl Champagne
//
//  Description: Fare Display Industry Fare Controller - Specialization of
//      IndustryFareController class allowing for industry fare retrieval
//      optimized for Fare Display entries.
//
//  Updates:
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Fares/FDIndustryFareController.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
static Logger
logger("atseintl.Fares.FDIndustryFareController");

FDIndustryFareController::FDIndustryFareController(FareDisplayTrx& trx,
                                                   Itin& itin,
                                                   FareMarket& fareMarket)
  : IndustryFareController(trx, itin, fareMarket), _fdTrx(trx)
{
}

bool
FDIndustryFareController::process(const std::vector<Fare*>* addOnFaresVec)
{
  // By default, Pure YY fares are processed for single carrier requests.
  // For shopper requests, don't get industry fares if this is not the YY
  // carrier.
  if (_fdTrx.isInternational() && _fdTrx.isShopperRequest() &&
      _fareMarket.governingCarrier() != INDUSTRY_CARRIER)
  {
    _processPureYY = false;

    if (_processPureYY)
    {
      LOG4CXX_INFO(logger,
                   "Skipping FD Industry Fare tuning for "
                       << _fareMarket.governingCarrier()
                       << " carrier, market has constructed industry fares.");
    }
    else
    {
      LOG4CXX_INFO(logger,
                   "Applying FD Industry Fare tuning for " << _fareMarket.governingCarrier()
                                                           << " carrier.");
    }
  }

  return IndustryFareController::process(addOnFaresVec);
}
}
