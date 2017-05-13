//----------------------------------------------------------------------------
//  File:        Diag894Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 894 Directionality
//
//  Copyright Sabre 2015
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

#include "Diagnostic/Diag894Collector.h"

#include "Common/BrandingUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "DataModel/FareUsage.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"
//TODO(artur.de.sousa.rocha) Should Xform include be moved to Common?
#include "Xform/BrandingResponseBuilder.h"
#include "Xform/BrandsOptionsFilterForDisplay.h"
#include "Xform/CommonFormattingUtils.h"

#include <sstream>

namespace tse
{

void
Diag894Collector::printHeader()
{
  if (_active)
  {
           //          1         2         3         4         5         6
           // 1234567890123456789012345678901234567890123456789012345678901234
    *this << "******** BRANDED FARES DIRECTIONALITY DIAGNOSTIC 894 ********\n";
    *this << " \n";
    *this << "DIAGNOSTIC SWITCHES:\n";
    const std::string& diagFare = getFare();
    const std::string& diagFareMarket = getFareMarket();
    const std::string& diagBrandCode = getBrandCode();
    *this << "FARE       : " << (diagFare.empty() ? "N/A" : diagFare) << "\n";
    *this << "FARE MARKET: " << (diagFareMarket.empty() ? "N/A" : diagFareMarket) << "\n";
    *this << "BRAND      : " << (diagBrandCode.empty() ? "N/A" : diagBrandCode) << "\n";
    *this << "DDINFO     : " << (isDDINFO() ? "YES" : "NO") << "\n";
    *this << " \n";
  }
}

void
Diag894Collector::printFooter()
{
  if (_active)
  {
             //          1         2         3         4         5         6
             // 1234567890123456789012345678901234567890123456789012345678901234
    *this << " \n********************* END DIAGNOSTIC 894 ********************\n";
  }
}

void
Diag894Collector::printFareValidationStatus(PaxTypeFare::BrandStatus status)
{
  if (!_active)
    return;
  DiagCollector& dc = asDiagCollector();
  dc << "FARE VALIDATED AS: ";
  switch (status)
  {
    case PaxTypeFare::BS_HARD_PASS: dc << "HARD PASS"; break;
    case PaxTypeFare::BS_SOFT_PASS: dc << "SOFT PASS"; break;
    default:
      dc << "FAIL";
  }
  dc << "\n";
}

void
Diag894Collector::printFareValidationStatusHotHardPass()
{
  if (!_active)
    return;
  DiagCollector& dc = asDiagCollector();
  dc << "FARE VALIDATED AS: ALREADY GOT HARD PASS IN THIS DIRECTION\n";
}

void
Diag894Collector::printProgramToFareMarketDirectionMatch(bool match)
{
  if (!_active)
    return;
  DiagCollector& dc = asDiagCollector();
  dc << "PROGRAM TO FM DIRECTIONALITY: " << (match ? "MATCH" : "NO MATCH") << "\n";
}

void
Diag894Collector::printProgramDirectionCalculationDetails(
  const BrandProgram& brandProgram, const FareMarket& fareMarket, bool isOriginal,
  bool isReversed, Direction direction)
{
  if (!_active)
    return;
  DiagCollector& dc = asDiagCollector();

  dc << " \nPROGRAM DIRECTIONALITY CALCULATION DETAILS:\n";
  dc << "PROGRAM ID          : " << brandProgram.programID() << "\n";
  dc << "PROGRAM ORIGINS     :"
     << DiagnosticUtil::containerToString(brandProgram.originsRequested(), " ", DiagnosticUtil::MAX_WIDTH, 21, false);
  dc << "\nPROGRAM DESTINATIONS:"
     << DiagnosticUtil::containerToString(brandProgram.destinationsRequested(), " ", DiagnosticUtil::MAX_WIDTH, 21, false);
  dc << "\nFM ORIGINS          :"
     << DiagnosticUtil::containerToString(fareMarket.getOriginsRequested(), " ", DiagnosticUtil::MAX_WIDTH, 21, false);
  dc << "\nFM DESTINATIONS     :"
     << DiagnosticUtil::containerToString(fareMarket.getDestinationsRequested(), " ", DiagnosticUtil::MAX_WIDTH, 21, false);
  dc << "\nDIRECTION ORIGINAL  : " << (isOriginal ? "YES" : "NO") << "\n";
  dc << "DIRECTION REVERSED  : " << (isReversed ? "YES" : "NO") << "\n";
  dc << "DIRECTIONALITY      : " << direction << "\n";
}

void
Diag894Collector::printQualifiedBrandInfo(int index, const QualifiedBrand& qualifiedBrand)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();

