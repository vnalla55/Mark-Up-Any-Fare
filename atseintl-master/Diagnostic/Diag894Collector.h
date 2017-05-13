//----------------------------------------------------------------------------
//  File:        Diag894Collector.h
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

#pragma once

#include "Common/IAIbfUtils.h"
#include "Common/TseEnums.h"
#include "Common/TseStlTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/DiagCollector.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"

namespace tse
{

class Diag894Collector : public DiagCollector
{
public:
  void printHeader() override;

  void printFooter();

  void printFareValidationStatus(PaxTypeFare::BrandStatus status);
  void printFareValidationStatusHotHardPass();

  void printProgramToFareMarketDirectionMatch(bool match);

  void printProgramDirectionCalculationDetails(
    const BrandProgram& brandProgram, const FareMarket& fareMarket,
    bool isOriginal, bool isReversed, Direction direction);

  void printQualifiedBrandInfo(int index, const QualifiedBrand& qualifiedBrand);

  void printPaxTypeFarInfo(const PaxTypeFare& paxTypeFare);

  void printItinFareUsageInfo(Itin& itin,
                              PricingTrx& trx);

  bool isDDINFO() const
  {
    return _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
  }

  const std::string& getFareMarket() const
  {
    return _trx->diagnostic().diagParamMapItem(Diagnostic::FARE_MARKET);
  }

  const std::string& getBrandCode() const
  {
    return _trx->diagnostic().diagParamMapItem(Diagnostic::BRAND_ID);
  }

  const std::string& getFare() const
  {
    return _trx->diagnostic().diagParamMapItem(Diagnostic::FARE_BASIS_CODE);
  }

  bool isValidForPaxTypeFare(const PricingTrx& trx, const PaxTypeFare& paxTypeFare) const;

  DiagCollector& asDiagCollector() { return static_cast<DiagCollector&>(*this); }

  void printImproper894Use();
private:

};

} // namespace tse

