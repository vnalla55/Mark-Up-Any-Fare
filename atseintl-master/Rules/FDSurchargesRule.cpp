//-------------------------------------------------------------------
//
//  File:        FDSurchargesRule.cpp
//  Created:     May 17, 2005
//  Authors:     LeAnn Perez
//
//  Description:
//
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//------------------------------------------------------------------

#include "Rules/FDSurchargesRule.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/SurchargesInfo.h"
#include "Rules/SecSurchargeAppl.h"

#include <vector>

namespace tse
{
FDSurchargesRule::FDSurchargesRule() {}

FDSurchargesRule::~FDSurchargesRule() {}

static Logger
logger("atseintl.Rules.FDSurchargesRule");

struct HasMatchingSurchargeType : public std::unary_function<Indicator, bool>
{
public:
  HasMatchingSurchargeType(const Indicator& surchargeType) : _surchargeType(surchargeType) {}

  bool operator()(const Indicator& surchargeType) const
  {
    return (_surchargeType == surchargeType);
  }

private:
  const Indicator& _surchargeType;
};

//-------------------------------------------------------------
// NotFoundInCat12 unary predicate to found out if same sector
// surcharge is also in cat 12
// ------------------------------------------------------------
class FoundInCat12 : public std::unary_function<SurchargeData*, bool>
{
public:
  FoundInCat12(std::vector<SurchargeData*> cat12Surcharges, const AirSeg& airSeg)
    : _cat12Surcharges(cat12Surcharges), _airSeg(airSeg)
  {
  }
  ~FoundInCat12() {}
  bool operator()(const SurchargeData* sectorSurcharge) const
  {
    std::vector<SurchargeData*>::const_iterator surchDataI = _cat12Surcharges.begin();
    std::vector<SurchargeData*>::const_iterator surchDataEndI = _cat12Surcharges.end();

    for (; surchDataI != surchDataEndI; surchDataI++)
    {
      const SurchargeData& cat12Surcharge = **surchDataI;

      if ((cat12Surcharge.surchargeType() == sectorSurcharge->surchargeType() ||
           /* This is hard coded for the time being, as it is observed that the same sector
            * surcharge is filed under type O in sectorsurcharge table and under type Q in
            * cat 12. If not for this hard code, this surcharge will be added twice. If any
            * thing changes about the way they are filed, this needs change as well */
           (cat12Surcharge.surchargeType() == 'Q' && sectorSurcharge->surchargeType() == 'O')) &&
          cat12Surcharge.brdAirport() == _airSeg.origAirport() &&
          cat12Surcharge.offAirport() == _airSeg.destAirport() &&
          cat12Surcharge.carrier() == sectorSurcharge->carrier() &&
          cat12Surcharge.amountSelected() == sectorSurcharge->amountSelected())
      {
        return true;
      }
    }
    return false;
  }

private:
  std::vector<SurchargeData*> _cat12Surcharges;
  AirSeg _airSeg;
};

/**
*  process()     Fare Display
**/
Record3ReturnTypes
FDSurchargesRule::process(PricingTrx& trx,
                          PaxTypeFare& paxTypeFare,
                          const CategoryRuleItemInfo* rule,
                          const SurchargesInfo* surchInfo)
{
  LOG4CXX_INFO(logger, "Entered FDSurchargesRule::process()");

  //--------------------------------------------------------------
  // Get a Fare Display Transaction from the Pricing Transaction
  //--------------------------------------------------------------
  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;

  if (!fdUtil.getFareDisplayTrx(&trx, fdTrx))
  {
    LOG4CXX_ERROR(logger, "FDSurchargesRule::process() - Unable to get FareDisplayTrx");
    LOG4CXX_INFO(logger, "Leaving FDSurchargesRule::validate() - SKIP");

    return SKIP;
  }

  Itin* itin = trx.itin().front();

  // Get FareDisplayInfo object
  FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (fareDisplayInfo == nullptr)
  {
    LOG4CXX_ERROR(logger, "FDSurchargesRule::process() - Unable to get FareDisplayInfo object");
    LOG4CXX_INFO(logger, "Leaving FDSurchargesRule::validate() - SKIP");
    return SKIP;
  }

  // Build farePath if one does not exist
  FarePath* fp = nullptr;
  if (fareDisplayInfo->farePath() == nullptr)
  {
    trx.dataHandle().get(fp);
    if (fp == nullptr)
    {
      LOG4CXX_ERROR(logger,
                    "FDSurchargesRule::process() - UNABLE TO ALLOCATE MEMORY FOR FAREPATH");
      LOG4CXX_INFO(logger, "Leaving FDSurchargesRule::validate() - SKIP");
      return SKIP;
    }
    // fp->totalNUCAmount()= paxTypeFare.originalFareAmount();
    fp->setTotalNUCAmount(paxTypeFare.nucFareAmount());
    fp->paxType() = paxTypeFare.actualPaxType();
    fp->itin() = itin;
    // Default to 1st ITIN PAX TYPE
    if (fp->paxType() == nullptr)
    {
      fp->paxType() = trx.paxType().front();
    }
    fareDisplayInfo->farePath() = fp;
  }
  else
  {
    fp = fareDisplayInfo->farePath();
  }

  // Build a fareUsage and populate it with travelSegs from Itin
  FareUsage* fareUsage;
  trx.dataHandle().get(fareUsage);
  if (fareUsage == nullptr)
  {
    LOG4CXX_ERROR(logger, "FDSurchargesRule::process() - UNABLE TO ALLOCATE MEMORY FOR FAREUSAGE");
    LOG4CXX_INFO(logger, "Leaving FDSurchargesRule::validate() - SKIP");
    return SKIP;
  }
  fareUsage->paxTypeFare() = &paxTypeFare;

  FareMarket* outbFareMarket = fareUsage->paxTypeFare()->fareMarket();

  fareUsage->travelSeg() = outbFareMarket->travelSeg();

  // Build a Pricing Unit
  PricingUnit* pricingUnit;
  trx.dataHandle().get(pricingUnit);
  if (pricingUnit == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "FDSurchargesRule::process() - UNABLE TO ALLOCATE MEMORY FOR PRICINGUNIT");
    LOG4CXX_INFO(logger, "Leaving FDSurchargesRule::validate() - SKIP");
    return SKIP;
  }
  pricingUnit->fareUsage().push_back(fareUsage);
  fp->pricingUnit().push_back(pricingUnit);