  dc << " \nPROGRAM/BRAND INFO:\n";
  dc << "BRAND INDEX " << index << ":\n";
  BrandedDiagnosticUtil::displayBrand(dc, qualifiedBrand);
  BrandedDiagnosticUtil::displayBrandProgram(dc, qualifiedBrand, true);
}

void
Diag894Collector::printPaxTypeFarInfo(const PaxTypeFare& paxTypeFare)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();

  dc << " \nPAX TYPE FARE INFORMATION:\n";
  dc << "FARE " << paxTypeFare.fareClass() << " TYPE "
     << (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED ? "X" :
        (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "R" :
        (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED ? "O" : "?")))
     << "\n";
  dc << "FARE MARKET " << paxTypeFare.fareMarket()->origin()->loc() << " "
     << (paxTypeFare.directionality() == FROM ? "-" : "+") << "--"
     << (paxTypeFare.directionality() == TO ? "-" : "+") << " "
     << paxTypeFare.fareMarket()->destination()->loc() << " ("
     << (paxTypeFare.directionality() == FROM ? "O" :
     (paxTypeFare.directionality() == TO ? "I" : "B"))
     << ")\n";
}

void
Diag894Collector::printImproper894Use()
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();
  dc << "INVALID USE OF DIAGNOSTIC 894" << std::endl;
  dc << "VALID ONLY FOR PRICING/SHOPPING BRANDED REQUESTS" << std::endl;
  dc.flushMsg();
}

namespace
{

std::vector<std::string> prepareBrandTagsShoppingTN(const PricingTrx& trx,
                                                    const Itin& itin,
                                                    const FarePath& fp,
                                                    const FareUsage& fu)
{
  std::vector<std::string> result;

  BrandCode brandCodeFromOuterSpace = NO_BRAND;
  if (trx.isBRAll())
  {
    const uint16_t brandIndex = fp.brandIndex();
    const uint16_t segmentIndex = itin.segmentOrder(fu.travelSeg().front()) - 1;
    const CarrierCode& carrierCode = fu.paxTypeFare()->fareMarket()->governingCarrier();
    const skipper::ItinBranding& itinBranding = itin.getItinBranding();
    Direction direction = Direction::BOTHWAYS;
    const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);
    if (useDirectionality)
      direction = fu.paxTypeFare()->getDirection();
    brandCodeFromOuterSpace =
      itinBranding.getBrandCode(brandIndex, segmentIndex, carrierCode, direction);
  }
  const BrandCode fareBrandCode = ShoppingUtil::isThisBrandReal(brandCodeFromOuterSpace) ?
    brandCodeFromOuterSpace : fu.paxTypeFare()->getFirstValidBrand(trx, fu.getFareUsageDirection());
  if (fareBrandCode.empty())
    return result;
  const unsigned int index = fu.paxTypeFare()->getValidBrandIndex(trx, &fareBrandCode,
                                                                  fu.getFareUsageDirection());
  if (index == INVALID_BRAND_INDEX)
    return result;
  const QualifiedBrand& qb = trx.brandProgramVec()[index];
  xform::formatBrandProgramData(result, qb, fareBrandCode);

  return result;
}

