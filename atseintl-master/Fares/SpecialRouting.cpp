//----------------------------------------------------------------------------
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Fares/SpecialRouting.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/FareInfo.h"
#include "Routing/MileageValidator.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingInfo.h"
#include "Routing/RtgKey.h"
#include "Routing/TravelRoute.h"

namespace tse
{

static Logger
logger("atseintl.Fares.SpecialRouting");

//----------------------------------------------------------------------------
// Constructor()
//----------------------------------------------------------------------------

SpecialRouting::SpecialRouting() {}

//----------------------------------------------------------------------------
// Destructor()
//----------------------------------------------------------------------------

SpecialRouting::~SpecialRouting() {}

//----------------------------------------------------------------------------
// validate()
//----------------------------------------------------------------------------

bool
SpecialRouting::validate(PricingTrx& trx,
                         std::vector<PaxTypeFare*>& fbrFares,
                         RoutingInfos& rInfos,
                         const TravelRoute& tvlRoute) const
{
  LOG4CXX_INFO(logger, "Entered SpecialRouting::validate()");
  std::vector<PaxTypeFare*>::iterator ptFare(fbrFares.begin());

  for (; ptFare != fbrFares.end(); ptFare++)
  {
    if ((*ptFare)->isRoutingProcessed() == false && processEmptyRouting(trx, **ptFare) == false)
    {
      bool updateComplete = false;
      RoutingInfos::iterator itr(rInfos.begin()), validPublishedRoutingFare(rInfos.end()),
          inValidPublishedRoutingFare(rInfos.end()), validMPMPublishedFare(rInfos.end()),
          inValidMPMPublishedFare(rInfos.end());

      for (; itr != rInfos.end(); itr++)
      {
        if (matchKey(trx, **ptFare, (*itr).first))
        {
          const RoutingInfo& rInfo = *(*itr).second;

          bool isRoutingFare = (rInfo.mapInfo() != nullptr);
          bool isMileageFare = (!isRoutingFare && rInfo.mileageInfo() != nullptr);

          if (isMileageFare)
          {
            if (validMPMPublishedFare == rInfos.end())
            {
              validMPMPublishedFare = ((rInfo.mileageInfo()->valid() == true) ? itr : rInfos.end());
            }

            if (inValidMPMPublishedFare == rInfos.end())
            {
              inValidMPMPublishedFare =
                  (rInfo.mileageInfo()->valid() == false) ? itr : rInfos.end();
            }
          }
          else
          {
            if (validPublishedRoutingFare == rInfos.end())
            {
              validPublishedRoutingFare = (rInfo.routingStatus() == true) ? itr : rInfos.end();
            }

            if (inValidPublishedRoutingFare == rInfos.end())
            {
              inValidPublishedRoutingFare = (rInfo.routingStatus() == false) ? itr : rInfos.end();
            }
          }

          if (validPublishedRoutingFare != rInfos.end())
          {
            updateComplete = true;
            updateFBRPaxTypewithRouting(trx, **ptFare, validPublishedRoutingFare);
            break;
          }
        }
        else
        {
          continue;
        }
      } // For rInfos

      if (!updateComplete)
      {
        if ((*ptFare)->routingNumber() == CAT25_INTERNATIONAL &&
            validMPMPublishedFare != rInfos.end())
        {
          updateFBRPaxTypewithMileage(trx, **ptFare, validMPMPublishedFare);
        }
        else if ((*ptFare)->routingNumber() == CAT25_DOMESTIC &&
                 validMPMPublishedFare != rInfos.end())
        {
          updateFBRPaxTypewithMileage(trx, **ptFare, validMPMPublishedFare);
        }
        else if ((*ptFare)->routingNumber() == CAT25_DOMESTIC &&
                 inValidPublishedRoutingFare != rInfos.end())
        {
          updateFBRPaxTypewithRouting(trx, **ptFare, inValidPublishedRoutingFare);
        }
        else
        {
          validateMileage(trx, **ptFare, tvlRoute);
        }
      }
    }
  } // For for PaxTypeFares

  return true;
}
bool
SpecialRouting::matchKey(PricingTrx& trx, PaxTypeFare& ptFare, const RtgKey& rKey) const
{
  // For SMF fare or Cat 25 fare, do not match the vendor code
  return ((ptFare.carrier() == rKey.carrier()) &&
          (ptFare.isFareByRule() || ptFare.vendor() == rKey.vendor() || isSMF(trx, ptFare)));
}

void
SpecialRouting::updateFBRPaxTypewithRouting(PricingTrx& trx,
                                            PaxTypeFare& pFare,
                                            RoutingInfos::iterator& rInfoItr) const
{
  Fare* fare = pFare.fare()->clone(trx.dataHandle(), false);
  FareInfo* fInfo = pFare.fare()->fareInfo()->clone(trx.dataHandle());

  FareInfo* publishedFI = fInfo; // dynamic_cast<FareInfo*>( fInfo );
  if (!pFare.isConstructed())
  {
    // ATPCO or SITA published fare
    if (trx.getTrxType() != PricingTrx::IS_TRX)
    {
      publishedFI->routingNumber() = (*rInfoItr).first.routingNumber();
    }
    fare->setFareInfo(fInfo);
    pFare.setFare(fare);
  }
  pFare.setIsRouting(true);
  pFare.setRoutingProcessed(true);
  pFare.setRoutingValid((*rInfoItr).second->routingStatus());
}

void
SpecialRouting::updateFBRPaxTypewithMileage(PricingTrx& trx,
                                            PaxTypeFare& pFare,
                                            RoutingInfos::iterator& rInfoItr) const
{

  Fare* fare = pFare.fare()->clone(trx.dataHandle(), false);
  FareInfo* fInfo = pFare.fare()->fareInfo()->clone(trx.dataHandle());

  FareInfo* publishedFI = fInfo; // dynamic_cast<FareInfo*>( fInfo );
  if (!pFare.isConstructed())
  {
    // ATPCO or SITA published fare
    if (trx.getTrxType() != PricingTrx::IS_TRX)
    {
      publishedFI->routingNumber() = MILEAGE_ROUTING;
    }
    fare->setFareInfo(fInfo);
    pFare.setFare(fare);
  }
  pFare.setIsRouting(false);
  pFare.setRoutingProcessed(true);
  pFare.setRoutingValid((*rInfoItr).second->routingStatus());

  MileageInfo* mInfo = (*rInfoItr).second->mileageInfo();

  if (mInfo != nullptr && pFare.isRoutingValid())
  {
    pFare.mileageSurchargePctg() = mInfo->surchargeAmt();
    pFare.mileageSurchargeAmt() = (pFare.nucFareAmount() * mInfo->surchargeAmt()) / 100.00;
  }
}

void
SpecialRouting::validateMileage(PricingTrx& trx, PaxTypeFare& pFare, const TravelRoute& tvlRoute)
    const
{
  if(trx.getOptions() && trx.getOptions()->isRtw())
    return;

  Fare* fare = pFare.fare()->clone(trx.dataHandle(), false);
  FareInfo* fInfo = pFare.fare()->fareInfo()->clone(trx.dataHandle());

  FareInfo* publishedFI = fInfo; // dynamic_cast<FareInfo*>( fInfo );
  if (!pFare.isConstructed())
  {
    // ATPCO or SITA published fare
    publishedFI->routingNumber() = MILEAGE_ROUTING;
    fare->setFareInfo(fInfo);
    pFare.setFare(fare);
  }
  MileageInfo mInfo;

  MileageValidator mileageValidator;
  mileageValidator.validate(trx, mInfo, tvlRoute);

  pFare.setIsRouting(false);
  pFare.setRoutingProcessed(true);
  pFare.setRoutingValid(mInfo.valid());

  if (mInfo.valid())
  {
    pFare.mileageSurchargePctg() = mInfo.surchargeAmt();
    pFare.mileageSurchargeAmt() = (pFare.nucFareAmount() * mInfo.surchargeAmt()) / 100.00;
  }
  else
  {
    pFare.mileageSurchargePctg() = 0;
    pFare.mileageSurchargeAmt() = 0.0;
  }
}

bool
SpecialRouting::processEmptyRouting(PricingTrx& trx, PaxTypeFare& pfare) const
{
  if ((pfare.routingNumber() == CAT25_EMPTY_ROUTING) &&
      (pfare.isFareByRule() == true)) // TODO is it duplicate checking, ask Nakamon
  {
    // first check whether the paxTypeFare is constructed or not
    // if yes take the routing number of the specified fare.

    Fare* fareClone = pfare.fare()->clone(trx.dataHandle(), false);
    FareInfo* fInfoClone = pfare.fare()->fareInfo()->clone(trx.dataHandle());

    FareInfo* publishedFI = fInfoClone; // dynamic_cast<FareInfo*>(fInfoClone);
    const PaxTypeFare& baseFare(*pfare.baseFare());
    if (LIKELY(!pfare.isConstructed()))
    {
      publishedFI->routingNumber() = baseFare.routingNumber();

      fareClone->setFareInfo(publishedFI);
      pfare.setFare(fareClone);
    }

    pfare.setRoutingProcessed(true);
    pfare.setRoutingValid(baseFare.isRoutingValid());
    if (!baseFare.isRouting())
    {
      pfare.mileageSurchargePctg() = baseFare.mileageSurchargePctg();
      pfare.mileageSurchargeAmt() = (pfare.nucFareAmount() * pfare.mileageSurchargePctg()) / 100.00;
      pfare.setIsRouting(false);
    }
    else
    {
      pfare.setIsRouting(true);
    }

    return true;
  }
  return false;
}

bool
SpecialRouting::isSMF(PricingTrx& trx, const PaxTypeFare& fare) const
{
  return (trx.dataHandle().getVendorType(fare.vendor()) == SMF_VENDOR);
}
}