  // Build a Return Pricing Unit for a RoundTrip Fare
  PricingUnit* returnPricingUnit = nullptr;
  FareUsage* returnFareUsage = nullptr;
  AirSeg* returnSeg = nullptr;

  if ((paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ||
       paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED) &&
      paxTypeFare.fareMarket()->travelSeg().size() == 1)
  {
    trx.dataHandle().get(returnFareUsage);
    if (returnFareUsage == nullptr)
    {
      LOG4CXX_ERROR(logger,
                    "FDSurchargesRule::process() - UNABLE TO ALLOCATE MEMORY FOR RETURNFAREUSAGE");
      LOG4CXX_INFO(logger, "Leaving FDSurchargesRule::validate() - SKIP");
      return SKIP;
    }
    returnFareUsage->paxTypeFare() = &paxTypeFare;

    TravelSeg& tvlSeg = *(fareUsage->travelSeg().front());
    returnSeg = buildReturnSegment(*fdTrx, tvlSeg);
    returnFareUsage->travelSeg().push_back(returnSeg);

    // Build a Return Pricing Unit
    trx.dataHandle().get(returnPricingUnit);
    if (returnPricingUnit == nullptr)
    {
      LOG4CXX_ERROR(
          logger, "FDSurchargesRule::process() - UNABLE TO ALLOCATE MEMORY FOR RETURNPRICINGUNIT");
      LOG4CXX_INFO(logger, "Leaving FDSurchargesRule::validate() - SKIP");
      return SKIP;
    }
    returnPricingUnit->fareUsage().push_back(returnFareUsage);
    fp->pricingUnit().push_back(returnPricingUnit);
  }

  // Validate outbound surcharges
  if (checkValidData(*fdTrx, fp, pricingUnit, fareUsage) &&
      isDirectionPass(rule, fareDisplayInfo->isLocationSwappedForSurcharges(), false))
  {
    validate(trx, *fp, *pricingUnit, *fareUsage, surchInfo);
  }

