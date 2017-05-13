//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#pragma once

#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

#include <vector>

namespace tse
{
class FareMarketPath;
class FaremarketPathMatrix;
class Itin;
class MergedFareMarket;
class PricingTrx;
class PUPathMatrix;

class Diag600Collector : public DiagCollector
{
public:
  void displayFareMarkets(const Itin& itin,
                          const std::vector<FareMarketPath*>& fareMarketPathMatrix,
                          const BrandCode& brandCode);

  void displayFareMarkestWithoutTag2Fare(const Itin& itin,
                                         const std::vector<MergedFareMarket*>& mfms);

  void displayFMPMatrix(const FareMarketPathMatrix& fMatrix,
                        const Itin* itin,
                        const std::vector<FareMarketPath*>* fmps = nullptr);

  void displayPUPathMatrix(const PUPathMatrix& puMatrix,
                           const Itin* itin = nullptr,
                           const FareMarketPath* fmp = nullptr,
                           const std::vector<FareMarketPath*>* fmps = nullptr);
};

} // namespace tse
