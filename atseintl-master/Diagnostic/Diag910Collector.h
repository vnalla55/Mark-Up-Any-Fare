//----------------------------------------------------------------------------
//  File:        Diag910Collector.h
//  Created:     2004-10-19
//
//  Description: Diagnostic 910: Shopping FarePath display
//
//  Updates:
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"
#include "Pricing/Shopping/FOS/FosTypes.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <string>

namespace tse
{
class FarePath;
class GroupFarePath;
class PricingTrx;

namespace fos
{
class FosTaskScope;
class FosStatistic;
}

using namespace utils;

class Diag910Collector : public DiagCollector
{
public:
  Diag910Collector() : _diagFlights(false), _diagFos(false) {}

  Diag910Collector& operator<<(const PricingTrx& trx) override;

  void printHeader(bool showOwFareKeyDetails = false);
  void printFarePaths(const ShoppingTrx::FlightMatrix& flightMatrix,
                      const ShoppingTrx::EstimateMatrix& estimateMatrix,
                      const ShoppingTrx& trx,
                      bool showOwFareKeyDetails = false);
  void printGroupFarePath(const GroupFarePath& paths,
                          const ShoppingTrx& trx,
                          bool showOwFareKeyDetails = false);
  void
  printFarePath(const FarePath& path, const ShoppingTrx& trx, bool showOwFareKeyDetails = false);
  void
  outputExcessiveOptions(const ShoppingTrx& trx, const SopIdVec& sops, GroupFarePath* gfp);
  void printVITAData(const ShoppingTrx& trx,
                     const SopIdVec& myCell,
                     const std::vector<TravelSeg*>& travelSeg,
                     const CarrierCode& validatingCarrier,
                     bool intTicketValidationResult,
                     bool validatingCarrierFromCache = false,
                     const std::string& validationMessage = std::string());
  void printVITAData(const ShoppingTrx& trx,
                     const SopIdVec& myCell,
                     const CarrierCode& validatingCarrier,
                     bool intTicketValidationResult,
                     const std::string& validationMessage = std::string());

  void printFlights(const ShoppingTrx& trx, const SopIdVec& sops);
  std::string sopsToStr(const ShoppingTrx& trx,
                        const SopIdVec& sops,
                        bool specialSolIndicators = true) const;

  // FOS methods
  void initFosDiagnostic();
  void printFosProcessing(const fos::FosTaskScope& task);
  void printFosDeferredProcessing(const fos::FosStatistic& stats,
                                  const fos::FosGeneratorStats& genStats);
  void printFosProcessingFinished(const fos::FosStatistic& stats);
  void printValidatorAdded(fos::ValidatorType);
  void printFilterAdded(fos::FilterType);
  void printFos(const ShoppingTrx& trx,
                const SopIdVec& combination,
                const SopIdVec& base,
                fos::ValidatorBitMask validBitMask);
  void printTracedFosStatus(const ShoppingTrx& trx,
                            const SopIdVec& sops,
                            fos::ValidatorBitMask validBM,
                            fos::ValidatorBitMask deferredBM,
                            fos::ValidatorBitMask invalidSopDetailsBM);
  void printTracedFosThrownAway(const ShoppingTrx& trx, const SopIdVec& sops);
  void printTracedFosFailedPredicate(const ShoppingTrx& trx,
                                     const SopCombination& sops,
                                     const INamedPredicate<SopCombination>& failedPredicate);
  void printFosFailedPredicate(const ShoppingTrx& trx,
                               const SopCombination& sops,
                               const INamedPredicate<SopCombination>& failedPredicate);

  void displayFarePath(PricingTrx& trx, FarePath& fPath);

private:
  void printFamilies(const ShoppingTrx::FlightMatrix& flightMatrix,
                     const ShoppingTrx::EstimateMatrix& estimateMatrix,
                     const ShoppingTrx& trx);

  void printServerInfo();

  void printFosTaskScope(const fos::FosTaskScope& task);
  void printFosValidationLegend();
  void printFosStatistic(const fos::FosStatistic& stats);
  std::string getFosValidatorStr(fos::ValidatorType) const;
  std::string getFosValidatorBitMaskStr(fos::ValidatorBitMask bitMask) const;
  std::string getFosFilterStr(fos::FilterType) const;
  char getFosValidatorShortcut(fos::ValidatorType) const;
  bool isTracedFos(const ShoppingTrx& trx, const SopIdVec& sops) const;

  bool _diagFlights;
  bool _diagFos;
  SopIdVec _diagTracedFos;
};
}