  // validate inbound surcharges
  if (checkValidData(*fdTrx, fp, returnPricingUnit, returnFareUsage) &&
      isDirectionPass(rule, !(fareDisplayInfo->isLocationSwappedForSurcharges()), true))
  {

    // Temporily use inboundfaremarket for surcharge validation of the inbound segment
    FareMarket* inbFareMarket = nullptr;
    trx.dataHandle().get(inbFareMarket);
    buildInboundFareMarket(paxTypeFare.fareMarket(), inbFareMarket);
    inbFareMarket->travelSeg().push_back(returnSeg);
    fareUsage->paxTypeFare()->fareMarket() = inbFareMarket;

    // Now off to Surcharge validation step
    validate(trx, *fp, *returnPricingUnit, *returnFareUsage, surchInfo);

    // Now restore the original fare market
    fareUsage->paxTypeFare()->fareMarket() = outbFareMarket;
  }

  if (fareUsage->surchargeData().empty() &&
      (returnFareUsage == nullptr || returnFareUsage->surchargeData().empty()))
  {
    LOG4CXX_DEBUG(logger, "No Valid Surcharges Found");
    LOG4CXX_INFO(logger, " Leaving FDSurchargesRule::validate() - SKIP");

    return SKIP;
  }

  // Copy applicable surcharges into FareDisplayInfo object based on Type.
  updateFareInfo(*fdTrx, fareDisplayInfo->outboundSurchargeData(), fareUsage->surchargeData());

  if (returnFareUsage != nullptr)
  {
    updateFareInfo(
        *fdTrx, fareDisplayInfo->inboundSurchargeData(), returnFareUsage->surchargeData());
  }

  if (fareDisplayInfo->inboundSurchargeData().empty() &&
      fareDisplayInfo->outboundSurchargeData().empty())
  {
    LOG4CXX_DEBUG(logger, "No Valid Surcharges Found");
    LOG4CXX_INFO(logger, " Leaving FDSurchargesRule::validate() - SKIP");

    return SKIP;
  }
  else
  {
    return PASS;
  }
}
//*******************************************************************
// processSectorSurcharge
// Collects and updates the sector surcharges
//*******************************************************************
Record3ReturnTypes
FDSurchargesRule::processSectorSurcharge(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  // Get FareDisplayInfo object
  FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (fareDisplayInfo == nullptr)
  {
    LOG4CXX_ERROR(
        logger,
        "FDSurchargesRule::processSectorSurcharge() - Unable to get FareDisplayInfo object");
    return SKIP;
  }

  /*********OUTBOUND SECTOR SURCHARGES***********/

  FareUsage fareUsage;
  Itin* itin = trx.itin().front();
  fareUsage.paxTypeFare() = &paxTypeFare;
  TravelSeg* travelSeg = itin->travelSeg()[0];
  AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
  if (travelSeg == nullptr)
  {
    LOG4CXX_ERROR(logger, "FDSurchargesRule::processSectorSurcharge() - NULL TravelSegment");
    return SKIP;
  }
  airSeg->setMarketingCarrierCode(paxTypeFare.carrier());
  airSeg->setOperatingCarrierCode(paxTypeFare.carrier());

  fareUsage.travelSeg().push_back(airSeg);

  SecSurchargeAppl::SecSurchRuleMap ssrMap;
  SecSurchargeAppl secSurchAppl;

  secSurchAppl.process(trx, paxTypeFare.carrier(), fareUsage, ssrMap);

  if (!fareUsage.surchargeData().empty())
  {
    // Check for duplicates from CAT12 and update oubound surcharges in faredisplayinfo
    addSectorSurcharges(
        trx, fareUsage.surchargeData(), airSeg, fareDisplayInfo->outboundSurchargeData());
    // This should be done after addSectorSurcharges as that step would erase the duplicates from
    // cat 12
    if (trx.isRD())
    {
      updateSectorSurchargeText(fareDisplayInfo, fareUsage.surchargeData(), airSeg);
    }
  }

  fareUsage.travelSeg().clear();
  fareUsage.surchargeData().clear();

  /*********INBOUND SECTOR SURCHARGES***********/

  if ((paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ||
       paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED))
  {
    AirSeg* returnSeg = nullptr;
    if (itin->travelSeg().size() == 2)
      returnSeg = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    else
    {
      TravelSeg& tvlSeg = *(itin->travelSeg().front());
      returnSeg = buildReturnSegment(trx, tvlSeg);
    }
    fareUsage.travelSeg().push_back(returnSeg);

    secSurchAppl.process(trx, paxTypeFare.carrier(), fareUsage, ssrMap);

    if (!fareUsage.surchargeData().empty())
    {
      // Check for duplicates from CAT12 and update inbound surcharges in faredisplayinfo
      addSectorSurcharges(
          trx, fareUsage.surchargeData(), returnSeg, fareDisplayInfo->inboundSurchargeData());
      // This should be done after addSectorSurcharges as that step would erase the duplicates from
      // cat 12
      if (trx.isRD())
      {
        updateSectorSurchargeText(fareDisplayInfo, fareUsage.surchargeData(), returnSeg);
      }
    }
  }
  return PASS;
}

