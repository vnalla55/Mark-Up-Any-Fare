//----------------------------------------------------------------------------
//  File:        Diag892Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 892 Branded Fares - Display brand information
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
//----------------------------------------------------------------------------

#include "Diagnostic/Diag892Collector.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/CabinType.h"
#include "Common/ShoppingUtil.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "Common/TseEnums.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"

#include <sstream>
#include <algorithm>

#include <math.h>

namespace tse
{

void
Diag892Collector::printHeader()
{
  if (_active)
  {
           //          1         2         3         4         5         6
           // 1234567890123456789012345678901234567890123456789012345678901234
    *this << "**************** BRANDED FARES DIAGNOSTIC 892 ***************\n";
  }
}

void
Diag892Collector::printFooter()
{
  if (_active)
  {
             //          1         2         3         4         5         6
             // 1234567890123456789012345678901234567890123456789012345678901234
    *this << "\n********************* END DIAGNOSTIC 892 ********************\n";
  }
}

void
Diag892Collector::printBrands(const std::set<BrandCode>& brandSet)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();

  std::string brandsString =
      DiagnosticUtil::containerToString(brandSet, ",", MAX_WIDTH, 0, true, true);
    dc << brandsString;
}

void
Diag892Collector::printItinParityInfo(const Itin* itin, const PricingTrx* trx,
  const skipper::UnorderedBrandCodes& brands, const skipper::UnorderedBrandCodes& currentLegBrands)
{
  if (!_active)
    return;

  TSE_ASSERT(itin != nullptr);
  TSE_ASSERT(trx != nullptr);

  DiagCollector& dc = asDiagCollector();

  std::ostringstream label;
  label << "ALL BRAND CODES FOR ITIN " << itin->itinNum() << ": ";
  dc << label.str();

  skipper::OrderedBrandCodes brandsToPrint(brands.begin(), brands.end());
  if (trx->getRequest()->isProcessParityBrandsOverride())
  {
    brandsToPrint.push_back("|"); // insert marker to distinguish additional brands
    // calculate if any additional brands are added
    if (currentLegBrands.empty())
      brandsToPrint.push_back("N/A");
    skipper::OrderedBrandCodes newBrands(currentLegBrands.size());
    skipper::OrderedBrandCodes::iterator it = std::set_difference(
        currentLegBrands.begin(), currentLegBrands.end(),
        brands.begin(), brands.end(),
        newBrands.begin());
    newBrands.resize(it - newBrands.begin());
    for (const BrandCode& brand: newBrands)
      brandsToPrint.push_back(brand);
  }
  dc << (brandsToPrint.empty() ? "N/A" :
         DiagnosticUtil::containerToString(brandsToPrint, " ", MAX_WIDTH, label.str().length(), true, true));
  dc << "\n\n";
}

void
Diag892Collector::printContextShoppingItinParityInfo(const Itin* itin, const PricingTrx* trx,
  const skipper::UnorderedBrandCodes& brands, const skipper::UnorderedBrandCodes& currentLegBrands)
{
  if (!_active)
    return;

  TSE_ASSERT(itin != nullptr);
  TSE_ASSERT(trx != nullptr);

  if (!trx->isContextShopping())
    return;

  DiagCollector& dc = asDiagCollector();

  dc << "\n** PARITY CALCULATION FOR ITIN " << itin->itinNum() << " **\n\n";

  uint16_t legIndex = 0;
  bool alreadyGotNotFixed = false;
  const bool isProcessParityBrandsOverride = trx->getRequest()->isProcessParityBrandsOverride();
  for (const TravelSegPtrVec& leg: itin->itinLegs())
  {
    if ((leg.size() == 1) &&
        (leg.at(0)->isArunk()))
    {
      dc << "(ARUNK)\n";
      continue;
    }
    dc << "LEG " << std::setw(2) << legIndex << ": ";
    if (!trx->getFixedLegs()[legIndex])
    {
      skipper::OrderedBrandCodes brandsToPrint(brands.begin(), brands.end());
      if (!alreadyGotNotFixed && isProcessParityBrandsOverride)
      {
        alreadyGotNotFixed = true; // only for the first leg
        brandsToPrint.push_back("|"); // insert marker to distinguish additional brands
        if (currentLegBrands.empty())
          brandsToPrint.push_back("N/A");
        // calculate if any additional brands are added
        skipper::OrderedBrandCodes newBrands(currentLegBrands.size());
        skipper::OrderedBrandCodes::iterator it = std::set_difference(
            currentLegBrands.begin(), currentLegBrands.end(),
            brands.begin(), brands.end(),
            newBrands.begin());
        newBrands.resize(it - newBrands.begin());
        for (const BrandCode& brand: newBrands)
          brandsToPrint.push_back(brand);
      }
      dc << (brandsToPrint.empty() ? "N/A" :
          DiagnosticUtil::containerToString(brandsToPrint, " ", MAX_WIDTH, 8, true, true));
    }
    else
      dc << "FIXED";
    dc << "\n";
    ++legIndex;
  }
  dc << "\n";
}

