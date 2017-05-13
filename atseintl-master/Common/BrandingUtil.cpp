//-------------------------------------------------------------------
//
//  File:        BrandingUtil.cpp
//  Created:     April 2013
//  Authors:
//
//  Description:
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

#include "Common/BrandingUtil.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TrxUtil.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/BrandedCarrier.h"
#include "DBAccess/CarrierPreference.h"
#include "FareCalc/CalcTotals.h"

namespace tse
{
FALLBACK_DECL(fallbackBrandDirectionality)
FALLBACK_DECL(fallbackXmlBrandInfoFaredisplay)
FALLBACK_DECL(fallbackHalfRTPricingForIbf);
FALLBACK_DECL(fallbackProperDirectionalityValidationInFQ)
FALLBACK_DECL(fallbackDisableCrossBrandInOneWay)

namespace
{
ConfigurableValue<bool>
brandedCarrier("S8_BRAND_SVC", "BRANDED_CARRIER", false);

Logger
logger("atseintl.BrandedFares.BrandingUtil");
}

bool
BrandingUtil::isDirectionalityToBeUsed(const PricingTrx& trx)
{
  if (_UNLIKELY(fallback::fallbackBrandDirectionality(&trx)))
    return false;

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
    return true;

  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    if ((trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS) ||
        (trx.activationFlags().isSearchForBrandsPricing()))
    return true;
  }