// Copy applicable surcharges into the FareDisplayInfo object.
void
FDSurchargesRule::updateFareInfo(const FareDisplayTrx& fdTrx,
                                 std::vector<SurchargeData*>& fdiSurchargeData,
                                 const std::vector<SurchargeData*>& surchargeData)
{ // lint !e578

  if (surchargeData.empty())
  {
    return;
  }

  std::vector<SurchargeData*>::const_iterator surData = surchargeData.begin();
  std::vector<SurchargeData*>::const_iterator surDataEnd = surchargeData.end();

  for (; surData != surDataEnd; surData++)
  {
    SurchargeData* surcharge = (*surData);

    std::vector<Indicator>::const_iterator j =
        find_if(fdTrx.getOptions()->surchargeTypes().begin(),
                fdTrx.getOptions()->surchargeTypes().end(),
                HasMatchingSurchargeType(surcharge->surchargeType()));

    if (j != fdTrx.getOptions()->surchargeTypes().end())
    {
      fdiSurchargeData.push_back((*surData));
    }
  }
}

/**
*  Check data in the pricing transaction
*  Return SKIP if any of the fields are not populated.
**/
bool
FDSurchargesRule::checkValidData(FareDisplayTrx& fdTrx,
                                 const FarePath* farePath,
                                 const PricingUnit* pricingUnit,
                                 FareUsage* fareUsage)
{

  if (fdTrx.getOptions() == nullptr)
  {
    return false;
  }

  if (fdTrx.getOptions()->surchargeTypes().empty())
  {
    return false;
  }

  if (farePath == nullptr)
  {
    return false;
  }

  if (pricingUnit == nullptr)
  {
    return false;
  }

  if (fareUsage == nullptr)
  {
    return false;
  }

  return true;
}
/**
*  Check for Directionality. This step was skipped for surcharges
* in CategoryRuleItem.
*  Return false if directionality is not passed.
**/
bool
FDSurchargesRule::isDirectionPass(const CategoryRuleItemInfo* cfrItem,
                                  bool isLocationSwapped,
                                  bool isInbound)
{
  if (cfrItem->inOutInd() != RuleConst::ALWAYS_APPLIES)
  {
    if (((cfrItem->inOutInd() == RuleConst::FARE_MARKET_OUTBOUND) && isInbound) ||
        ((cfrItem->inOutInd() == RuleConst::FARE_MARKET_INBOUND) && !isInbound))
    {
      return false;
    }
  }

  // check the GEO_location directionality
  Indicator directionality = cfrItem->directionality();

  if (directionality == RuleConst::ALWAYS_APPLIES)
  {
    return true;
  }

  if (directionality == RuleConst::FROM_LOC1_TO_LOC2)
  {
    if (isLocationSwapped)
    {
      return false;
    }
  }
  else if (directionality == RuleConst::TO_LOC1_FROM_LOC2)
  {
    if (!isLocationSwapped)
    {
      return false;
    }
  }
  else if (directionality == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2)
  {
    if (!isInbound)
    {
      if (isLocationSwapped)
        return false;
    }
    else if (isInbound)
    {
      if (!isLocationSwapped)
        return false;
    }
    else
      return false;
  }
  else if (directionality == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1)
  {
    if (!isInbound)
    {
      if (!isLocationSwapped)
        return false;
    }
    else if (isInbound)
    {
      if (isLocationSwapped)
        return false;
    }
    else
      return false;
  }
  else
  {
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
// buildInboundFareMarket()
//---------------------------------------------------------------------------
void
FDSurchargesRule::buildInboundFareMarket(const FareMarket* fareMarket, FareMarket* inbFareMarket)
{
  LOG4CXX_DEBUG(logger, "Entered FDSurchargesRule::buildInboundFareMarket()");
  inbFareMarket->origin() = fareMarket->destination();
  inbFareMarket->boardMultiCity() = fareMarket->offMultiCity();
  inbFareMarket->destination() = fareMarket->origin();
  inbFareMarket->offMultiCity() = fareMarket->boardMultiCity();
  inbFareMarket->setGlobalDirection(fareMarket->getGlobalDirection());
  inbFareMarket->geoTravelType() = fareMarket->geoTravelType();
  inbFareMarket->direction() = FMDirection::INBOUND;
  inbFareMarket->governingCarrierPref() = fareMarket->governingCarrierPref();
  inbFareMarket->travelBoundary() = fareMarket->travelBoundary();
  inbFareMarket->governingCarrier() = fareMarket->governingCarrier();
  inbFareMarket->travelDate() = fareMarket->travelDate();
}

AirSeg*
FDSurchargesRule::buildReturnSegment(const FareDisplayTrx& trx, const TravelSeg& tvlSeg)
{
  // Build a Return travel segment
  AirSeg* returnSeg = nullptr;
  trx.dataHandle().get(returnSeg);
  if (returnSeg == nullptr)
  {
    LOG4CXX_ERROR(
        logger,
        "FDSurchargesRule::buildReturnSegment() - UNABLE TO ALLOCATE MEMORY FOR RETURNSEGMENT");
    return nullptr;
  }

  returnSeg->segmentOrder() = 1;
  returnSeg->departureDT() = tvlSeg.departureDT();
  returnSeg->origAirport() = tvlSeg.destAirport();
  returnSeg->origin() = tvlSeg.destination();
  returnSeg->destAirport() = tvlSeg.origAirport();
  returnSeg->destination() = tvlSeg.origin();
  returnSeg->boardMultiCity() = tvlSeg.offMultiCity();
  returnSeg->offMultiCity() = tvlSeg.boardMultiCity();

  if (tvlSeg.isAir())
  {
    const AirSeg& airSeg = static_cast<const AirSeg&>(tvlSeg);
    returnSeg->setMarketingCarrierCode(airSeg.marketingCarrierCode());
    returnSeg->setOperatingCarrierCode(airSeg.operatingCarrierCode());
  }
  return returnSeg;
}

void
FDSurchargesRule::addSectorSurcharges(const FareDisplayTrx& trx,
                                      std::vector<SurchargeData*>& sectorSurcharges,
                                      const AirSeg* airSeg,
                                      std::vector<SurchargeData*>& cat12Surcharges)
{
  if (!cat12Surcharges.empty())
  {
    std::vector<SurchargeData*>::iterator new_end = std::remove_if(
        sectorSurcharges.begin(), sectorSurcharges.end(), FoundInCat12(cat12Surcharges, *airSeg));
    sectorSurcharges.erase(new_end, sectorSurcharges.end());
  }
  if (!sectorSurcharges.empty())
  {
    updateFareInfo(trx, cat12Surcharges, sectorSurcharges);
  }
}
void
FDSurchargesRule::updateSectorSurchargeText(FareDisplayInfo* fareDisplayInfo,
                                            const std::vector<SurchargeData*>& sectorSurcharges,
                                            const AirSeg* airSeg)
{
  if (sectorSurcharges.empty())
  {
    return;
  }

  std::vector<SurchargeData*>::const_iterator surData = sectorSurcharges.begin();
  std::vector<SurchargeData*>::const_iterator surDataEnd = sectorSurcharges.end();

  std::ostringstream oss;

  for (; surData != surDataEnd; surData++)
  {
    // The formatting of this text is done to be in alignment with
    // Normal Rule Text Display
    oss << "   FROM " << airSeg->origAirport() << " TO " << airSeg->destAirport() << ", "
        << (*surData)->surchargeDesc() << " OF " << (*surData)->currSelected() << " "
        << (*surData)->amountSelected() << " APPLIES." << std::endl;
  }
  fareDisplayInfo->secSurchargeText() += oss.str();
}
}
