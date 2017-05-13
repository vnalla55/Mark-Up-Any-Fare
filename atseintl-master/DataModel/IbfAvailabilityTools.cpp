//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "DataModel/IbfAvailabilityTools.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TNBrandsTypes.h"
#include "DBAccess/Loc.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"

namespace tse
{

namespace
{
Logger
logger("atseintl.DataModel.IbfAvailabilityTools");
}

void
IbfAvailabilityTools::setBrandsStatus(IbfErrorMessage status,
                                      PaxTypeFare& ptf,
                                      const PricingTrx& trx,
                                      const std::string& location)
{
  if (trx.getTrxType() == PricingTrx::IS_TRX)
    return;

  LOG4CXX_DEBUG(logger,
                "Setting brand status " << status << " for fare " << ptf.fareClass() << " in "
                                        << location);
  ptf.setIbfErrorMessage(status);

  FareMarket* fm = ptf.fareMarket();
  TSE_ASSERT(fm != nullptr);

  // we want all brands which are not explicitly failed, but need hard passes
  // and all brand separately
  std::vector<skipper::BrandCodeDirection> brands;
  std::vector<skipper::BrandCodeDirection> brandsHard;

  // in case of context shopping, if considered pax type fare is on fare market
  // which is on fixed leg, and this fare has any brand we should update status
  // for all possible brands on this fare market as it is fixed and only this
  // brand (and this implies status) can be used for leg/itin status calculation.
  if (UNLIKELY(trx.isContextShopping() &&
      skipper::TNBrandsFunctions::isAnySegmentOnFixedLeg(fm->travelSeg(), trx.getFixedLegs()) &&
      ptf.hasValidBrands(false)))
  {
    for (auto& qualifiedBrand: trx.brandProgramVec())
    {
      skipper::BrandCodeDirection brand(qualifiedBrand.second->brandCode(), Direction::BOTHWAYS);
      brands.push_back(brand);
      brandsHard.push_back(brand);
    }
    //NO_BRAND is used if all legs are fixed
    brands.push_back(skipper::BrandCodeDirection(NO_BRAND, Direction::BOTHWAYS));
    brandsHard.push_back(skipper::BrandCodeDirection(NO_BRAND, Direction::BOTHWAYS));
  }
  else
  {
    ptf.getValidBrands(trx, brands, false);
    ptf.getValidBrands(trx, brandsHard, true);
  }

  if (UNLIKELY(trx.isBRAll()))
  {
    brands.push_back(skipper::BrandCodeDirection(NO_BRAND, Direction::BOTHWAYS));
    brandsHard.push_back(skipper::BrandCodeDirection(NO_BRAND, Direction::BOTHWAYS));
  }

  for (const auto& brandAndDirection : brands)
  {
    LOG4CXX_DEBUG(logger,
                  "Fare market: (" << fm->origin()->loc() << "-" << fm->governingCarrier() << "-"
                                   << fm->destination()->loc() << "), setting brand status "
                                   << status << " for fare " << ptf.fareClass() << " for brand "
                                   << brandAndDirection.brandCode << " with direction "
                                   << directionToString(brandAndDirection.direction)
                                   << " in " << location);
    fm->updateStatusForBrand(brandAndDirection.brandCode, brandAndDirection.direction, status);
  }
  for (const auto& brandAndDirection : brandsHard)
  {
    fm->updateStatusForBrand(brandAndDirection.brandCode, brandAndDirection.direction, status, true);
  }
}

std::map<LegId, IbfErrorMessageSequenceAcc>
IbfAvailabilityTools::calculateLegStatuses(const FareMarketPath& fmp)
{
  LOG4CXX_DEBUG(logger, "Calling IbfAvailabilityTools::calculateErrorsPerLeg");

  std::map<LegId, IbfErrorMessageSequenceAcc> errorsPerLeg;

  for (const MergedFareMarket* mfm : fmp.fareMarketPath())
  {
    const std::set<unsigned int> mfmLegs = IbfAvailabilityTools::getLegsForMfm(*mfm);

    for (const auto mfmLeg : mfmLegs)
    {
      errorsPerLeg[mfmLeg].updateStatus(mfm->getIbfErrorMessage());
      LOG4CXX_DEBUG(logger,
                    "MFM (" << mfm->origin()->loc() << "-" << mfm->destination()->loc()
                            << "), setting " << mfm->getIbfErrorMessage() << " for leg " << mfmLeg
                            << " -> new status: " << errorsPerLeg[mfmLeg].getStatus());
    }
  }
  return errorsPerLeg;
}

void
IbfAvailabilityTools::calculateLegStatusesForMatrix(
    std::vector<std::map<LegId, IbfErrorMessageSequenceAcc> >& result,
    const std::vector<FareMarketPath*>& fareMarketPathMatrix)
{
  result.clear();
  for (FareMarketPath* fmp : fareMarketPathMatrix)
  {
    TSE_ASSERT(fmp != nullptr);
    const std::map<LegId, IbfErrorMessageSequenceAcc> epLegs = calculateLegStatuses(*fmp);
    result.push_back(epLegs);
  }
}

std::map<LegId, IbfErrorMessageChoiceAcc>
IbfAvailabilityTools::summarizeLegStatuses(
    const std::vector<std::map<LegId, IbfErrorMessageSequenceAcc> >& legStatuses)
{
  std::map<LegId, IbfErrorMessageChoiceAcc> finalPerLeg;
  for (const auto& m : legStatuses)
  {
    for (std::map<LegId, IbfErrorMessageSequenceAcc>::const_iterator it = m.begin(); it != m.end();
         ++it)
    {
      finalPerLeg[it->first].updateStatus(it->second.getStatus());
      LOG4CXX_DEBUG(logger, "Updating " << it->second.getStatus() << " for leg "
                    << it->first << " -> new status: " << finalPerLeg[it->first].getStatus());
    }
  }
  return finalPerLeg;
}

IbfErrorMessage
IbfAvailabilityTools::calculateItinStatus(
    const std::vector<std::map<LegId, IbfErrorMessageSequenceAcc> >& legStatuses)
{
  IbfErrorMessageChoiceAcc itinStatus;
  for (const auto& m : legStatuses)
  {
    IbfErrorMessageSequenceAcc legSequenceStatus;
    for (std::map<LegId, IbfErrorMessageSequenceAcc>::const_iterator it = m.begin(); it != m.end();
         ++it)
    {
      legSequenceStatus.updateStatus(it->second.getStatus());
      LOG4CXX_DEBUG(logger, "Updating " << it->second.getStatus() << " for leg "
                    << it->first << " -> new status: " << legSequenceStatus.getStatus());
    }
    itinStatus.updateStatus(legSequenceStatus.getStatus());
    LOG4CXX_DEBUG(logger, "Updating " << legSequenceStatus.getStatus()
                  << " for itin -> new status: " << itinStatus.getStatus());
  }
  return itinStatus.getStatus();
}

IbfErrorMessage
IbfAvailabilityTools::calculateAllStatusesForMatrix(
    std::map<LegId, IbfErrorMessage>& legStatuses,
    const std::vector<FareMarketPath*>& fareMarketPathMatrix)
{
  std::vector<std::map<LegId, IbfErrorMessageSequenceAcc> > legStatusesForMatrix;
  calculateLegStatusesForMatrix(legStatusesForMatrix, fareMarketPathMatrix);
  std::map<LegId, IbfErrorMessageChoiceAcc> finalPerLeg =
      summarizeLegStatuses(legStatusesForMatrix);

  legStatuses.clear();
  for (std::map<LegId, IbfErrorMessageChoiceAcc>::const_iterator it = finalPerLeg.begin();
       it != finalPerLeg.end();
       ++it)
  {
    legStatuses[it->first] = it->second.getStatus();
  }
  return calculateItinStatus(legStatusesForMatrix);
}

std::set<unsigned int>
IbfAvailabilityTools::getLegsForMfm(const MergedFareMarket& mfm)
{
  std::set<unsigned int> legs;
  for (TravelSeg* tSeg : mfm.travelSeg())
  {
    TSE_ASSERT(tSeg != nullptr);
    legs.insert(tSeg->legId());
  }
  for (const std::vector<TravelSeg*>& sideTrips : mfm.sideTripTravelSeg())
    for (TravelSeg* tSeg : sideTrips)
    {
      TSE_ASSERT(tSeg != nullptr);
      legs.insert(tSeg->legId());
    }
  return legs;
}

IbfErrorMessage
IbfAvailabilityTools::translateForOutput(const IbfErrorMessage msg)
{
  switch (msg)
  {
  case IbfErrorMessage::IBF_EM_NO_FARE_FILED:
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  case IbfErrorMessage::IBF_EM_EARLY_DROP:
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  case IbfErrorMessage::IBF_EM_NOT_OFFERED:
    return IbfErrorMessage::IBF_EM_NOT_OFFERED;
  case IbfErrorMessage::IBF_EM_NOT_AVAILABLE:
    return IbfErrorMessage::IBF_EM_NOT_AVAILABLE;
  case IbfErrorMessage::IBF_EM_NO_FARE_FOUND:
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  case IbfErrorMessage::IBF_EM_NOT_SET:
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  default:
    LOG4CXX_ERROR(logger, "Unknown value of IbfErrorMessage = " << msg);
    TSE_ASSERT(!"Unknown value of IbfErrorMessage");
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  }
}

IbfErrorMessage
IbfAvailabilityTools::translateForOutput_newCbs(const IbfErrorMessage msg)
{
  switch (msg)
  {
  case IbfErrorMessage::IBF_EM_NO_FARE_FILED:
    return IbfErrorMessage::IBF_EM_NO_FARE_FILED;
  case IbfErrorMessage::IBF_EM_EARLY_DROP:
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  case IbfErrorMessage::IBF_EM_NOT_OFFERED:
    return IbfErrorMessage::IBF_EM_NOT_OFFERED;
  case IbfErrorMessage::IBF_EM_NOT_AVAILABLE:
    return IbfErrorMessage::IBF_EM_NOT_AVAILABLE;
  case IbfErrorMessage::IBF_EM_NO_FARE_FOUND:
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  case IbfErrorMessage::IBF_EM_NOT_SET:
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  default:
    LOG4CXX_ERROR(logger, "Unknown value of IbfErrorMessage = " << msg);
    TSE_ASSERT(!"Unknown value of IbfErrorMessage");
    return IbfErrorMessage::IBF_EM_NO_FARE_FOUND;
  }
}

void
IbfAvailabilityTools::calculateStatusesForFareMarketTravelSegments(
  const FareMarket* fm, const BrandCode& brand,
  std::map<const TravelSeg*, IbfErrorMessageChoiceAcc>& travelSegStatus)
{
  LOG4CXX_DEBUG(logger, "Updating " << fm->origin()->loc() << "-" << fm->destination()->loc()
                        << " status");

  //code used only in BrandedFaresRequests so BOTHWAYS
  IbfErrorMessage status = fm->getStatusForBrand(brand, Direction::BOTHWAYS);
  for (const TravelSeg* tSeg: fm->travelSeg())
  {
    TSE_ASSERT(tSeg != nullptr);
    travelSegStatus[tSeg].updateStatus(status);
    LOG4CXX_DEBUG(logger, "Updating travel segment " << tSeg->origin()->loc() << "-"
                          << tSeg->destination()->loc() << " with status " << status
                          << " -> new status: " << travelSegStatus[tSeg].getStatus());
  }
}

IbfErrorMessage
IbfAvailabilityTools::calculateAllStatusesForItinBrand(const Itin* itin,
  const BrandCode& brand, std::map<LegId, IbfErrorMessage>& legStatuses)
{
  TSE_ASSERT(itin != nullptr);

  std::map<const TravelSeg*, IbfErrorMessageChoiceAcc> travelSegStatus;
  std::map<LegId, IbfErrorMessageSequenceAcc> legsStatus;

  // travel segments statuses
  for (const FareMarket* fm: itin->fareMarket())
  {
    TSE_ASSERT(fm != nullptr);
    calculateStatusesForFareMarketTravelSegments(fm, brand, travelSegStatus);
  }

  // legs statuses
  for (const auto& tSegStatus: travelSegStatus)
  {
    legsStatus[tSegStatus.first->legId()].updateStatus(tSegStatus.second.getStatus());
    LOG4CXX_DEBUG(logger, "Updating leg " << tSegStatus.first->legId()
                          << " with status " << tSegStatus.second.getStatus()
                          << " -> new status: " << legsStatus[tSegStatus.first->legId()].getStatus());
  }

  IbfErrorMessageSequenceAcc itinStatus;
  legStatuses.clear();
  // itin status and legs status placed in result variable
  for (const auto& legStatus: legsStatus)
  {
      legStatuses[legStatus.first] = legStatus.second.getStatus();
      itinStatus.updateStatus(legStatus.second.getStatus());
      LOG4CXX_DEBUG(logger, "Updating " << legStatus.second.getStatus()
                    << " for itin -> new status: " << itinStatus.getStatus());
  }
 return itinStatus.getStatus();
}

void
IbfAvailabilityTools::updateIbfAvailabilitySoldoutsForItin(
  Itin* itin, const BrandCode& brandCode)
{
  TSE_ASSERT(itin != nullptr);

  std::map<LegId, IbfErrorMessage> finalPerLeg;
  const IbfErrorMessage itinStatus = calculateAllStatusesForItinBrand(
      itin, brandCode, finalPerLeg);
  itin->getMutableIbfAvailabilityTracker().setStatus(brandCode, itinStatus);
  for (const auto& legErrorElement : finalPerLeg)
  {
    itin->getMutableIbfAvailabilityTracker().setStatusForLeg(
        brandCode, legErrorElement.first, legErrorElement.second);
  }
}

} // namespace tse
