//----------------------------------------------------------------------------
//  File:        Diag889Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 889 formatter
//
//  Updates:
//          date - initials - description.
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/SvcFeesDiagCollector.h"

#include <map>

namespace tse
{
class SvcFeesFareIdInfo;
class BrandProgram;
class BrandInfo;

class Diag889Collector : public SvcFeesDiagCollector
{
  friend class Diag889CollectorTest;

public:
  explicit Diag889Collector(Diagnostic& root) : SvcFeesDiagCollector(root) {}
  Diag889Collector() {}

  void printT189Banner();
  void printPaxTypeFareCommonHeader();
  Diag889Collector& operator<<(const PaxTypeFare& paxFare) override;
  void printSeqNoAndStatus(const SvcFeesFareIdInfo* svcFeesFareIdInfo, StatusT189 status);
  void printTravelInfo(const FareMarket* fm);
  void printSeparator();
  void displayT189Secondary(const SvcFeesFareIdInfo* t189Secondary);
  void printDetailInfo(const SvcFeesFareIdInfo* svcFeesFareIdInfo, StatusT189 status);
  void printDetailBrandProgram(const BrandProgram* brandPr);
  void printBrand(const BrandInfo* brand);
  void printT189NotFound(const BrandInfo* brand);
  void printBrandProgram(const BrandProgram* brandPr);
  void printFailGlobalDirection(const BrandProgram& brandPr, const PaxTypeFare& paxTypeFare);
  void printFailDirectionality(const BrandProgram& brandPr, const PaxTypeFare& paxTypeFare,
    bool useDirectionality = false, Direction programDirection = Direction::BOTHWAYS);
  void printBrandT189Item(const SvcFeesFareIdInfo* svcFeesFareIdInfo, const BrandInfo* brand);
  void printBlankLine();
  void printSize(const char* txt, int size);
  void printDataNotFound();
  void printDataNotFound(const FareMarket& fareMarket);
  void printCompleteBrandProgram(const BrandProgram* brandPr);
  void printCompleteBrand(const BrandInfo* brand);
  void printBrandProgramNotFound();
  void printBrandProgramNotFound(const FareMarket& fareMarket);
  void printBrandNotFound();
  void printFareMarketNotMatched(const FareMarket& fareMarket);
  void printNoFaresFound(const FareMarket& fareMarket);
  void printT189SecondaryItemNoAndSeqNo(const SvcFeesFareIdInfo* svcFeesFareIdInfo, bool status);
  void printT189NotExist(const FareMarket& fareMarket);
  void printT189SecondaryDetailInfo(const SvcFeesFareIdInfo* info, bool status);
  void printFareNotValid();
  void printCarrierNotMatched(const FareMarket& fareMarket);
  void printBrandSizeAndCurrentBrand(const BrandInfo* brand, int size);
  void printCurrentBrand(const BrandInfo* brand);
  void displayT189Status(StatusT189 status, bool displayDetails);
  void printBrandFilter(const FareMarket& fareMarket, const BrandCode& brandCode, bool match);
  void printIsValidForBranding(const PaxTypeFare& paxTypeFare);
  void printTable189Header();
  void printItemNoAndSeqNo(const SvcFeesFareIdInfo* svcFeesFareIdInfo, bool primary);
  void printT189SecondaryStatus(DiagCollector& dc, bool status);
  void printNoProgramFound(const FareMarket& fareMarket);
  void printMatchBasedOnFareBasisCodeHeader();
  void printMatchBasedOnExcludedFareBasisCodeHeader();
  void printMatchBasedOnFareBasisCode(const FareBasisCode& fareBasisCode, bool isValid);
  void printMatchBasedOnBookingCodeHeaderPrimary();
  void printMatchBasedOnBookingCodeHeaderSecondary();
  void printMatchBasedOnBookingCode(const BookingCode& bookingCode, bool isValid);
  void printValidateCbasResult(PaxTypeFare::BrandStatus status);
  void printYyFareIgnored();

private:
  void displayStatus(StatusT189 rc);
};
} // namespace tse

