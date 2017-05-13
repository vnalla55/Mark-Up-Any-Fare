#include "MinFares/MinFareLogic.h"

#include "Common/DiagMonitor.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/MinFareFareTypeGrp.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "DBAccess/RuleItemInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "MinFares/HIPMinimumFare.h"
#include "MinFares/MatchDefaultLogicTable.h"
#include "MinFares/MatchExceptionTable.h"
#include "MinFares/MatchRuleLevelExclTable.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/MinFareSpecialFareSelection.h"
#include "MinFares/MinimumFare.h"
#include "Rules/RuleUtil.h"


namespace tse
{
static Logger
logger("atseintl.MinFares.MinFareLogic");

bool
MinFareLogic::isMixedCabin(const PricingUnit& pu, CabinType& lowestCabin)
{
  bool mixedCabin = false;
  const std::vector<FareUsage*>& fus = pu.fareUsage();
  for (const auto fu : fus)
  {
    if (!fu)
      continue;

    const PaxTypeFare* paxTypeFare = fu->paxTypeFare(); // lint !e530
    if (!paxTypeFare)
      continue;

    CabinType cabin(paxTypeFare->cabin());
    if (cabin.isUnknownClass())
    {
      continue;
    }

    if (lowestCabin.isUnknownClass())
    {
      lowestCabin = cabin;
    }
    else if (cabin != lowestCabin)
    {
      mixedCabin = true;
      lowestCabin = (cabin > lowestCabin ? cabin : lowestCabin);
    }
  }

  return mixedCabin;
}

MinFareFareSelection::EligibleFare
MinFareLogic::eligibleFare(const PricingUnit& pu)
{
  return (pu.puType() == PricingUnit::Type::ONEWAY ? MinFareFareSelection::ONE_WAY
                                             : MinFareFareSelection::HALF_ROUND_TRIP);
}

MinFareFareSelection::FareDirectionChoice
MinFareLogic::fareDirection(const FareUsage& fu)
{
  return (fu.isInbound() ? MinFareFareSelection::INBOUND : MinFareFareSelection::OUTBOUND);
}

const PtfPair
MinFareLogic::selectQualifyConstFare(MinimumFareModule module,
                                     PricingTrx& trx,
                                     const Itin& itin,
                                     const PaxTypeFare& paxTypeFare,
                                     const PaxType& requestedPaxType,
                                     const CabinType& cabin,
                                     bool selectNormalFare,
                                     MinFareFareSelection::FareDirectionChoice fareDirection,
                                     MinFareFareSelection::EligibleFare eligibleFare,
                                     const std::vector<TravelSeg*>& travelSeg,
                                     const DateTime& travelDate,
                                     const MinFareAppl* matchedApplItem,
                                     const MinFareDefaultLogic* matchedDefaultItem,
                                     const RepricingTrx* repricingTrx,
                                     const FarePath* farePath,
                                     const PricingUnit* pu,
                                     const PaxTypeCode& actualPaxTypeCode)
{

  // PL 8754 and 9715: when selecting INF or CNN fare, if the thru fare is
  // of higher pax type (CNN or ADT), then the same pax fare type should be
  // used. ie. if thru fare is ADT, then ADT fare should be used even though
  // INF or CNN fare is requested.
  PaxTypeStatus reqPaxTypeStatus =
      PaxTypeUtil::paxTypeStatus(requestedPaxType.paxType(), paxTypeFare.vendor());
  PaxTypeStatus paxTypeStatus = PAX_TYPE_STATUS_UNKNOWN;
  if (paxTypeFare.paxType())
  {
    paxTypeStatus =
        PaxTypeUtil::paxTypeStatus(paxTypeFare.paxType()->paxType(), paxTypeFare.vendor());
  }
  else if (farePath && farePath->paxType())
  {
    paxTypeStatus =
        PaxTypeUtil::paxTypeStatus(farePath->paxType()->paxType(), paxTypeFare.vendor());
  }

  if (reqPaxTypeStatus == PAX_TYPE_STATUS_UNKNOWN || reqPaxTypeStatus > paxTypeStatus)
  {
    reqPaxTypeStatus = paxTypeStatus;
  }

  std::vector<PricingUnit*> pricingUnits;
  if (pu != nullptr)
    pricingUnits.push_back(const_cast<PricingUnit*>(pu));

  if (selectNormalFare)
  {
    MinFareNormalFareSelection fareSelector(module,
                                            eligibleFare,
                                            fareDirection,
                                            cabin,
                                            trx,
                                            itin,
                                            travelSeg,
                                            pricingUnits,
                                            &requestedPaxType,
                                            travelDate,
                                            farePath,
                                            &paxTypeFare,
                                            matchedApplItem,
                                            matchedDefaultItem,
                                            repricingTrx,
                                            actualPaxTypeCode);
    return fareSelector.newSelectFare(reqPaxTypeStatus);
  }
  else
  {
    MinFareSpecialFareSelection fareSelector(module,
                                             eligibleFare,
                                             fareDirection,
                                             paxTypeFare.fcaFareType(),
                                             trx,
                                             itin,
                                             travelSeg,
                                             pricingUnits,
                                             &requestedPaxType,
                                             travelDate,
                                             farePath,
                                             &paxTypeFare,
                                             matchedApplItem,
                                             matchedDefaultItem,
                                             repricingTrx,
                                             actualPaxTypeCode);

    return fareSelector.newSelectFare(reqPaxTypeStatus);
  }
}

const PaxTypeFare*
MinFareLogic::selectQualifyFare(MinimumFareModule module,
                                PricingTrx& trx,
                                const Itin& itin,
                                const PaxTypeFare& paxTypeFare,
                                const PaxType& requestedPaxType,
                                const CabinType& cabin,
                                bool selectNormalFare,
                                MinFareFareSelection::FareDirectionChoice fareDirection,
                                MinFareFareSelection::EligibleFare eligibleFare,
                                const std::vector<TravelSeg*>& travelSeg,
                                const DateTime& travelDate,
                                const MinFareAppl* matchedApplItem,
                                const MinFareDefaultLogic* matchedDefaultItem,
                                const RepricingTrx* repricingTrx,
                                const FarePath* farePath,
                                const PricingUnit* pu,
                                const PaxTypeCode& actualPaxTypeCode)
{
  // Select fare for Nigeria Currency Adjustment.
  DiagManager diag(trx, Diagnostic776);
  if (module == NCJ)
  {
    if (diag.isActive())
    {
      diag << "NIGERIA CURRENCY ADJUSTMENT:\n";
      diag << paxTypeFare.tcrTariffCat();
      MinimumFare::printFareInfo(paxTypeFare, diag.collector(), NCJ, " ");
    }

    const PaxTypeFare* ptf = nullptr;
    ptf = getRevNigeriaFare(trx, *farePath, *pu, &paxTypeFare, travelDate);
    if (ptf != nullptr)
    {
      LOG4CXX_DEBUG(logger, "Found fare by getRevNigeriaFare");

      if (diag.isActive())
      {
        diag << ptf->tcrTariffCat();
        MinimumFare::printFareInfo(*ptf, diag.collector(), NCJ, " ");
      }
      return ptf;
    }

    LOG4CXX_DEBUG(logger, "No Fare Found by getRevNigeriaFare, try default logic...");
  }

  // PL 8754 and 9715: when selecting INF or CNN fare, if the thru fare is
  // of higher pax type (CNN or ADT), then the same pax fare type should be
  // used. ie. if thru fare is ADT, then ADT fare should be used even though
  // INF or CNN fare is requested.
  PaxTypeStatus reqPaxTypeStatus =
      PaxTypeUtil::paxTypeStatus(requestedPaxType.paxType(), paxTypeFare.vendor());
  PaxTypeStatus paxTypeStatus = PAX_TYPE_STATUS_UNKNOWN;
  if (paxTypeFare.paxType())
  {
    paxTypeStatus =
        PaxTypeUtil::paxTypeStatus(paxTypeFare.paxType()->paxType(), paxTypeFare.vendor());
  }
  else if (farePath && farePath->paxType())
  {
    paxTypeStatus =
        PaxTypeUtil::paxTypeStatus(farePath->paxType()->paxType(), paxTypeFare.vendor());
  }

  if (reqPaxTypeStatus == PAX_TYPE_STATUS_UNKNOWN || reqPaxTypeStatus > paxTypeStatus)
  {
    reqPaxTypeStatus = paxTypeStatus;
  }

  std::vector<PricingUnit*> pricingUnits;
  if (pu != nullptr)
    pricingUnits.push_back(const_cast<PricingUnit*>(pu));

  const PaxTypeFare* retPaxTypeFare = nullptr;

  if (selectNormalFare)
  {
    MinFareNormalFareSelection fareSelector(module,
                                            eligibleFare,
                                            fareDirection,
                                            cabin,
                                            trx,
                                            itin,
                                            travelSeg,
                                            pricingUnits,
                                            &requestedPaxType,
                                            travelDate,
                                            farePath,
                                            &paxTypeFare,
                                            matchedApplItem,
                                            matchedDefaultItem,
                                            repricingTrx,
                                            actualPaxTypeCode);

    retPaxTypeFare = fareSelector.selectFare(reqPaxTypeStatus, (module == NCJ));
  }
  else
  {
    MinFareSpecialFareSelection fareSelector(module,
                                             eligibleFare,
                                             fareDirection,
                                             paxTypeFare.fcaFareType(),
                                             trx,
                                             itin,
                                             travelSeg,
                                             pricingUnits,
                                             &requestedPaxType,
                                             travelDate,
                                             farePath,
                                             &paxTypeFare,
                                             matchedApplItem,
                                             matchedDefaultItem,
                                             repricingTrx,
                                             actualPaxTypeCode);

    retPaxTypeFare = fareSelector.selectFare(reqPaxTypeStatus, (module == NCJ));
  }

  if (module == NCJ)
  {
    if (diag.isActive())
    {
      if (retPaxTypeFare != nullptr)
      {
        diag << retPaxTypeFare->tcrTariffCat();
        MinimumFare::printFareInfo(*retPaxTypeFare, diag.collector(), NCJ, " ");
      }
      else
      {
        MinimumFare::printExceptionInfo(paxTypeFare, diag.collector());
        diag << "NO FARE FOUND\n";
      }
      diag << " \n";
    }
  }

  return retPaxTypeFare;
}

MoneyAmount
MinFareLogic::selectLowestCabinFare(DiagCollector* diag,
                                    MinimumFareModule module,
                                    const PricingUnit& pu,
                                    const CabinType& lowestCabin,
                                    PricingTrx& trx,
                                    const FarePath& farePath,
                                    const PaxType& requestedPaxType,
                                    const DateTime& travelDate,
                                    bool selectNormalFare,
                                    const MinFareAppl* matchedApplItem,
                                    const MinFareDefaultLogic* matchedDefaultItem,
                                    const RepricingTrx* repricingTrx)
{
  MoneyAmount lowestCabinFareAmount = 0.0;

  const std::vector<FareUsage*>& fus = pu.fareUsage();

  for (const auto fu : fus)
  {
    if (!fu)
      continue;

    const FareUsage& fareUsage = *fu;
    const PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare(); // lint !e530
    const FareMarket* fareMarket = paxTypeFare.fareMarket();
    const std::vector<TravelSeg*>& tvlSeg = fareMarket->travelSeg();

    if (lowestCabin == paxTypeFare.cabin())
    {
      lowestCabinFareAmount += (paxTypeFare.nucFareAmount() + paxTypeFare.mileageSurchargeAmt() +
                                fareUsage.minFarePlusUp().getSum(HIP));

      continue;
    }

    // Get the repriced fare before doing fare selection...
    FareUsage* repricedFu = nullptr;
    const PaxTypeFare* repricedNormalFare = MinFareLogic::getRepricedNormalFare(
        trx,
        farePath,
        pu,
        fareUsage, // original fare Usage
        repricedFu, // repriced fare usage to hold the new HIP amount
        MinFareLogic::eligibleFare(pu),
        lowestCabin,
        travelDate);

    // ...select fare with the repriced fare:
    const PaxTypeFare* lowestCabinFare =
        MinFareLogic::selectQualifyFare(module,
                                        trx,
                                        *(farePath.itin()),
                                        *repricedNormalFare,
                                        requestedPaxType,
                                        lowestCabin,
                                        selectNormalFare,
                                        MinFareLogic::fareDirection(fareUsage),
                                        MinFareLogic::eligibleFare(pu),
                                        tvlSeg,
                                        travelDate,
                                        matchedApplItem,
                                        matchedDefaultItem,
                                        repricingTrx,
                                        &farePath,
                                        &pu);

    if (!lowestCabinFare)
    {
      // TODO:
      lowestCabinFareAmount += (paxTypeFare.nucFareAmount() + paxTypeFare.mileageSurchargeAmt() +
                                fareUsage.minFarePlusUp().getSum(HIP));

      if (diag && diag->isActive())
      {
        *diag << "NO REPRICED FARE FOUND - USE SAME FARE\n";
        MinimumFare::printFareInfo(paxTypeFare, *diag, module, "  ");
      }
    }
    else
    {
      if (diag && diag->isActive())
      {
        *diag << "REPRICED TO LOWER CABIN\n";
        MinimumFare::printFareInfo(*lowestCabinFare, *diag, module, "  ");
      }

      // DO HIP check on the new Fare:
      FareUsage tmpFareUsage;
      tmpFareUsage.paxTypeFare() = const_cast<PaxTypeFare*>(lowestCabinFare);

      HIPMinimumFare hip(trx);
      hip.process(tmpFareUsage, pu, farePath);

      lowestCabinFareAmount +=
          (lowestCabinFare->nucFareAmount() + lowestCabinFare->mileageSurchargeAmt() +
           tmpFareUsage.minFarePlusUp().getSum(HIP));
    }
  }

  return lowestCabinFareAmount;
}

const PaxTypeFare*
MinFareLogic::getRepricedNormalFare(PricingTrx& trx,
                                    const FarePath& farePath,
                                    const PricingUnit& pu,
                                    const FareUsage& originalFu,
                                    FareUsage*& repricedFu,
                                    MinFareFareSelection::EligibleFare eligibleFare,
                                    const CabinType& lowestCabin,
                                    const DateTime& travelDate,
                                    bool sameFareClass)
{
  // Select the lowestCabin normal fare from the same market as originalFare
  // that passing all rules except for BookingCode.
  const PaxTypeFare& originalFare = *originalFu.paxTypeFare();

  const PaxTypeBucket* paxTypeCortege =
      originalFare.fareMarket()->paxTypeCortege(farePath.paxType()); // lint !e1561
  if (paxTypeCortege == nullptr)
    return &originalFare;

  PaxTypeFare* lowestCabinFare = nullptr;

  const std::vector<PaxTypeFare*>& fares = paxTypeCortege->paxTypeFare();
  std::vector<PaxTypeFare*>::const_iterator iter = fares.begin();
  for (; iter != fares.end(); iter++)
  {
    PaxTypeFare* curFare = *iter;
    if (curFare->isNormal() && (curFare->directionality() == originalFare.directionality()) &&
        (curFare->cabin() == lowestCabin) &&
        (!sameFareClass || (originalFare.fareClass() == curFare->fareClass())) &&
        MinFareFareSelection::isEligibleFare(*curFare, originalFare, eligibleFare) &&
        curFare->isValidNoBookingCode())
    {
      lowestCabinFare = curFare;
      break;
    }
  }

  if (lowestCabinFare == nullptr)
  {
    repricedFu = const_cast<FareUsage*>(&originalFu);
    return &originalFare;
  }

  // Do HIP(&BHC) for the new fare
  trx.dataHandle().get(repricedFu);
  if (!repricedFu)
  {
    LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (repricedFu)");
  }
  else
  {
    // Only copy info as needed from original FareUsage
    repricedFu->paxTypeFare() = lowestCabinFare;
    repricedFu->inbound() = (const_cast<FareUsage*>(&originalFu))->inbound();

    // Check for Nigerian fare
    const PaxTypeFare* adjustedFare = nullptr;
    std::vector<TravelSeg*>& travelSegs = lowestCabinFare->fareMarket()->travelSeg();
    if (pu.nigeriaCurrencyAdjustment() &&
        ((repricedFu->isOutbound() && (travelSegs.front()->origin()->nation() == NIGERIA)) ||
         (repricedFu->isInbound() && (travelSegs.back()->destination()->nation() == NIGERIA))))
    {
      adjustedFare = RuleUtil::selectAdjustedFare(trx,
                                                  const_cast<FarePath&>(farePath),
                                                  const_cast<PricingUnit&>(pu),
                                                  *repricedFu,
                                                  travelDate);
    }

    if (adjustedFare &&
        (((adjustedFare->nucFareAmount() + adjustedFare->mileageSurchargeAmt()) -
          (lowestCabinFare->nucFareAmount() + lowestCabinFare->mileageSurchargeAmt())) > 0))
    {
      repricedFu->adjustedPaxTypeFare() = adjustedFare;
    }

    HIPMinimumFare hip(trx);
    hip.process(*repricedFu, pu, farePath);
  }

  return lowestCabinFare;
}

bool
MinFareLogic::checkDomesticExclusion(PricingTrx& trx,
                                     const Itin& itin,
                                     const PaxTypeFare& paxTypeFare,
                                     const MinFareAppl* applItem,
                                     const MinFareDefaultLogic* defLogicItem,
                                     const std::vector<TravelSeg*>& tvlSeg)
{
  bool ret = isDomestic(itin, tvlSeg);
  if (!ret)
    return false;

  const std::vector<FareType>* fareTypes = nullptr;

  if (defLogicItem != nullptr)
  {
    if (defLogicItem->domExceptInd() == YES)
    {
      ret = LocUtil::isInLoc(tvlSeg.front()->origin()->loc(),
                             defLogicItem->domLoc().locType(),
                             defLogicItem->domLoc().loc(),
                             Vendor::SABRE,
                             MANUAL,
                             GeoTravelType::International,
                             LocUtil::OTHER,
                             trx.getRequest()->ticketingDT());

      if (defLogicItem->domAppl() == MinimumFare::EXCLUDE)
      {
        ret = !ret;
      }
    }
    else
    {
      if (defLogicItem->domAppl() == MinimumFare::INCLUDE)
      {
        ret = !ret;
      }
    }

    if (defLogicItem->domFareTypeExcept() == YES)
    {
      fareTypes = &(defLogicItem->fareTypes());
    }
  }
  else
  {
    if (applItem->domExceptInd() == YES)
    {
      ret = LocUtil::isInLoc(tvlSeg.front()->origin()->loc(),
                             applItem->domLoc().locType(),
                             applItem->domLoc().loc(),
                             Vendor::SABRE,
                             MANUAL,
                             GeoTravelType::International,
                             LocUtil::OTHER,
                             trx.getRequest()->ticketingDT());

      if (applItem->domAppl() == MinimumFare::EXCLUDE)
      {
        ret = !ret;
      }
    }
    else
    {
      if (applItem->domAppl() == MinimumFare::INCLUDE)
      {
        ret = !ret;
      }
    }

    if (applItem->domFareTypeExcept() == YES)
    {
      fareTypes = &(applItem->domFareTypes());
    }
  }

  if (fareTypes && !fareTypes->empty())
  {
    for (const auto& fareType : *fareTypes)
    {
      if (!fareType.empty() && !RuleUtil::matchFareType(fareType, paxTypeFare.fcaFareType()))
      {
        ret = !ret;
        break;
      }
    }
  }

  return ret;
}

const MinFareRuleLevelExcl*
MinFareLogic::getRuleLevelExcl(PricingTrx& trx,
                               const Itin& itin,
                               const PaxTypeFare& paxTypeFare,
                               MinimumFareModule module,
                               std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleMap,
                               const DateTime& travelDate)
{
  const MinFareRuleLevelExcl* retRule = nullptr;
  uint16_t mapKey = getMapKey(paxTypeFare, itin);

  // Search the saved items first
  std::multimap<uint16_t, const MinFareRuleLevelExcl*>::const_iterator iter;
  std::multimap<uint16_t, const MinFareRuleLevelExcl*>::const_iterator lowerPos =
      ruleMap.lower_bound(mapKey);
  std::multimap<uint16_t, const MinFareRuleLevelExcl*>::const_iterator upperPos =
      ruleMap.upper_bound(mapKey);
  for (iter = lowerPos; iter != upperPos; iter++)
  {
    const MinFareRuleLevelExcl* curRule = iter->second;
    Indicator applInd = MinFareLogic::getMinFareExclIndicator(module, *curRule);

    if (applInd != MinimumFare::BLANK)
      return curRule;
  }

  // Retreve the item from datahandle
  MatchRuleLevelExclTable matchedRuleLevelExcl(module, trx, itin, paxTypeFare, travelDate);
  if (!matchedRuleLevelExcl())
    return retRule;

  retRule = matchedRuleLevelExcl.matchedRuleItem();

  // Check if this item can be used by multiple modules.
  // If yes, save it to the map.
  uint16_t numApply = 0;
  if (LIKELY(retRule->hipMinFareAppl() != MinimumFare::BLANK))
    numApply++;
  if (LIKELY(retRule->ctmMinFareAppl() != MinimumFare::BLANK))
    numApply++;
  if (LIKELY(retRule->backhaulMinFareAppl() != MinimumFare::BLANK))
    numApply++;
  if (LIKELY(retRule->dmcMinFareAppl() != MinimumFare::BLANK))
    numApply++;
  if (LIKELY(retRule->comMinFareAppl() != MinimumFare::BLANK))
    numApply++;
  if (LIKELY(retRule->copMinFareAppl() != MinimumFare::BLANK))
    numApply++;
  if (LIKELY(retRule->cpmMinFareAppl() != MinimumFare::BLANK))
    numApply++;

  if (LIKELY(numApply > 1))
  {
    ruleMap.insert(
        std::pair<uint16_t, const MinFareRuleLevelExcl*>(getMapKey(paxTypeFare, itin), retRule));
  }

  return retRule;
}

const MinFareAppl*
MinFareLogic::getApplication(PricingTrx& trx,
                             const FarePath& farePath,
                             const Itin& itin,
                             const PaxTypeFare& paxTypeFare,
                             MinimumFareModule module,
                             std::multimap<uint16_t, const MinFareAppl*>& applMap,
                             std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap,
                             const DateTime& travelDate)
{
  const MinFareAppl* retAppl = nullptr;
  uint16_t mapKey = getMapKey(paxTypeFare, itin);

  // Search the saved items first
  std::multimap<uint16_t, const MinFareAppl*>::const_iterator iter;
  std::multimap<uint16_t, const MinFareAppl*>::const_iterator lowerPos =
      applMap.lower_bound(mapKey);
  std::multimap<uint16_t, const MinFareAppl*>::const_iterator upperPos =
      applMap.upper_bound(mapKey);

  for (iter = lowerPos; iter != upperPos; iter++)
  {
    const MinFareAppl* curAppl = iter->second;
    Indicator applInd = MinFareLogic::getMinFareApplIndicator(module, *curAppl);

    if (applInd != MinimumFare::BLANK)
      return curAppl;
  }

  // Retreve the item from datahandle
  MatchExceptionTable matchExceptionTable(module,
                                          trx,
                                          farePath,
                                          itin,
                                          paxTypeFare,
                                          travelDate,
                                          paxTypeFare.fareMarket()->governingCarrier());

  if (UNLIKELY(!matchExceptionTable()))
    return retAppl;

  retAppl = matchExceptionTable.matchedApplItem();
  const MinFareDefaultLogic* defaultLogic = matchExceptionTable.matchedDefaultItem();
  if (defaultLogic != nullptr)
  {
    defaultLogicMap.insert(std::make_pair(mapKey, defaultLogic));
  }

  // Check if this item can be used by multiple modules.
  // If yes, save it to the map.
  if (LIKELY(retAppl))
  {
    uint16_t numApply = 0;
    if (retAppl->hipCheckAppl() != MinimumFare::BLANK)
      numApply++;
    if (retAppl->ctmCheckAppl() != MinimumFare::BLANK)
      numApply++;
    if (retAppl->backhaulCheckAppl() != MinimumFare::BLANK)
      numApply++;
    if (retAppl->dmcCheckAppl() != MinimumFare::BLANK)
      numApply++;
    if (retAppl->comCheckAppl() != MinimumFare::BLANK)
      numApply++;
    if (retAppl->cpmCheckAppl() != MinimumFare::BLANK)
      numApply++;

    if (numApply > 1)
    {
      applMap.insert(std::make_pair(mapKey, retAppl));
    }
  }

  return retAppl;
}

const MinFareDefaultLogic*
MinFareLogic::getDefaultLogic(MinimumFareModule module,
                              PricingTrx& trx,
                              const Itin& itin,
                              const PaxTypeFare& paxTypeFare,
                              std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap)
{
  const MinFareDefaultLogic* retLogic = nullptr;
  uint16_t mapKey = getMapKey(paxTypeFare, itin);

  // Search the saved items first
  std::map<uint16_t, const MinFareDefaultLogic*>::const_iterator iter =
      defaultLogicMap.find(mapKey);
  if (iter != defaultLogicMap.end())
    return iter->second;

  // Retreve the item from datahandle
  MatchDefaultLogicTable matchDefaultLogicTable(module, trx, itin, paxTypeFare);
  if (!matchDefaultLogicTable())
    return retLogic;

  retLogic = matchDefaultLogicTable.matchedDefaultItem();
  if (retLogic != nullptr)
    defaultLogicMap.insert(
        std::pair<uint16_t, const MinFareDefaultLogic*>(getMapKey(paxTypeFare, itin), retLogic));

  return retLogic;
}

uint16_t
MinFareLogic::getMapKey(const PaxTypeFare& paxTypeFare, const Itin& itin)
{
  return itin.segmentOrder(paxTypeFare.fareMarket()->travelSeg().front()) * 100 +
         itin.segmentOrder(paxTypeFare.fareMarket()->travelSeg().back());
}

Indicator
MinFareLogic::getMinFareApplIndicator(MinimumFareModule module, const MinFareAppl& appl)
{
  switch (module)
  {
  case HIP:
    return appl.hipCheckAppl();
  case BHC:
    return appl.backhaulCheckAppl();
  case CTM:
    return appl.ctmCheckAppl();
  case COM:
    return appl.comCheckAppl();
  case DMC:
    return appl.dmcCheckAppl();
  case CPM:
    return appl.cpmCheckAppl();
  default:
    return MinimumFare::BLANK;
  }
}

Indicator
MinFareLogic::getMinFareExclIndicator(MinimumFareModule module, const MinFareRuleLevelExcl& rule)
{
  switch (module)
  {
  case HIP:
    return rule.hipMinFareAppl();
  case BHC:
    return rule.backhaulMinFareAppl();
  case CTM:
    return rule.ctmMinFareAppl();
  case COM:
    return rule.comMinFareAppl();
  case DMC:
    return rule.dmcMinFareAppl();
  case COP:
    return rule.copMinFareAppl();
  case CPM:
    return rule.cpmMinFareAppl();
  case OSC:
  case RSC:
    return 'Y'; // No fields for OSC/RSC
  default:
    return MinimumFare::BLANK;
  }
}

GlobalDirection
MinFareLogic::getGlobalDirection(PricingTrx& trx,
                                 const std::vector<TravelSeg*>& tvlSegs,
                                 DateTime travelDate)
{
  GlobalDirection dir;
  if (GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, tvlSegs, dir))
    return dir;