std::vector<std::string> prepareBrandTagsShoppingAS(const PricingTrx& trx,
                                                    const Itin& itin,
                                                    const FarePath& fp,
                                                    const FareUsage& fu)
{
  std::vector<std::string> result;

  // GRI level brand code
  std::string brandCode = ShoppingUtil::getFarePathBrandCode(&fp);
  if (trx.isContextShopping())
  {
    // Display brand from fare component (as it may be different than on
    // GRI level if on fixed leg)
    // Additionally if all legs are fixed then fare path's brand may be NO_BRAND
    if (skipper::TNBrandsFunctions::isAnySegmentOnFixedLeg(
                fu.paxTypeFare()->fareMarket()->travelSeg(), trx.getFixedLegs()))
      brandCode = fu.paxTypeFare()->getFirstValidBrand(trx, fu.getFareUsageDirection());
  }
  if (brandCode == ANY_BRAND_LEG_PARITY)
    brandCode = fu.getBrandCode();
  // CBS is excluded in ANY_BRAND_LEG_PARITY and in Catch All Bucket
  else if (trx.getRequest()->isChangeSoldoutBrand() && !brandCode.empty())
  {
    if (!trx.isContextShopping() ||
        !skipper::TNBrandsFunctions::isAnySegmentOnCurrentlyShoppedLeg(
               fu.paxTypeFare()->fareMarket()->travelSeg(), trx.getFixedLegs()))
    {
      //getStatusForBrand(brandCode, Direction::BOTHWAYS) should be used for all branded
      //requests except BRALL
      IbfErrorMessage ibfErrorMessage = IbfAvailabilityTools::translateForOutput(
          fu.paxTypeFare()->fareMarket()->getStatusForBrand(brandCode, Direction::BOTHWAYS));

      if (ibfErrorMessage != IbfErrorMessage::IBF_EM_NO_FARE_FOUND)
        brandCode = fu.paxTypeFare()->getFirstValidBrand(trx, fu.getFareUsageDirection());
    }
  }
  if (!brandCode.empty())
  {
    if (trx.isNotExchangeTrx() ||
        !fu.paxTypeFare()->isFromFlownOrNotShoppedFM())
    {
      const unsigned int index =
        fu.paxTypeFare()->getValidBrandIndex(trx, &brandCode, fu.getFareUsageDirection());
      if (index != INVALID_BRAND_INDEX)
      {
        const QualifiedBrand& qb = trx.brandProgramVec().at(index);
        xform::formatBrandProgramData(result, qb, brandCode);
      }
    }
  }

  return result;
}

std::vector<std::string> prepareBrandTagsShopping(const PricingTrx& trx,
                                                  const Itin& itin,
                                                  const FarePath& fp,
                                                  const FareUsage& fu)
{
  if (trx.getRequest()->isBrandedFaresRequest())
    return prepareBrandTagsShoppingAS(trx, itin, fp, fu);
  else if (trx.isBrandsForTnShopping())
    return prepareBrandTagsShoppingTN(trx, itin, fp, fu);
  else
    return std::vector<std::string>();
}

std::vector<std::string> prepareBrandTagsExpedia(const PricingTrx& trx,
                                                 const Itin& itin,
                                                 const FarePath& fp,
                                                 const FareUsage& fu)
{
  std::vector<std::string> result;

  // in multiple brand mode we need to know brand context we're in
  // in case a fare is valid for more than one brand we have to display one actually used.
  const uint16_t brandIndex = fp.brandIndex();
  if (brandIndex == INVALID_BRAND_INDEX)
    return result;

  const uint16_t segmentIndex = itin.segmentOrder(fu.travelSeg().front()) - 1;
  const CarrierCode& carrierCode = fu.paxTypeFare()->fareMarket()->governingCarrier();

  const skipper::ItinBranding& itinBranding = itin.getItinBranding();
  Direction direction = Direction::BOTHWAYS;
  if (BrandingUtil::isDirectionalityToBeUsed(trx))
    direction = fu.paxTypeFare()->getDirection();
  BrandCode brandCode = itinBranding.getBrandCode(brandIndex, segmentIndex, carrierCode, direction);

  // In NO_BRAND ( the cheapest scenario ) we still want to display brands if they are defined for a
  // given fare
  const BrandCode fareBrandCode =
    (brandCode != NO_BRAND) ? brandCode :
                              fu.paxTypeFare()->getFirstValidBrand(trx, fu.getFareUsageDirection());

  if (fareBrandCode.empty())
    return result;

  const unsigned int index = fu.paxTypeFare()->getValidBrandIndex(trx, &fareBrandCode,
                                                                  fu.getFareUsageDirection());
  if (index == INVALID_BRAND_INDEX)
    return result;

  const QualifiedBrand& qb = trx.brandProgramVec().at(index);
  xform::formatBrandProgramData(result, qb, fareBrandCode);

  return result;
}