void
Diag892Collector::printBrandsPerFareMarket(skipper::BrandCodesPerFareMarket& brands)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();

  dc << "\n** CALCULATED VALID BRANDS PER FARE MARKET **\n";

  for (const auto& fmBrands: brands)
  {
    TSE_ASSERT(fmBrands.first);
    dc << "\n" << fmBrands.first->boardMultiCity() << "-"
       << fmBrands.first->governingCarrier()
       << "-" << fmBrands.first->offMultiCity() << ": ";
    dc << DiagnosticUtil::containerToString(fmBrands.second, " ", MAX_WIDTH, 12, false, true);
  }
  dc << "\n";
}

void
Diag892Collector::printContextShoppingItinInfo(const Itin* itin, const PricingTrx* trx)
{
  if (!_active)
    return;

  TSE_ASSERT(itin != nullptr);
  TSE_ASSERT(trx != nullptr);

  const bool isContextShopping = trx->isContextShopping();
  const skipper::FareComponentShoppingContextsForSegments& contextData =
      trx->getFareComponentShoppingContexts();

  DiagCollector& dc = asDiagCollector();

  dc << "** PROCESSING ITIN " << itin->itinNum() << " **\n\n";

  uint16_t legIndex = 0;
  for (const TravelSegPtrVec& tSegs: itin->itinLegs())
  {
    if ((tSegs.size() == 1) && (tSegs[0]->isArunk()))
    {
      dc << "(ARUNK) " << tSegs[0]->origAirport() << "-"
         << tSegs[0]->destAirport() << "\n";
      continue;
    }

    dc << "LEG " << std::setw(2) << legIndex << ":";
    if (isContextShopping)
    {
      if (!trx->getFixedLegs()[legIndex])
        dc << " NOT";
      dc << " FIXED";
    }
    dc << "\n";
    for (TravelSeg* tSeg: tSegs)
    {
      dc << "        " << tSeg->origAirport() << "-" << tSeg->destAirport();
      if (isContextShopping)
      {
        const auto iter = contextData.find(tSeg->pnrSegment());
        if (iter != contextData.end())
        {
          BrandCode& brand = iter->second->brandCode;
          if (!brand.empty())
            dc << ": " << brand;
          else
            dc << ": NO BRAND DATA";
        }
        else
        {
          if (trx->getFixedLegs()[legIndex])
            dc << ": NO CONTEXT DATA";
        }
      }
      dc << "\n";
    }
    ++legIndex;
  }

  if (!itin->brandFilterMap().empty())
  {
    dc << "\nREQUESTED BRAND FILTERS FOR ITIN " << itin->itinNum() << ":\n";

    BrandFilterMap::const_iterator iter = itin->brandFilterMap().begin();
    for ( ;iter != itin->brandFilterMap().end(); ++iter)
    {
      const BrandCode& brandCode = iter->first;
      dc << "BRAND ID = " << brandCode << " PROGRAM IDs :";
      int len = 11 + brandCode.length() + 14;

      dc << DiagnosticUtil::containerToString(iter->second, " ", MAX_WIDTH, len, false, true);
      if (iter->second.empty())
        dc << " N/A";

      dc << "\n";
    }
    dc << "\n";
  }
  else
    dc << "\nNO BRAND FILTERS WERE REQUESTED FOR ITIN " << itin->itinNum() << "\n";
}

