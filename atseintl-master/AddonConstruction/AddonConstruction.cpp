//-------------------------------------------------------------------
//
//  File:        AddonConstruction.cpp
//  Created:     May 14, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Carrier Addon Construction
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
//-------------------------------------------------------------------

#include "AddonConstruction/AddonConstruction.h"

#include "AddonConstruction/ConstructedCacheDataWrapper.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/ConstructionVendor.h"
#include "AddonConstruction/FareDup.h"
#include "AddonConstruction/FareUtil.h"
#include "AddonConstruction/GatewayPair.h"
#include "AddonConstruction/VendorAtpco.h"
#include "AddonConstruction/VendorSita.h"
#include "AddonConstruction/VendorSmf.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DBAccess/AddonFareInfo.h"
#include "Diagnostic/Diag252Collector.h"
#include "Diagnostic/Diag257Collector.h"
#include "Diagnostic/Diag259Collector.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

static Logger
logger("atseintl.AddonConstruction.AddonConstruction");

bool
AddonConstruction::defineVendorCarrierPairs(DataHandle& dataHandle,
                                            const CarrierCode& carrier,
                                            const LocCode& origin,
                                            const LocCode& boardCity,
                                            const LocCode& destination,
                                            const LocCode& offCity,
                                            VCList& vcPairs,
                                            bool disableYY)
{
  // governing carrier, origin airport & city

  defineVendorCarrierPairs(dataHandle, carrier, origin, vcPairs);
  defineVendorCarrierPairs(dataHandle, carrier, boardCity, vcPairs);

  // governing carrier, destination airport & city

  defineVendorCarrierPairs(dataHandle, carrier, destination, vcPairs);
  defineVendorCarrierPairs(dataHandle, carrier, offCity, vcPairs);

  if (LIKELY(!disableYY))
  {
    // industry carrier, origin airport & city

    defineVendorCarrierPairs(dataHandle, INDUSTRY_CARRIER, origin, vcPairs);
    defineVendorCarrierPairs(dataHandle, INDUSTRY_CARRIER, boardCity, vcPairs);

    // industry carrier, destination airport & city

    defineVendorCarrierPairs(dataHandle, INDUSTRY_CARRIER, destination, vcPairs);
    defineVendorCarrierPairs(dataHandle, INDUSTRY_CARRIER, offCity, vcPairs);
  }

  return true;
}

bool
AddonConstruction::defineVendorCarrierPairs(DataHandle& dataHandle,
                                            const CarrierCode& carrier,
                                            const LocCode& interiorMarket,
                                            VCList& vcPairs)
{
  const std::vector<AddonFareInfo*>& addonFares = dataHandle.getAddOnFare(interiorMarket, carrier);

  // loop via retrieved fares and find distinct
  // combinations vendor/carrier

  std::vector<AddonFareInfo*>::const_iterator i = addonFares.begin();
  std::vector<AddonFareInfo*>::const_iterator ie = addonFares.end();

  VCPair vc;
  vc.second = carrier;

  for (; i != ie; ++i)
  {
    const AddonFareInfo& addonFare = **i;

    if (vc.first != addonFare.vendor())
    {
      vc.first = addonFare.vendor();

      if (vcPairs.find(vc) == vcPairs.end())
        vcPairs.insert(vc);
    }
  }

  return true;
}

bool
AddonConstruction::runConstructionProcess(ConstructionJob& cj, ConstructedCacheDataWrapper& dw)
{
  LOG4CXX_INFO(logger,
               "Entered AddonConstruction::process(...) for "
                   << cj.od(CP_ORIGIN) << "-" << cj.carrier() << "-" << cj.od(CP_DESTINATION));

  processConstructionPoint(cj, CP_ORIGIN);

  if (LIKELY(!cj.trx().getOptions()->isRtw()))
    processConstructionPoint(cj, CP_DESTINATION);

  // process specified vendor

  if (cj.constructionVendor() != nullptr)
  {
    ConstructionVendor& vendor = *(cj.constructionVendor());

    // build fares for vendor & populate cache with them

    {
      TSELatencyData metrics(cj.trx(), "AC PROCESS VENDOR");

      vendor.construction(dw);
    }
  }

  return true;
}