  if (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
  {
    if (!fallback::fallbackProperDirectionalityValidationInFQ(&trx))
      return true;
  }

  return false;
}

//------------------------------------------------------
// BrandingUtil::isBrandGrouping()
//------------------------------------------------------
bool
BrandingUtil::isBrandGrouping(FareDisplayTrx& trx)
{
  if (!FareDisplayUtil::isBrandGroupingEnabled() || trx.isShopperRequest() ||
      trx.dataHandle().isHistEnabled(trx.getRequest()->ticketingDT()))
  {
    return false;
  }

  // Check if this carrier is branded

  if (!brandedCarrier.getValue())
    return false; // No S8 Brand Carrier activated - do not call S8 service

  const std::vector<BrandedCarrier*>& brandedCarrierVec = trx.dataHandle().getS8BrandedCarriers();

  if (brandedCarrierVec.empty())
  {
    return false;
  }

  std::vector<BrandedCarrier*>::const_iterator i = brandedCarrierVec.begin();
  std::vector<BrandedCarrier*>::const_iterator e = brandedCarrierVec.end();

  CarrierCode carrier = trx.requestedCarrier();

  for (; i != e; i++)
  {
    if ((*i)->carrier() == carrier)
    {
      return true;
    }
  }
  trx.bfErrorCode() = PricingTrx::CARRIER_NOT_ACTIVE;
  return false;
}

bool
BrandingUtil::isPopulateFareDataMapVec(FareDisplayTrx& trx)
{
  if (!fallback::fallbackXmlBrandInfoFaredisplay(&trx))
  {
    if (!(TrxUtil::pssPoAtsePath(trx) || TrxUtil::libPoAtsePath(trx)))
      return true;
  }

  return false;
}

bool
BrandingUtil::isNoBrandsOffered(const PricingTrx& trx, const CalcTotals& calcTotals)
{
  TSE_ASSERT(nullptr != calcTotals.farePath);
  TSE_ASSERT(nullptr != calcTotals.farePath->itin());
  const Itin& itin = *(calcTotals.farePath->itin());
  const skipper::ItinBranding& itinBranding = itin.getItinBranding();
  const uint16_t brandIndex = calcTotals.farePath->brandIndex();

  if (INVALID_BRAND_INDEX == brandIndex)
    return true;

  const bool useDirectionality = isDirectionalityToBeUsed(trx);

  for (auto& pricingUnit : calcTotals.farePath->pricingUnit())
  {
    for (auto& fareUsage : pricingUnit->fareUsage())
    {
      TSE_ASSERT(!fareUsage->travelSeg().empty());
      const uint16_t segmentIndex = itin.segmentOrder(fareUsage->travelSeg().front()) - 1;
      TSE_ASSERT(nullptr != fareUsage->paxTypeFare());
      const PaxTypeFare& paxTypeFare = *(fareUsage->paxTypeFare());
      const CarrierCode& carrierCode = paxTypeFare.fareMarket()->governingCarrier();
      Direction direction = Direction::BOTHWAYS;
      if (useDirectionality)
        direction = paxTypeFare.getDirection();
      BrandCode brandCode;
      brandCode = itinBranding.getBrandCode(
        brandIndex, segmentIndex, carrierCode, direction, fareUsage->getFareUsageDirection());
      const BrandCode fareBrandCode =
          (brandCode != NO_BRAND) ?
            brandCode : paxTypeFare.getFirstValidBrand(trx, fareUsage->getFareUsageDirection());
      if (!fareBrandCode.empty())
        return false;
    }
  }
  return true;
}

BrandRetrievalMode
BrandingUtil::getBrandRetrievalMode(const PricingTrx& trx)
{
  // check if the entry is coming from an airline partition
  if (TrxUtil::isRequestFromAS(trx))
  {
    const CarrierPreference* cp = trx.dataHandle().getCarrierPreference(
        trx.billing()->partitionID(), trx.getRequest()->ticketingDT());
    if (cp != nullptr && cp->getApplyBrandedFaresPerFc() == YES)
      return BrandRetrievalMode::PER_FARE_COMPONENT;
    else
      return BrandRetrievalMode::PER_O_AND_D;
  }
  return BrandRetrievalMode::PER_FARE_COMPONENT;
}

bool
BrandingUtil::isCrossBrandOptionNeeded(const PricingTrx& trx)
{
  if (!fallback::fallbackDisableCrossBrandInOneWay(&trx))
  {
    if (trx.getRequest()->isParityBrandsPath())
    {
      if (trx.itin().front()->itinLegs().size() == 1 ||
          trx.getRequest()->originBasedRTPricing())
        return false;
      if (trx.isContextShopping() &&
          std::all_of(trx.getFixedLegs().begin(), trx.getFixedLegs().end(),
                      [](bool fixed){ return fixed; }))
        return false;
    }
  }
  return trx.getRequest()->isCheapestWithLegParityPath();
}

namespace
{
bool
fmMatchesDiagRequest(const PricingTrx& trx, const FareMarket* fm)
{
  const std::string& requestedFm = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_MARKET);
  if (requestedFm.empty())
    return true; // if no specific fm requested then match all

  std::string fmOD = fm->origin()->loc();
  fmOD.append(fm->destination()->loc());

  if (requestedFm == fmOD)
    return true;

  return false;
}

bool
brandMatchesDiagRequest(const PricingTrx& trx, unsigned int brandIndex)
{
  const std::string& requestedBrand = trx.diagnostic().diagParamMapItem(Diagnostic::BRAND_ID);
  if (requestedBrand.empty())
    return true; // if no specific brand requested then match all

  TSE_ASSERT(brandIndex < trx.brandProgramVec().size());
  const BrandCode& brandCode = trx.brandProgramVec()[brandIndex].second->brandCode();

  if (requestedBrand.compare(brandCode) == 0)
    return true;

  return false;
}

void
removeSoftPassesIfHardPassExists(PricingTrx& trx,
                                 FareMarket* fm,
                                 unsigned int brandIndex,
                                 DiagCollector* diag)
{
  TSE_ASSERT(fm != nullptr);

  if (fm->fareCompInfo() != nullptr)
      return;

  if (UNLIKELY(fm->useDummyFare() && !fallback::fallbackHalfRTPricingForIbf(&trx)))
    return;

  const BrandCode& brandCode =
      ShoppingUtil::getBrandCode(trx, fm->brandProgramIndexVec()[brandIndex]);

  const bool printAllFares =
      diag && trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "HARDSOFTPASSALL";

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);
  std::set<Direction> hardPassedDirections;