  return GlobalDirection::XX;
}

PaxTypeStatus
MinFareLogic::paxTypeStatus(const PaxTypeFare& fare)
{
  if (fare.paxType())
    return PaxTypeUtil::paxTypeStatus(fare.paxType()->paxType(), fare.vendor());
  else
    return PAX_TYPE_STATUS_UNKNOWN;
}

PaxTypeStatus
MinFareLogic::paxTypeStatus(const PricingUnit& pu)
{
  PaxTypeStatus retStatus = PAX_TYPE_STATUS_ADULT;
  for (FareUsage* fu : pu.fareUsage())
  {
    PaxTypeFare* fare = fu->paxTypeFare();
    if (fare == nullptr)
      continue;

    PaxTypeStatus farePaxTypeStatus = paxTypeStatus(*fare);
    if (farePaxTypeStatus > retStatus)
      retStatus = farePaxTypeStatus;
  }

  return retStatus;
}

class NG_Normal_Pred
{
public:
  NG_Normal_Pred(const PaxTypeFare& paxTypeFare, MinFareFareSelection::EligibleFare eligibleFare)
    : _ptf(paxTypeFare), _eligibleFare(eligibleFare)
  {
  }
  bool operator()(const PaxTypeFare* paxTypeFare)
  {
    if (!paxTypeFare->isValidNoBookingCode())
      return false;

    if (!paxTypeFare->isNormal())
    {
      return false;
    }

    if (!((_ptf.directionality() == FROM) && (paxTypeFare->directionality() == TO)) &&
        !((_ptf.directionality() == TO) && (paxTypeFare->directionality() == FROM)))
    {
      return false;
    }

    if (_ptf.tcrTariffCat() != RuleConst::PRIVATE_TARIFF &&
        paxTypeFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    {
      return false;
    }

    if (!MinFareFareSelection::isEligibleFare(*paxTypeFare, _ptf, _eligibleFare))
    {
      return false;
    }

    if (_ptf.fcaFareType() != paxTypeFare->fcaFareType())
    {
      return false;
    }

    if (paxTypeFare->cabin() != _ptf.cabin())
    {
      return false;
    }

    return true;
  }

private:
  const PaxTypeFare& _ptf;
  MinFareFareSelection::EligibleFare _eligibleFare;
};

class NG_Special_Pred
{
public:
  NG_Special_Pred(const PaxTypeFare& paxTypeFare,
                  const FareType& fareType,
                  MinFareFareSelection::EligibleFare eligibleFare)
    : _ptf(paxTypeFare), _fareType(fareType), _eligibleFare(eligibleFare)
  {
  }