void
Diag892Collector::printItinInfo(const Itin* itin, bool isSearchForBrandsPricing)
{
  if (!_active)
    return;

  TSE_ASSERT(itin != nullptr);

  DiagCollector& dc = asDiagCollector();

  dc << "** PROCESSING ITIN " << itin->itinNum() << " **\n\n";
  uint16_t legIndex = 0;

  for (const TravelSegPtrVec& tSegs : itin->itinLegs())
  {
    if (isSearchForBrandsPricing)
      dc << "\n";
    else
      dc << "LEG " << legIndex << ":";

    for (TravelSeg* tSeg : tSegs)
      dc << " " << tSeg->origAirport() << "-" << tSeg->destAirport();

    if ((tSegs.size() == 1) && (tSegs[0]->isArunk()))
      dc << " (ARUNK)";

    dc << "\n";
    ++legIndex;
  }

  if (!itin->brandFilterMap().empty())
  {
    dc << "\nREQUESTED BRAND FILTERS FOR ITIN " << itin->itinNum() << ":\n";

    BrandFilterMap::const_iterator iter = itin->brandFilterMap().begin();
    for ( ;iter != itin->brandFilterMap().end(); ++iter)
    {
      const BrandCode& brandCode = iter->first;
      dc << "BRAND ID = " << brandCode << " PROGRAM IDs :";
      int len = 11 + brandCode.length() + 14;

      dc << DiagnosticUtil::containerToString(iter->second, " ", MAX_WIDTH, len, false, true);
      if (iter->second.empty())
        dc << " N/A";

      dc << "\n";
    }
    dc << "\n";
  }
  else
    dc << "\nNO BRAND FILTERS WERE REQUESTED FOR ITIN " << itin->itinNum() << "\n";
}

void
Diag892Collector::printBrandsRemovedFromTrx(const PricingTrx& trx,
  const std::vector<QualifiedBrand>& newQualifiedBrands)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();

  std::string brandsStr;
  std::set<BrandCode> brands;

  dc << "\nALLOWED BRANDS: "; //len 16
  if (trx.getBrandsFilterForIS().empty())
    dc << "ALL (NO FILTERING)\n";
  else
  {
    brandsStr = DiagnosticUtil::containerToString(trx.getBrandsFilterForIS(), ",",
      MAX_WIDTH, 16, true, true);
    dc << brandsStr << "\n";
  }
  dc << "\nTRX BRANDS BEFORE FILTERING: "; // len 29

  for (const QualifiedBrand& qBrand: trx.brandProgramVec())
    brands.insert(qBrand.second->brandCode());
  brandsStr =
      DiagnosticUtil::containerToString(brands, ",", MAX_WIDTH, 29, true, true);
  dc << brandsStr << "\n";
  dc << "TRX BRANDS AFTER FILTERING:  "; // len 29
  brands.clear();
  for (const QualifiedBrand& qBrand: newQualifiedBrands)
    brands.insert(qBrand.second->brandCode());
  brandsStr =
      DiagnosticUtil::containerToString(brands, ",", MAX_WIDTH, 29, true, true);
  dc << brandsStr << "\n\n";
}

void
Diag892Collector::printRemovedFareMarkets(std::set<FareMarket*>& fms)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();

  dc << "\nFARE MARKETS REMOVED IN ITIN ANALYZER:\n";
  for (FareMarket* fm : fms)
  {
    if (!fm)
      continue;

    dc << fm->origin()->loc() << "-" << fm->governingCarrier() << "-" << fm->destination()->loc()
       << " BECAUSE OF ERROR CODE : " << fm->failCode() << "\n";
  }
  dc << "\n";
}

void Diag892Collector::printComparator(
    const BrandedFaresComparator& comparator)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();

  dc << "\nORDER OF BRANDS IN COMPARATOR:\n";
  dc << DiagnosticUtil::containerToString(comparator.getOrderedBrands(), ",", MAX_WIDTH, 0, true, true)
     << "\n\n";
}

