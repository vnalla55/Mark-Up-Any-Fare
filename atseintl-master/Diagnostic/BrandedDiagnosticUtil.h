//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/PricingUnit.h"
#include "Common/IAIbfUtils.h"

#include <string>


namespace tse
{
class Diagnostic;
class FareMarket;
class FarePath;
class PaxTypeFare;
class PricingTrx;
class Trx;

class BrandedDiagnosticUtil
{
public:
  BrandedDiagnosticUtil() = delete;
  virtual ~BrandedDiagnosticUtil() = delete;

  static std::string
  displayBrandedFaresFarePathValidation(PricingTrx* trx, const FarePath& farePath);

  static void displayBrand(std::ostringstream& out, const QualifiedBrand& qb);

  static void displayBrandProgram(std::ostringstream& out, const QualifiedBrand& qb,
                                  bool showOnd = false);

  static void displayBrandIndices(std::ostringstream& out,
                                  const std::vector<QualifiedBrand>& qbs,
                                  const std::vector<int>& indexVector,
                                  bool showOnd = false);

  static void displayAllBrandIndices(std::ostringstream& out,
                                     const std::vector<QualifiedBrand>& qbs,
                                     bool showOnd = false);

  static void displayFareMarketErrorMessagePerBrand(std::ostringstream& out, const PricingTrx& trx,
                                                    const FareMarket& fm);

  static void displayBrandRetrievalMode(std::ostringstream& out, BrandRetrievalMode mode);

  static void displayTripType(std::ostringstream& out, IAIbfUtils::TripType tripType);

  static void displayFareMarketsWithBrands(std::ostringstream& out,
                                           const std::vector<FareMarket*>& fareMarkets,
                                           const std::vector<QualifiedBrand>& qbs,
                                           const IAIbfUtils::OdcsForBranding* odcs = nullptr,
                                           bool showOnd = false);
private:
  static void displayErrorMessagesLegend(std::ostringstream& out);

  static void displayFareMarket(std::ostringstream& out, int index, FareMarket* fm,
                                const IAIbfUtils::OdcsForBranding* odcs,
                                bool showOnd = false);

  static void displayFareMarketBrandIndicies(std::ostringstream& out, const FareMarket& fm,
                                             const std::vector<QualifiedBrand>& qbs);
};

} // end tse namespace
