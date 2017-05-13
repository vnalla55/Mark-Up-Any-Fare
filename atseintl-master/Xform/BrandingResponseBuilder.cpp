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

#include "Xform/BrandingResponseBuilder.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/TseConsts.h"
#include "Common/TNBrands/ItinBranding.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "FareCalc/FareCalcCollector.h"

namespace tse
{
FALLBACK_DECL(fallbackMipBForTNShopping);

void
BrandingResponseBuilder::addBrandForItin(const Itin* itin,
                                         const std::string& programId,
                                         const std::string& brandId)
{
  // Operator [] creates necessary nodes if previously
  // non-existent
  _returnMap[itin][brandId].insert(programId);
}

void
BrandingResponseBuilder::addItin(const Itin* itin)
{
  _returnMap[itin];
}

BrandingResponseType
BrandingRequestUtils::trxToResponse(const BrandingTrx& trx)
{
  BrandingResponseBuilder brb;

  for (const auto itin : trx.itin())
  {
    TSE_ASSERT(itin != nullptr);

    if (!fallback::fallbackMipBForTNShopping(&trx))
      brb.addItin(itin);

    if (trx.getRequest()->isBrandedFaresRequest())
      BrandingResponseUtils::addItinSpecificBrandsToResponse(itin, brb);
    else if (trx.isBrandsForTnShopping() && !fallback::fallbackMipBForTNShopping(&trx))
      BrandingResponseUtils::addAnyBrandToBrandedItins(itin, brb);
  }
  return brb.getBrandingResponse();
}

std::pair<const FarePath*, CalcTotals*>
BrandingResponseUtils::findFarePathAndCalcTotalsForPaxTypeAndBrand(const Itin& itin,
                                                                   const FareCalcCollector& calc,
                                                                   const PaxType* paxType,
                                                                   const size_t brandIndex,
                                                                   const PricingTrx& pricingTrx)
{
  auto path = findFarePathForPaxTypeAndBrand(itin, paxType, brandIndex, pricingTrx);

  CalcTotals* totals = nullptr;

  if (path != nullptr)
  {
    totals = calc.findCalcTotals(path);
  }
  else
  {
    totals = calc.findCalcTotals(paxType, brandIndex);
  }

  return std::make_pair(path, totals);
}

const FarePath*
BrandingResponseUtils::findFarePathForPaxTypeAndBrand(const Itin& itin,
                                                      const PaxType* paxType,
                                                      const size_t brandIndex,
                                                      const PricingTrx& pricingTrx)
{
  for (const FarePath* path : itin.farePath())
  {
    bool brandMatch = false;
    const BrandCode* brandCode = nullptr;
    if (!pricingTrx.isBrandsForTnShopping() && !pricingTrx.validBrands().empty())
      brandCode = &(pricingTrx.validBrands()[brandIndex]);

    if (brandCode)
      brandMatch = path->getBrandCode() == *brandCode;
    else if (pricingTrx.isFlexFare())
      brandMatch = path->getFlexFaresGroupId() == brandIndex;
    else
      brandMatch = path->brandIndex() == brandIndex;

    if (paxType == path->paxType() && brandMatch)
    {
      return path;
    }
  }

  return nullptr;
}

void
BrandingResponseUtils::addItinSpecificBrandsToResponse(const Itin* itin,
                                                       BrandingResponseBuilder& brb)
{
  const BrandCodeSet& itinSpecificBrands = itin->brandCodes();
  const Itin::ProgramsForBrandMap& programs = itin->getProgramsForBrandMap();

  for (const auto& itinSpecificBrand : itinSpecificBrands)
  {
    Itin::ProgramsForBrandMap::const_iterator brandSpecificProgramsIter =
      programs.find(itinSpecificBrand);
    if (brandSpecificProgramsIter != programs.end())
    {
      const Itin::ProgramSet& brandSpecificPrograms = brandSpecificProgramsIter->second;
      for (const auto& brandSpecificProgram : brandSpecificPrograms)
      {
        brb.addBrandForItin(itin, brandSpecificProgram, itinSpecificBrand);
      }
    }
  }
}

void
BrandingResponseUtils::addAnyBrandToBrandedItins(const Itin* itin,
                                                 BrandingResponseBuilder& brb)
{
  if (itin->getItinBranding().hasAnyBrandOnAnyFareMarket())
    brb.addBrandForItin(itin, "", ANY_BRAND);
}


} // namespace tse