bool
AddonConstruction::runReconstructionProcess(ConstructionJob& cj, ConstructedCacheDataWrapper& dw)
{
  Diag259Collector* diag259{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&cj.trx()))
  {
    diag259 = cj.diagnostic<Diag259Collector>();
  }
  else
  {
    diag259 = cj.diag259();
  }

  // build vector of GW pairs that need to be reconstructeed
  CacheGatewayPairVec gwToReconstruct;
  gwToReconstruct.reserve(dw.gateways().size());
  for (const auto& i : dw.gateways())
    if (i->needsReconstruction())
    {
      if (UNLIKELY(diag259 != nullptr))
        diag259->writeGWPairToReconstruct(*i);

      gwToReconstruct.push_back(i);
    }

  if (UNLIKELY(diag259 != nullptr))
    diag259->writeGWPairToReconstructFooter();

  // retrieve add-on fares and validate zones for them
  processConstructionPoint(cj, CP_ORIGIN);

  if (!cj.trx().getOptions()->isRtw())
    processConstructionPoint(cj, CP_DESTINATION);

  // process specified vendor
  if (cj.constructionVendor() != nullptr)
  {
    for (const auto& gateway : gwToReconstruct)
      gateway->initialize(&cj, cj.constructionVendor());

    cj.constructionVendor()->reconstruction(dw, gwToReconstruct);
  }

  // mark gateways processed
  for (const auto& gateway : dw.gateways())
    gateway->needsReconstruction() = false;

  return true;
}

void
AddonConstruction::processConstructionPoint(ConstructionJob& cj, ConstructionPoint cp)
{
  TSELatencyData metrics(cj.trx(), "AC PROCESS CONSTRUCTION POINT");

  LOG4CXX_INFO(logger,
               "Entered AddonConstruction::processConstructionPoint(...) for loc" << cj.od(cp));

  Diag252Collector* diag252{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&cj.trx()))
  {
    diag252 = cj.diagnostic<Diag252Collector>();
  }
  else
  {
    diag252 = cj.diag252();
  }

  if (UNLIKELY(diag252 != nullptr))
    diag252->writeAddonFareDiagHeader(cp);

  // retrieve fares for airport

  unsigned int addonFareCount = processAddonFares(cj, cp, cj.od(cp));

  // retrieve fares for city

  if (cj.od(cp) != cj.odCity(cp))
    addonFareCount += processAddonFares(cj, cp, cj.odCity(cp));

  if (UNLIKELY(diag252 != nullptr))
    diag252->writeAddonFareDiagFooter();

  LOG4CXX_DEBUG(logger, "Num addons found: " << addonFareCount);
}

unsigned int
AddonConstruction::processAddonFares(ConstructionJob& cj,
                                     ConstructionPoint cp,
                                     const LocCode location)
{
  LOG4CXX_INFO(logger, "Entered AddonConstruction::processAddonFares(...) #1");

  // retrieve fares for location

  TSELatencyData metrics(cj.trx(), "AC RETRIEVE ADDON FARES");

  const std::vector<AddonFareInfo*>& addonFares =
      cj.dataHandle().getAddOnFare(location, cj.carrier());

  LOG4CXX_INFO(logger,
               addonFares.size() << " Addon Fares found for " << location << "-" << cj.carrier());

  // looooop via retrieved fares; build vendor object and
  // populate them with add-on fares

  std::vector<AddonFareInfo*>::const_iterator addonFaresIt = addonFares.begin();

  std::vector<AddonFareInfo*>::const_iterator addonFaresEnd = addonFares.end();

  Diag252Collector* diag252{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&cj.trx()))
  {
    diag252 = cj.diagnostic<Diag252Collector>();
  }
  else
  {
    diag252 = cj.diag252();
  }

  AddonZoneStatus zs;
  for (; addonFaresIt != addonFaresEnd; ++addonFaresIt)
  {
    const AddonFareInfo& addonFare = **addonFaresIt;

    if (cj.vendorCode() == addonFare.vendor())
    {
      const LocCode& gateway = addonFare.gatewayMarket();

      if (gateway == addonFare.interiorMarket() || gateway == cj.od(CP_ORIGIN) ||
          gateway == cj.odCity(CP_ORIGIN) || gateway == cj.od(CP_DESTINATION) ||
          gateway == cj.odCity(CP_DESTINATION))
      {
        zs = AZ_UNACCEPTABLE;
      }
      else
      {
        if (cj.constructionVendor() == nullptr)
          getVendor(cj);

        if (LIKELY(!cj.trx().getOptions()->isRtw()))
        {
          zs = cj.constructionVendor()->addAddonFare(cp, addonFare);
        }
        else
        {
          zs = isApplicableForRw(cj, addonFare);

          if (zs == AZ_PASS)
            cj.constructionVendor()->addAddonFareRw(addonFare);
        }
      }

      if (UNLIKELY(diag252 != nullptr))
        diag252->writeAddonFare(addonFare, zs);
    }
  }

  return addonFares.size();
}