std::vector<std::string> prepareBrandTagsPbb(const PricingTrx& trx,
                                             const Itin& itin,
                                             const FarePath& fp,
                                             const FareUsage& fu)
{
  std::vector<std::string> result;

  std::vector<BrandCode> brands;
  fu.paxTypeFare()->getValidBrands(trx, brands);
  if (!brands.empty())
  {
    BrandCode bc = brands.front();
    const size_t index = fu.paxTypeFare()->getValidBrandIndex(trx, &bc, fu.getFareUsageDirection());
    if (index < trx.brandProgramVec().size())
    {
      const QualifiedBrand& qb = trx.brandProgramVec()[index];
      xform::formatBrandProgramData(result, qb, bc);
    }
  }
  return result;
}

std::vector<std::string> prepareBrandTagsPricing(const PricingTrx& trx,
                                                 const Itin& itin,
                                                 const FarePath& fp,
                                                 const FareUsage& fu)
{
  if (trx.activationFlags().isSearchForBrandsPricing())
    return prepareBrandTagsExpedia(trx, itin, fp, fu);
  else if (trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS)
    return prepareBrandTagsPbb(trx, itin, fp, fu);
  else
    return std::vector<std::string>();
}

std::vector<std::string> prepareBrandTags(const PricingTrx& trx,
                                          const Itin& itin,
                                          const FarePath& fp,
                                          const FareUsage& fu)
{
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
    return prepareBrandTagsShopping(trx, itin, fp, fu);
  else if (trx.getTrxType() == PricingTrx::PRICING_TRX)
    return prepareBrandTagsPricing(trx, itin, fp, fu);
  else
    return std::vector<std::string>();
}

std::vector<std::pair<int, BrandCode>> calculateMipGriToPrint(const PricingTrx& trx, Itin& itin)
{
  // calculates indices of brands/spaces to print on GRI level
  // mimics XMLShoppingResponse::generateITNbody() behavior
  std::vector<std::pair<int, BrandCode>> brandIndexVector;
  const bool isIbf = !trx.validBrands().empty() && !trx.isBrandsForTnShopping();
  uint16_t brandSize = 0;
  if (trx.getRequest()->brandedFareEntry())
    brandSize = trx.getRequest()->getBrandedFareSize();
  else if (isIbf)
    brandSize = trx.validBrands().size();

  if ((brandSize == 1) && !isIbf)
    brandIndexVector.push_back(std::make_pair(0, NO_BRAND));
  else if (((brandSize > 1) || isIbf))
  {
    // combines XMLShoppingResponse::itinIsValidBrandSolution() and
    // XMLShoppingResponse::generateITNbody() (brandSize > 1) || isIbf case
    if (trx.validBrands().empty())
      return brandIndexVector; //nothing can be valid
    for (uint16_t brandIndex = 0; brandIndex < brandSize; ++brandIndex)
    {
      const BrandCode& brandCode = trx.validBrands()[brandIndex];
      for (const auto farePath : itin.farePath())
      {
        if ((farePath->getBrandCode() == brandCode) ||
            (farePath->brandIndex() == brandIndex))
        {
          BrandCode brand = NO_BRAND;
          if (trx.getRequest()->brandedFareEntry())
            brand = trx.getRequest()->brandId(brandIndex);
          else
          {
            std::string brandCode = ShoppingUtil::getBrandCodeString(trx, brandIndex);
            if (!brandCode.empty())
              brand = brandCode;
            else
              brand = ShoppingUtil::getFakeBrandString(trx, brandIndex);
          }
          brandIndexVector.push_back(std::make_pair(brandIndex, brand));
        } //else excluded, not interested in IBF errors in diag
      }
    }
  }
  else if (trx.isBRAll())
  {
    BrandsOptionsFilterForDisplay brandOptionFilter(trx);
    BrandsOptionsFilterForDisplay::BrandingSpacesWithSoldoutVector filteredOptionsVector =
      brandOptionFilter.collectOptionsForDisplay(itin);

    for(const BrandsOptionsFilterForDisplay
                ::BrandingOptionWithSoldout& option: filteredOptionsVector)
    {
      if (!option.second) // not interested in soldouts in diag
        brandIndexVector.push_back(std::make_pair(option.first, ""));
    }
  }
  else
    brandIndexVector.push_back(std::make_pair(INVALID_BRAND_INDEX, NO_BRAND));

  return brandIndexVector;
}