namespace {
  template <typename B>
  void printSegmentOrientedBrandCodesTemplate(Diag892Collector& diag,
                                              const B& brands,
                                              const char* title)
  {
    if (!diag.isActive())
      return;

    diag << "\n" << title << ":\n";
    for (size_t index = 0; index < brands.size(); ++index)
    {
      diag << "SEGMENT " << (index < 10 ? "0" : "") << index << ": ";
      auto iter = brands[index].cbegin();
      for (; iter != brands[index].cend(); ++iter)
      {
        const CarrierCode& cx = iter->first.carrier;
        const Direction& dir = iter->first.direction;
        if (iter != brands[index].cbegin())
        {
          diag << std::string(12, ' ');
        }
        std::ostringstream carrier;
        if (cx.empty())
        {
          carrier << "?";
        }
        else
        {
          carrier << cx;
        }
        if (carrier.str().length() < 3)
        {
          carrier << std::string(3 - carrier.str().length(), ' ');
        }
        diag << carrier.str() << " (" << directionToIndicator(dir) << "): ";

        if (iter->second.empty() ||
            (iter->second.size() == 1 && *iter->second.cbegin() == NO_BRAND))
        {
          diag << "--\n";
        }
        else
        {
          diag << DiagnosticUtil::containerToString(
              iter->second, ",", diag.MAX_WIDTH,
              12 + 3/*cx*/ + 4/*dir*/ + 2, true, true) << "\n";
        }
      }
      if (brands[index].empty())
        diag << "\n";
      diag << "\n";
    }
    diag << "\n";
  }
}

void Diag892Collector::printSegmentOrientedBrandCodesAfterSorting(
    const skipper::SegmentOrientedBrandCodeArraysPerCarrier& brands)
{
  printSegmentOrientedBrandCodesTemplate(*this, brands,
                                         "SORTED BRAND CODES PER CARRIER");
}

void Diag892Collector::printSegmentOrientedBrandCodes(
    const skipper::SegmentOrientedBrandCodesPerCarrier& brands)
{
  printSegmentOrientedBrandCodesTemplate(*this, brands,
                                         "BRAND CODES PER CARRIER");
}

void Diag892Collector::printSpacesSegments(size_t begin, size_t end, size_t segmentCount,
                                           const std::vector<std::vector<std::string>>& spaceString)
{
  std::vector<std::string> segments;
  bool nextCxr = true;
  size_t cxrIndex = 0;
  while (nextCxr)
  {
    segments.clear();
    nextCxr = false;
    for (size_t segIndex = begin; segIndex < segmentCount && segIndex < end; ++segIndex)
    {
      if (cxrIndex < spaceString[segIndex].size())
      {
        segments.push_back(spaceString[segIndex][cxrIndex]);
        if (cxrIndex + 1 < spaceString[segIndex].size())
        {
          nextCxr = true;
        }
      }
      else
      {
        segments.push_back(std::string(18, ' '));
      }
    }
    *this << std::string(3, ' ')
          << DiagnosticUtil::containerToString(segments, "  ", MAX_WIDTH, 3, false) << "\n";
    ++cxrIndex;
  }
}

void Diag892Collector::printBrandingOptionSpaces(
    const skipper::BrandingOptionSpaces& spaces)
{
  if (!_active)
    return;

  DiagCollector& dc = asDiagCollector();

  size_t segmentCount = spaces[0].size();
  dc << "\nBRANDING OPTION SPACES GENARATED FOR THIS ITIN:\n";

  //we can fit information about max 3 segments in one line, so split headers
  const size_t maxNumOfSeg = 3;
  std::vector<std::vector<std::string>> headers;
  //create headers
  for (size_t index = 0; index < segmentCount; ++index)
  {
    if (index % maxNumOfSeg == 0)
      headers.push_back(std::vector<std::string>());

    std::stringstream tmp;
    tmp << "SEG. " << (index < 10 ? "0" : "") << index << std::string(11, ' ');
    headers.back().push_back(tmp.str());
  }

  for (size_t spacesIndex = 0; spacesIndex < spaces.size(); ++spacesIndex)
  {
    dc << (spacesIndex < 10 ? " " : "") << spacesIndex << ":";
    std::vector<std::vector<std::string> > spaceString;
    brandingOptionSpaceToString(spaces[spacesIndex], spaceString);
    size_t begin = 0;
    for(const auto& header : headers)
    {
      if (begin != 0)
        dc << "\n" << std::string(3, ' ');
      dc << DiagnosticUtil::containerToString(header, "  ", MAX_WIDTH, 3, false) << "\n";
      printSpacesSegments(begin, begin + header.size(), segmentCount, spaceString);
      begin += header.size();
    }
    dc << "\n\n";
  }
}