AddonZoneStatus
AddonConstruction::isApplicableForRw(ConstructionJob& cj, const AddonFareInfo& addonFare)
{
  static const RoutingNumber RW_ROUTNIG_NUM = "4444";

  if (addonFare.owrt() != ROUND_TRIP_MAYNOT_BE_HALVED)
    return AZ_UNACCEPTABLE;

  if (addonFare.footNote1().find('F') != std::string::npos ||
      addonFare.footNote1().find('T') != std::string::npos ||
      addonFare.footNote2().find('F') != std::string::npos ||
      addonFare.footNote2().find('T') != std::string::npos)
    return AZ_UNACCEPTABLE;

  if (addonFare.routing() != RW_ROUTNIG_NUM)
    return AZ_UNACCEPTABLE;

  if (addonFare.arbZone() != 0)
    return AZ_FAIL;

  return AZ_PASS;
}

void
AddonConstruction::getVendor(ConstructionJob& cj)
{
  if (cj.isAtpco())
    cj.constructionVendor() = VendorAtpco::getNewVendor(cj);

  else if (LIKELY(cj.isSita()))
    cj.constructionVendor() = VendorSita::getNewVendor(cj);

  else if (cj.isSMF())
    cj.constructionVendor() = VendorSmf::getNewVendor(cj);
}

void
AddonConstruction::createResponse(ConstructionJob& cj, CacheConstructedFareInfoVec& cachedFares)
{
  DateTime travelDate(cj.travelDate().date());
  DateTime ticketingDate(cj.ticketingDate().date());

  bool isHistorical = cj.isHistorical();

  Diag257Collector* diag257{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&cj.trx()))
  {
    diag257 = cj.diagnostic<Diag257Collector>();
  }
  else
  {
    diag257 = cj.diag257();
  }

  if (UNLIKELY(diag257 != nullptr))
    diag257->writeDupsRemovalHeader();

  // loop via all cached ConstructedFareInfo's for the vendor

  CacheConstructedFareInfoVec::iterator cacheIt = cachedFares.begin();
  for (; cacheIt != cachedFares.end(); ++cacheIt)
  {
    FareInfo* fi = &((*cacheIt)->fareInfo());

    bool isEffective;
    if (UNLIKELY(isHistorical))
      isEffective = fi->effInterval().isEffective(ticketingDate, travelDate);
    else
      isEffective = fi->effInterval().isEffective(travelDate);

    if (isEffective)
    {
      ConstructedFareInfo* cfi = (*cacheIt)->clone(cj.trx().dataHandle());

      // calculate Constructed Fare Amount

      if (LIKELY(FareUtil::calculateTotalAmount(*cfi, cj)))
        if (LIKELY(FareUtil::nucToFareCurrency(*cfi, cj)))
        {
          // add ConstructedFareData to response
          FareDup::addFIToResponse(cj, cfi);
        }
    }

    else if (UNLIKELY(diag257 != nullptr))
    {
      // to be able display this fare correctly we need
      // to calculate it's amount...

      ConstructedFareInfo* cfi = (*cacheIt)->clone(cj.trx().dataHandle());

      // calculate Constructed Fare Amount

      if (FareUtil::calculateTotalAmount(*cfi, cj))
        FareUtil::nucToFareCurrency(*cfi, cj);

      diag257->writeNotEffective(*cfi);
    }
  }

  // diag. 257 footer

  if (UNLIKELY(diag257 != nullptr))
    diag257->writeDupsRemovalFooter();
}
}