std::vector<const FareUsage*> calculateOrderedFareUsages(const Itin& itin, const FarePath& farePath)
{
  std::vector<const FareUsage*> tmp(itin.travelSeg().size(), nullptr);
  std::vector<const FareUsage*> result;
  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      for (const TravelSeg* seg : fu->travelSeg())
      {
        if (seg->isAir())
        {
          const int16_t segmentIndex = itin.segmentOrder(seg) - 1;
          TSE_ASSERT(segmentIndex >= 0 && size_t(segmentIndex) < tmp.size());
          tmp[segmentIndex] = fu;
        }
      }
    }
  }

  // remove duplicates (same fare usage on following segments)
  const FareUsage* prvFareUsage = nullptr;
  for (const FareUsage* fu: tmp)
  {
    if (fu != prvFareUsage)
    {
      if (fu != nullptr)
        result.push_back(fu);
      prvFareUsage = fu;
    }
  }

  return result;
}

void printFareUsageInfo(DiagCollector& dc,
                        const PaxType& paxType,
                        const FarePath& farePath,
                        const Itin& itin,
                        PricingTrx& trx)
{
  dc << "* PAX " << paxType.paxType() << " *" << std::endl;
  std::vector<const FareUsage*> fareUsages = calculateOrderedFareUsages(itin, farePath);
  for (const FareUsage* fareUsage : fareUsages)
  {
    const PaxTypeFare* ptf = fareUsage->paxTypeFare();
    dc << "A01=" << fareUsage->travelSeg().front()->origAirport() << ",";
    dc << "A02=" << fareUsage->travelSeg().back()->destAirport() << ",";
    dc << "A11=" << fareUsage->travelSeg().front()->boardMultiCity() << ",";
    dc << "A12=" << fareUsage->travelSeg().back()->offMultiCity() << ",";
    dc << "B00=" << ptf->fareMarket()->governingCarrier() << ",";
    dc << "B50=" << ptf->createFareBasis(trx) << ",";
    dc << "S70=" <<
      ((fareUsage->isInbound() && !fareUsage->dirChangeFromOutbound()) ? "TO" : "FR") << ",";
    dc << "PTF894=" << ptf->origin() << "-" << ptf->carrier() << "-"
      << ptf->destination() << "-" << directionToString(ptf->getDirection()) << ",";
    dc << "FI894=" << ptf->fare()->fareInfo()->market1() << "-"
      << ptf->fare()->fareInfo()->market2() << "-"
      << (ptf->fare()->fareInfo()->directionality() == FROM ? "FROM" :
          (ptf->fare()->fareInfo()->directionality() == TO ? "TO" : "BOTH"));
    std::vector<std::string> brandInfo = prepareBrandTags(trx, itin, farePath, *fareUsage);
    for (std::string& tagValue: brandInfo)
      dc << "," << tagValue;
    dc << std::endl;
  }
  dc << "* END PAX " << paxType.paxType() << std::endl;
}

void printMipItinFareUsageInfo(DiagCollector& dc, const Itin& itin, PricingTrx& trx,
                               const std::vector<std::pair<int, BrandCode>>& brandIndexVector)
{
  for (auto& brandIndex: brandIndexVector)
  {
    if (brandIndexVector.size() > 1)
      dc << "* GRI " << brandIndex.second << (brandIndex.second.empty() ? "*" : " *") << std::endl;
    for (const PaxType* paxType : trx.paxType())
    {
      const FarePath* farePath =
        BrandingResponseUtils::findFarePathForPaxTypeAndBrand(itin, paxType, brandIndex.first, trx);
      if (farePath == nullptr)
        continue;
      printFareUsageInfo(dc, *paxType, *farePath, itin, trx);
    }
    if (brandIndexVector.size() > 1)
      dc << "* END GRI " << brandIndex.second << (brandIndex.second.empty() ? "*" : " *") << std::endl;
  }
}

