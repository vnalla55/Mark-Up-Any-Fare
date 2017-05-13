//-------------------------------------------------------------------
//  File   : FareGroupingMgr.cpp
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/FareGroupingMgr.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/FQFareGroupingMgr.h"
#include "FareDisplay/GroupingStrategy.h"
#include "FareDisplay/RDFareGroupingMgr.h"
#include "FareDisplay/RoutingSequenceGenerator.h"
#include "FareDisplay/StrategyBuilder.h"
#include "Fares/FDFareCurrencySelection.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.FareGroupingMgr");

FareGroupingMgr*
FareGroupingMgr::create(FareDisplayTrx& trx)
{
  if (trx.isRD() || trx.getRequest()->requestType() == MP_REQUEST)
  {
    RDFareGroupingMgr* mgr(nullptr);
    trx.dataHandle().get(mgr);
    return mgr;
  }
  else if (trx.isFQ())
  {
    FQFareGroupingMgr* mgr(nullptr);
    trx.dataHandle().get(mgr);
    return mgr;
  }
  else
    return nullptr;
}

bool
FareGroupingMgr::isSequenceTranslationRequired(FareDisplayTrx& trx)
{
  if (!trx.isSDSOutputType() && trx.isShopperRequest())
  {
    return false;
  }
  else if (trx.isShortRD() && !trx.isERD())
    return false;
  else
    return (trx.itin().front()->geoTravelType() == GeoTravelType::International || trx.isSameCityPairRqst());
}

void
FareGroupingMgr::processGrouping(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering FareGroupingMgr::processGrouping() ");

  StrategyBuilder builder;
  GroupingStrategy* strategy = builder.buildStrategy(trx);

  if (strategy != nullptr)
  {
    strategy->apply();
  }

  if (isSequenceTranslationRequired(trx))
  {
    RoutingSequenceGenerator seqGenerator;
    if (!strategy->fares().empty())
      seqGenerator.generateRoutingSequence((strategy->fares()));
    if (!strategy->yy_fares().empty())
      seqGenerator.generateRoutingSequence((strategy->yy_fares()));
  }

  LOG4CXX_INFO(logger, "Leaving FareGroupingMgr::processGrouping() ");
}
}