void Diag892Collector::printBrandingOptionsHeader()
{
  if (_active)
    {
      //                 1         2         3         4         5         6
      //        1234567890123456789012345678901234567890123456789012345678901234
      *this << "************** CALCULATING BRANDING OPTION SPACES  *************\n";
    }
}

void Diag892Collector::printAfterReqBrandsFilteringHeader()
{
  if (_active)
  {
    //                 1         2         3         4         5         6
    //        1234567890123456789012345678901234567890123456789012345678901234
    *this << "************** AFTER BRAND FILTERING  *************\n";
  }
}

void Diag892Collector::printValidBrandsForCabinHeader()
{
  if (_active)
  {
    //                 1         2         3         4         5         6
    //        1234567890123456789012345678901234567890123456789012345678901234
    *this << "** BRANDS VALID FOR REQUESTED CABIN: **\n";
  }
}

void Diag892Collector::brandingOptionSpaceToString(
    const skipper::BrandingOptionSpace& space,
    std::vector<std::vector<std::string> >& brandsPerSegment)
{
  for (const skipper::CarrierBrandPairs& carriersBrandsinSegment : space)
  {
    std::vector<std::string> currentSegmentBrands;

    skipper::CarrierBrandPairs::const_iterator it = carriersBrandsinSegment.begin();
    for ( ; it != carriersBrandsinSegment.end(); ++it)
    {
      std::stringstream tmp;

      const CarrierCode& cx = it->first.carrier;
      const Direction& dir = it->first.direction;
      std::ostringstream carrier;
      if (cx.empty())
      {
        //this should never happen...
        carrier << "?";
      }
      else
      {
        carrier << cx;
      }
      if (carrier.str().length() < 3)
      {
        carrier << std::string(3 - carrier.str().length(), ' ');
      }
      tmp << carrier.str() << "(" << directionToIndicator(dir) << "): ";

      if (it->second.empty())
      {
        //this should never happen...
        tmp << "??" << std::string(8, ' ');
      }
      else
      {
        if (it->second == NO_BRAND)
        {
          tmp << "--" << std::string(8, ' ');
        }
        else
        {
          tmp << it->second << std::string(10 - it->second.length(), ' ');
        }
      }
      currentSegmentBrands.push_back(tmp.str());
    }
    brandsPerSegment.push_back(currentSegmentBrands);
  }
}

void Diag892Collector::collectFareForValidBrand(size_t segIndex,
                             const CarrierCode& carrier,
                             const BrandCode& brandCode,
                             const FareClassCode& fareClass)
{
  std::ostringstream ss;
  ss << "SEG: " << segIndex << " CARRIER: " << carrier << " BRAND: " << brandCode << " FARE: " << fareClass;
  _filterByCabinInfoVec.push_back(ss.str());
}