void printPricingItinFareUsageInfoPBB(DiagCollector& dc, Itin& itin, PricingTrx& trx)
{
  for (PaxType* paxType : trx.paxType())
  {
    auto farePathIt = std::find_if(itin.farePath().begin(), itin.farePath().end(),
      [&paxType](FarePath* fp) { return fp->paxType() == paxType; });
    if (farePathIt == itin.farePath().end())
      continue;
    printFareUsageInfo(dc, *paxType, *(*farePathIt), itin, trx);
  }
}

void printPricingItinFareUsageInfoExpedia(DiagCollector& dc, Itin& itin, PricingTrx& trx)
{
  BrandsOptionsFilterForDisplay brandOptionFilter(trx);
  BrandsOptionsFilterForDisplay::BrandingSpacesWithSoldoutVector filteredOptionsVector =
    brandOptionFilter.collectOptionsForDisplay(itin);
  for (PaxType* paxType : trx.paxType())
  {
    for (const auto& option : filteredOptionsVector)
    {
      if (option.second) // ignore soldouts
        continue;
      const FarePath* farePath =
        BrandingResponseUtils::findFarePathForPaxTypeAndBrand(itin, paxType, option.first, trx);
      if (farePath == nullptr)
        continue;
      printFareUsageInfo(dc, *paxType, *farePath, itin, trx);
    }
  }
}

void printPricingItinFareUsageInfo(DiagCollector& dc, Itin& itin, PricingTrx& trx)
{
  if (trx.activationFlags().isSearchForBrandsPricing())
    printPricingItinFareUsageInfoExpedia(dc, itin, trx);
  else if (trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS)
    printPricingItinFareUsageInfoPBB(dc, itin, trx);
}

} // namespace

void
Diag894Collector::printItinFareUsageInfo(Itin& itin, PricingTrx& trx)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();
  std::vector<std::pair<int, BrandCode>> brandIndexVector;
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    brandIndexVector = calculateMipGriToPrint(trx, itin);
    printMipItinFareUsageInfo(dc, itin, trx, brandIndexVector);
  }
  else if (trx.getTrxType() == PricingTrx::PRICING_TRX)
    printPricingItinFareUsageInfo(dc, itin, trx);
  else
    return;
}

bool
Diag894Collector::isValidForPaxTypeFare(const PricingTrx& trx, const PaxTypeFare& paxTypeFare) const
{
  const std::string& diagFare = getFare();
  const std::string& diagFareMarket = getFareMarket();
  const std::string& diagBrandCode = getBrandCode();
  std::string fm = paxTypeFare.fareMarket()->origin()->loc();
  fm.append(paxTypeFare.fareMarket()->destination()->loc());

  //   FB  |   FM  |   BC  |
  // ------+-------+-------+--------------
  // empty | empty | empty | = no diag
  //  ALL  |  ALL  |  ALL  | = all
  // fare  |  ALL  |  ALL  | = only fare limitation
  // fare  |  fm   |  ALL  | = only fare on fm
  // fare  |  ALL  | brand | = only fare with brand
  // fare  |  fm   | brand | = only fare on fm with brand
  //  ALL  |  fm   |  ALL  | = only fm limitation
  //  ALL  |  fm   | brand | = only fm with brand
  //  ALL  |  ALL  | brand | = only brand
  bool validDiagSwitch = true;
  if (validDiagSwitch && diagFare.empty() && diagFareMarket.empty() && diagBrandCode.empty())
    validDiagSwitch = false;
  if (validDiagSwitch && (diagFare != "ALL") && (diagFare != paxTypeFare.fareClass()))
    validDiagSwitch = false;
  if (validDiagSwitch && (diagFareMarket != "ALL") && (diagFareMarket != fm))
    validDiagSwitch = false;
  if (validDiagSwitch && (diagBrandCode != "ALL"))
  { // check if brand code is valid
    validDiagSwitch = false;
    for (int index: paxTypeFare.fareMarket()->brandProgramIndexVec())
    {
      if (trx.brandProgramVec()[index].second->brandCode() == diagBrandCode)
      {
        validDiagSwitch = true;
        break;
      }
    }
  }
  return validDiagSwitch;
}

} // namespace tse