  if (ShoppingUtil::validHardPassExists(
          fm, brandIndex, hardPassedDirections, diag, useDirectionality, printAllFares))
  {
    skipper::BrandCodeDirection brandInDirection =
        skipper::BrandCodeDirection(brandCode, Direction::BOTHWAYS);

    if (!useDirectionality || ShoppingUtil::hardPassForBothDirectionsFound(hardPassedDirections))
    {
      ShoppingUtil::removeSoftPassesFromFareMarket(fm, brandIndex, diag);
    }
    else
    {
      TSE_ASSERT(hardPassedDirections.size() == 1);
      const Direction hardPassedDirection = *(hardPassedDirections.begin());
      brandInDirection.direction = hardPassedDirection;
      ShoppingUtil::removeSoftPassesFromFareMarket(fm, brandIndex, diag, hardPassedDirection);
    }
    // once we remove soft passed fares we can only use hard passed fares' availability
    // when computing availability statuses for soldout itineraries
    fm->setUseHardPassBrandAvailabilityOnly(brandInDirection);
  }
  else if (UNLIKELY(diag))
  {
    *diag << "VALID HARD PASS NOT FOUND, USING SOFT PASSES\n\n ";
  }
}
}

bool
BrandingUtil::diag892InHardSoftPassMode(const PricingTrx& trx)
{
  if (LIKELY(trx.diagnostic().diagnosticType() != Diagnostic892))
    return false;

  const std::string& dd = trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
  if (dd == "HARDSOFTPASS" || dd == "HARDSOFTPASSALL")
    return true;

  return false;
}

void
BrandingUtil::processSoftPassFares(PricingTrx& trx)
{
  DiagCollector* diag = nullptr;

  if (UNLIKELY(diag892InHardSoftPassMode(trx)))
  {
    DCFactory* factory = DCFactory::instance();
    diag = factory->create(trx);
    diag->enable(Diagnostic892);
    *diag << "REMOVING SOFT PASSES ON MARKETS THAT HAVE VALID HARD PASS\n"
             "SKIPPING FARES THAT ARE INVALID OR NOT OFFERED ON ALL SEGS\n"
             "FARES DISPLAYED WITH AVAILABILITY STATUS:\n"
             " O-NOT OFFERED, A-NOT AVAILABLE, N-NOT SET, F-AVAILABLE\n"
             "AND BRAND STATUS (H-ARD/S-OFT/F-AIL)\n\n";
  }

  for (FareMarket* fm : trx.fareMarket())
  {
    bool displayDiagForThisFm = diag && fmMatchesDiagRequest(trx, fm);
    if (UNLIKELY(displayDiagForThisFm))
    {
      *diag << "FARE MARKET: " << fm->origin()->loc() << "-" << fm->governingCarrier() << "-"
            << fm->destination()->loc() << "\n\n";
    }

    for (unsigned int brandIndex = 0; brandIndex < fm->brandProgramIndexVec().size(); ++brandIndex)
    {
      const bool displayDiagForThisBrand =
          displayDiagForThisFm &&
          brandMatchesDiagRequest(trx, fm->brandProgramIndexVec()[brandIndex]);

      if (UNLIKELY(displayDiagForThisBrand))
      {
        const QualifiedBrand& qualifiedBrand =
            trx.brandProgramVec()[fm->brandProgramIndexVec()[brandIndex]];

        *diag << "BRAND: " << qualifiedBrand.second->brandCode()
              << " PROGRAM ID: " << qualifiedBrand.first->programID() << std::endl;
      }

      DiagCollector* diagForThisFmAndBrand = (displayDiagForThisBrand ? diag : nullptr);
      removeSoftPassesIfHardPassExists(trx, fm, brandIndex, diagForThisFmAndBrand);
    }
  }

  if (UNLIKELY(diag))
  {
    diag->flushMsg();
  }
}
}