void Diag892Collector::printCarrierBrandAndCabinInfo(
  const skipper::SegmentOrientedBrandCodesPerCarrierInCabin& brandsPerCabin)
{
  if (!_active)
    return;

  *this << "\n** BRANDS PER CABIN SEGMENT AND CARRIER: **";

  for (skipper::SegmentOrientedBrandCodesPerCarrierInCabin::const_iterator cabinIter =
         brandsPerCabin.begin();
       cabinIter != brandsPerCabin.end();
       ++cabinIter)
  {
    std::ostringstream scabin;
    scabin << "CABIN: " << CabinType::getClassAlphaNum(cabinIter->first);
    *this << "\n\n" << scabin.str();

    for (size_t segIndex = 0; segIndex < cabinIter->second.size(); ++segIndex)
    {
      std::ostringstream ssegment;
      ssegment << " SEG: " << (segIndex < 10 ? "0" : "") << segIndex;
      if (segIndex != 0)
      {
        *this << "\n" << std::string(scabin.str().length(), ' ');
      }
      *this << ssegment.str();

      const skipper::BrandCodesPerCarrier& carrierBrands =
          brandsPerCabin.at(cabinIter->first).at(segIndex);
      for (skipper::BrandCodesPerCarrier::const_iterator carrierIter =
             carrierBrands.begin();
           carrierIter != carrierBrands.end();
           ++carrierIter)
      {
        const CarrierCode& cx = carrierIter->first.carrier;
        const Direction& dir = carrierIter->first.direction;
        if (carrierIter != carrierBrands.begin())
        {
          *this << "\n"
                << std::string(scabin.str().length() + ssegment.str().length(), ' ');
        }

        std::ostringstream sbrands;
        std::ostringstream carrier;
        if (cx.empty())
        {
          carrier << "?";
        }
        else
        {
          carrier << cx;
        }
        if (carrier.str().length() < 3)
        {
          carrier << std::string(3 - carrier.str().length(), ' ');
        }
        carrier << " (" << directionToIndicator(dir) << ")";
        sbrands << " CARRIER: " << carrier.str() << " BRANDS: ";
        *this << sbrands.str();

        if (carrierIter->second.empty())
        {
          *this << "--";
        }
        else
        {
          *this << DiagnosticUtil::containerToString(
                     carrierIter->second,
                     ",",
                     MAX_WIDTH,
                     scabin.str().length() + ssegment.str().length() + sbrands.str().length(),
                     true,
                     true);
        }
      }
    }
  }
  *this << "\n\n";
}

void Diag892Collector::printCabinHeader(size_t cabinIndex)
{
  if (_active)
  {
           //          1         2         3         4         5         6
           // 1234567890123456789012345678901234567890123456789012345678901234
    *this << "\nINFORMATION FOR CABIN " << CabinType::getClassAlphaNum(cabinIndex)
          << ":\n";
  }
}

void Diag892Collector::printFilterByCabinInfo()
{
  if (_active)
  {
    //                 1         2         3         4         5         6
    //        1234567890123456789012345678901234567890123456789012345678901234
    *this << "* BRANDS DEEMED VALID FOR REQ.CABIN BECAUSE OF FOLLOWING FARES: \n\n";

    if (_filterByCabinInfoVec.empty())
    {
      *this << "N/A\n\n";
      return;
    }

    for (const std::string& message : _filterByCabinInfoVec)
    {
      *this << message << std::endl;
    }
    *this << std::endl;
  }
}

void Diag892Collector::printDeduplicationHeader()
{
  if (_active)
  {
    //                 1         2         3         4         5         6
    //        1234567890123456789012345678901234567890123456789012345678901234
    *this << "\n\n*********** BRANDING OPTION SPACES DEDUPLICATION INFO **********\n\n";
  }
}

void Diag892Collector::printDeduplicationInfo(
  const Itin* itin, const skipper::BrandingOptionSpacesDeduplicator::KeyBrandsMap& keys)
{
  if (!_active)
    return;

  TSE_ASSERT(itin != nullptr);

  DiagCollector& dc = asDiagCollector();

  dc << "ITIN: " << itin->itinNum() << "\n";
  skipper::BrandingOptionSpacesDeduplicator::KeyBrandsMap::const_iterator iter =
      keys.begin();
  for (; iter != keys.end(); ++iter)
  {
    dc << "KEY: ";
    for (const auto& elem : iter->first)
    {
      dc << elem.first << ":" << (elem.second == NO_BRAND ? "--" : elem.second) << " ";
    }
    dc << "\n";
    dc << "INDICES: " << DiagnosticUtil::containerToString(iter->second, ",", MAX_WIDTH, 9);
    dc << "\n\n";
  }
  dc << "\n";
}

void Diag892Collector::printSegmentsLegIdInfo(const TravelSegPtrVec& segments)
{
  if (!_active)
    return;

  for (TravelSeg* seg : segments)
  {
    *this << seg->origin()->loc() << "-" << seg->destination()->loc()
          << "(" << static_cast<char>(seg->segmentType()) << ")"
          << ":" << seg->legId() << "\n";
  }
}

} // tse
