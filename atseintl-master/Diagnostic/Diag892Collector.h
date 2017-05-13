//----------------------------------------------------------------------------
//  File:        Diag892Collector.h
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

#pragma once

#include "BrandedFares/BrandedFaresComparator.h"
#include "Common/IAIbfUtils.h"
#include "Common/TseEnums.h"
#include "Common/TNBrands/BrandingOptionSpacesDeduplicator.h"
#include "DataModel/TNBrandsTypes.h"
#include "Diagnostic/DiagCollector.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"

namespace tse
{

class Diag892Collector : public DiagCollector
{
public:
  const size_t MAX_WIDTH;

  Diag892Collector() : MAX_WIDTH(64) {}

  void printHeader() override;
  void printFooter();
  void printBrands(const std::set<BrandCode>& brandSet);
  void printItinInfo(const Itin* itin, bool isSearchForBrandsPricing);
  void printItinParityInfo(const Itin* itin, const PricingTrx* trx,
    const skipper::UnorderedBrandCodes& brands,
    const skipper::UnorderedBrandCodes& currentLegBrands);
  void printContextShoppingItinInfo(const Itin* itin, const PricingTrx* trx);
  void printContextShoppingItinParityInfo(const Itin* itin, const PricingTrx* trx,
    const skipper::UnorderedBrandCodes& brands,
    const skipper::UnorderedBrandCodes& currentLegBrands);
  void printRemovedFareMarkets(std::set<FareMarket*>& fms);
  void printBrandsRemovedFromTrx(const PricingTrx& trx,
    const std::vector<QualifiedBrand>& newQualifiedBrands);

  void printBrandingOptionSpaces(const skipper::BrandingOptionSpaces& spaces);
  void printSpacesSegments(size_t begin, size_t end, size_t segmentCount,
                           const std::vector<std::vector<std::string>>& spaceString);
  void printBrandingOptionsHeader();
  void printAfterReqBrandsFilteringHeader();
  void printValidBrandsForCabinHeader();
  void printSegmentOrientedBrandCodes(
      const skipper::SegmentOrientedBrandCodesPerCarrier& brands);
  void printSegmentOrientedBrandCodesAfterSorting(
      const skipper::SegmentOrientedBrandCodeArraysPerCarrier& brands);
  void printComparator(const BrandedFaresComparator& comparator);
  bool isDDINFO()
  {
    return _active && _trx &&
      _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
  }
  bool isDDPARITY()
  {
    return _active && _trx &&
      _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PARITY";
  }
  void printBrandsPerFareMarket(skipper::BrandCodesPerFareMarket& brands);
  void printFilterByCabinInfo();
  void collectFareForValidBrand(size_t segIndex,
      const CarrierCode& carrier,
      const BrandCode& brandCode,
      const FareClassCode& fareClass);

  void printCarrierBrandAndCabinInfo(
    const skipper::SegmentOrientedBrandCodesPerCarrierInCabin& brandsPerCabin);
  void printCabinHeader(size_t cabinIndex);
  void printNA() { if (_active) { *this << "N/A\n"; }}

  void printDeduplicationHeader();
  void printDeduplicationInfo(const Itin* itin,
                              const skipper::BrandingOptionSpacesDeduplicator::KeyBrandsMap& keys);

  void printSegmentsLegIdInfo(const TravelSegPtrVec& segments);
  DiagCollector& asDiagCollector() { return static_cast<DiagCollector&>(*this); }

protected:
  // Updates given vector of vector of string by adding information about given
  // brandingOptionSpace (combination of brands) in a following format:
  // [segment][]:  string
  //       [0][0]: CX1: AA
  //       [0][1]: CX2: ZZ
  //       [1][0]: CX1: --
  // -- is an indicator for NO_BRAND
  void brandingOptionSpaceToString(
      const skipper::BrandingOptionSpace& space,
      std::vector<std::vector<std::string> >& brandsPerSegment);
private:
  std::vector<std::string>  _filterByCabinInfoVec;
};

} // namespace tse