  bool operator()(const PaxTypeFare* paxTypeFare)
  {
    if (!paxTypeFare->isValidNoBookingCode())
      return false;

    if (!paxTypeFare->isSpecial())
    {
      return false;
    }

    if (!((_ptf.directionality() == FROM) && (paxTypeFare->directionality() == TO)) &&
        !((_ptf.directionality() == TO) && (paxTypeFare->directionality() == FROM)))
    {
      return false;
    }

    if (_ptf.tcrTariffCat() != RuleConst::PRIVATE_TARIFF &&
        paxTypeFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    {
      return false;
    }

    if (!MinFareFareSelection::isEligibleFare(*paxTypeFare, _ptf, _eligibleFare))
    {
      return false;
    }

    if (_fareType != paxTypeFare->fcaFareType())
    {
      return false;
    }

    if (_ptf.cabin() != paxTypeFare->cabin())
    {
      return false;
    }

    return true;
  }

private:
  const PaxTypeFare& _ptf;
  const FareType& _fareType;
  MinFareFareSelection::EligibleFare _eligibleFare;
};

const PaxTypeFare*
MinFareLogic::getRevNigeriaFare(PricingTrx& trx,
                                const FarePath& farePath,
                                const PricingUnit& pu,
                                const PaxTypeFare* paxTypeFare,
                                const DateTime& travelDate)
{
  if (paxTypeFare == nullptr)
    return nullptr;

  const FareMarket* fareMarket = paxTypeFare->fareMarket();

  if (fareMarket == nullptr)
    return nullptr;

  const PaxTypeBucket* ptc = fareMarket->paxTypeCortege(farePath.paxType());

  if (ptc == nullptr)
    return nullptr;

  DiagManager diag(trx, Diagnostic776);

  // bool displayed = false;
  // if (diag.isactive() && !displayed)
  // {
  //     diag << "nigeria currency adjustment:\n";
  //     diag << paxtypefare->tcrtariffcat();
  //     minimumfare::printfareinfo(*paxtypefare, diag.collector(), ncj, " ");
  //
  //     // do not display a duplicate ng lookup:
  //     if (trx.diagnostic().tostring().find(diag.collector().str()) != std::string::npos)
  //     {
  //         diag.collector().clear();
  //         displayed = true;
  //     }
  // }

  MinFareFareSelection::EligibleFare aEligibleFare = eligibleFare(pu);
  const PaxTypeFare* retPaxTypeFare = nullptr;

  if (paxTypeFare->isSpecial())
  {
    LOG4CXX_DEBUG(logger, "getRevNigeriaFare: special fare");
    FareType fareType = paxTypeFare->fcaFareType();
    while (retPaxTypeFare == nullptr && !fareType.empty())
    {
      std::vector<PaxTypeFare*>::const_iterator iter =
          std::find_if(ptc->paxTypeFare().begin(),
                       ptc->paxTypeFare().end(),
                       NG_Special_Pred(*paxTypeFare, fareType, aEligibleFare));
      if (iter != ptc->paxTypeFare().end())
      {
        retPaxTypeFare = *iter;
        break;
      }

      // try again with next higher fare type.
      fareType = getHigherFareType(trx, *paxTypeFare, fareType, *farePath.itin(), travelDate);
    }
  }

  if (paxTypeFare->isNormal() || retPaxTypeFare == nullptr)
  {
    LOG4CXX_DEBUG(logger, "getRevNigeriaFare: normal fare");
    std::vector<PaxTypeFare*>::const_iterator iter =
        std::find_if(ptc->paxTypeFare().begin(),
                     ptc->paxTypeFare().end(),
                     NG_Normal_Pred(*paxTypeFare, aEligibleFare));
    if (iter != ptc->paxTypeFare().end())
    {
      retPaxTypeFare = *iter;
    }
  }

  // if (diag.isActive() && !displayed)
  // {
  //     if (retPaxTypeFare != 0)
  //     {
  //         diag << retPaxTypeFare->tcrTariffCat();
  //         MinimumFare::printFareInfo(*retPaxTypeFare, diag.collector(), NCJ, " ");
  //     }
  //     else
  //     {
  //         MinimumFare::printExceptionInfo(*paxTypeFare, diag.collector());
  //         diag << "NO FARE FOUND\n";
  //     }
  //     diag << " \n";
  // }

  return retPaxTypeFare;
}

FareType
MinFareLogic::getHigherFareType(PricingTrx& trx,
                                const PaxTypeFare& paxTypeFare,
                                const FareType& fareType,
                                const Itin& itin,
                                const DateTime& travelDate)
{
  bool isProm = paxTypeFare.fareTypeDesignator().isFTDPromotional();

  if (isProm)
    return getHigherPromFareType(trx, paxTypeFare, fareType, itin, travelDate);
  else
    return getHigherSpclFareType(trx, fareType, travelDate);
}

FareType
MinFareLogic::getHigherSpclFareType(PricingTrx& trx,
                                    const FareType& fareType,
                                    const DateTime& travelDate)

{
  FareTypeMatrix* matchFareTypeMatrix = nullptr;

  const std::vector<FareTypeMatrix*>& fareTypeHierachy =
      trx.dataHandle().getAllFareTypeMatrix(travelDate);
  std::vector<FareTypeMatrix*>::const_reverse_iterator i = fareTypeHierachy.rbegin();
  for (; i != fareTypeHierachy.rend(); i++)
  {
    FareTypeMatrix* curFareTypeMatrix = *i;

    if (matchFareTypeMatrix == nullptr)
    {
      if (curFareTypeMatrix->fareType() == fareType)
        matchFareTypeMatrix = curFareTypeMatrix;
    }
    else
    {
      if (curFareTypeMatrix->fareTypeAppl() ==
          matchFareTypeMatrix->fareTypeAppl()) // Both special or normal
      {
        if ((matchFareTypeMatrix->fareTypeDesig().isFTDPromotional()) && // FOR Promotional
            (!curFareTypeMatrix->fareTypeDesig().isFTDPromotional()))
          return "";
        return curFareTypeMatrix->fareType();
      }

      return "";
    }
  }
  return "";
}

FareType
MinFareLogic::getHigherPromFareType(PricingTrx& trx,
                                    const PaxTypeFare& paxTypeFare,
                                    const FareType& fareType,
                                    const Itin& itin,
                                    const DateTime& travelDate)
{
  std::map<uint16_t, const MinFareDefaultLogic*> defLogicMap;
  const MinFareDefaultLogic* defLogic = getDefaultLogic(NCJ, trx, itin, paxTypeFare, defLogicMap);

  std::string spclProcName = "";
  if (defLogic != nullptr)
  {
    spclProcName = defLogic->specialProcessName();
  }

  const MinFareFareTypeGrp* fareTypeGroup =
      trx.dataHandle().getMinFareFareTypeGrp(spclProcName, travelDate);

  if (fareTypeGroup != nullptr)
  {
    const std::vector<MinFareFareTypeGrpSeg*>& fareTypeHierarchy = fareTypeGroup->segs();

    std::vector<MinFareFareTypeGrpSeg*>::const_iterator i = fareTypeHierarchy.begin();
    bool foundCurFareType = false;
    int32_t setNo = -1;
    for (; i != fareTypeHierarchy.end(); i++)
    {
      if ((setNo != -1) && ((**i).setNo() != setNo)) // Temp check before verify setno with Carrie
      {
        // starting diff set, do not continue
        break;
      }

      FareType curType = (**i).fareType();
      if (foundCurFareType)
      {
        if (curType == fareType) // Avoid duplicate record
          continue;
        else
          return curType;
      }
      else
      {
        if (curType == fareType)
        {
          foundCurFareType = true;
          setNo = (**i).setNo();
        }
      }
    }
  }
  return "";
}

bool
MinFareLogic::isDomestic(const Itin& itin,
                         const std::vector<TravelSeg*>& travelSegs,
                         bool checkHiddenStop /*=true*/)
{
  NationCode nation;
  bool isUsCa = false;
  bool isScandinavia = false;

  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); ++i)
  {
    TravelSeg* curTvlSeg = (*i);
    if (UNLIKELY(curTvlSeg == nullptr))
      continue;

    if (nation.empty())
    {
      nation = curTvlSeg->origin()->nation();
      if (LIKELY(itin.geoTravelType() == GeoTravelType::International))
      {
        isUsCa = LocUtil::isDomesticUSCA(*curTvlSeg->origin());
        isScandinavia = LocUtil::isScandinavia(*curTvlSeg->origin());
      }
    }

    if (curTvlSeg->origin()->nation() != nation || curTvlSeg->destination()->nation() != nation)
    {
      if ((isUsCa && LocUtil::isDomesticUSCA(*curTvlSeg->origin()) &&
           LocUtil::isDomesticUSCA(*curTvlSeg->destination())) ||
          (isScandinavia && LocUtil::isScandinavia(*curTvlSeg->origin()) &&
           LocUtil::isScandinavia(*curTvlSeg->destination())))
      {
        // Let it pass
      }
      else
      {
        return false;
      }
    }

    if (LIKELY(checkHiddenStop))
    {
      std::vector<const Loc*>& hiddenStops = curTvlSeg->hiddenStops();
      std::vector<const Loc*>::iterator stopIter = hiddenStops.begin();
      for (; stopIter != hiddenStops.end(); stopIter++)
      {
        const Loc& stopLoc = **stopIter;
        if (stopLoc.nation() != nation)
        {
          if ((isUsCa && LocUtil::isDomesticUSCA(stopLoc)) ||
              (isScandinavia && LocUtil::isScandinavia(stopLoc)))
          {
            // Let it pass
          }
          else
          {
            return false;
          }
        }
      }
    }
  }

  return true;
}
} // tse
