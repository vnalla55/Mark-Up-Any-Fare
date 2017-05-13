//-------------------------------------------------------------------
//
//  File:        BrandedFareDiagnostics.h
//  Created:     2014
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2014
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"


#include <vector>

namespace tse
{

class PricingTrx;
class SvcFeesFareIdInfo;
class BrandProgram;
class BrandInfo;
class Diag889Collector;
class FareMarket;
struct SecSvcFeesFareIdInfo;

class BrandedFareDiagnostics
{
  friend class S8BrandedFaresSelectorTest;
  PricingTrx& _trx;
  Diag889Collector*& _diag889;
  bool createDiag();
  void deleteDiag();

public:
  BrandedFareDiagnostics(PricingTrx& trx, Diag889Collector*& diag889)
  : _trx(trx), _diag889(diag889)
  {
    createDiag();
  }

  virtual ~BrandedFareDiagnostics()
  {
    deleteDiag();
  }
  bool isDiag889Enabled() const { return _diag889 != nullptr; }
  void printT189Banner();
  void printNoFaresFound(const FareMarket& fareMarket);

  void printDataNotFound(const FareMarket& fareMarket);
  void printCarrierNotMatched(const FareMarket& fareMarket);
  void printHeaderInfoSeparator(const FareMarket* fm, bool& hasPrintedPaxTypeFareCommonHeader);
  void printPaxTypeFare(const PaxTypeFare* paxTypeFare);
  void printFareNotValid();
  void printIsValidForBranding(const PaxTypeFare& paxTypeFare);
  void printNoProgramFound(const FareMarket& fareMarket);
  void printSeparator();
  void printHeaderInfoSeparator();
  void printTravelInfo(const FareMarket* fm);
  void printPaxTypeFareCommonHeader();
  void printSeqNoAndStatus(const SvcFeesFareIdInfo* svcFeesFareIdInfo, StatusT189 status);
  void printDetailInfo(const SvcFeesFareIdInfo* svcFeesFareIdInfo, StatusT189 status);
  void printDetailBrandProgram(const BrandProgram* brandPr);
  void printBrandProgram(const BrandProgram* brandPr);
  void printT189NotFound(const BrandInfo* brand);
  void printBrandT189Item(const SvcFeesFareIdInfo* svcFeesFareIdInfo, const BrandInfo* brand);
  void printBrand(const BrandInfo* brand);
  void printBlankLine();
  void printSize(const char* txt, int size);
  void printDataNotFound();
  void printCompleteBrandProgram(const BrandProgram* brandPr);
  void printCompleteBrand(const BrandInfo* brand);
  void printBrandProgramNotFound();
  void printBrandProgramNotFound(const FareMarket& fareMarket);
  void printBrandNotFound();
  void printFareMarketNotMatched(const FareMarket& fareMarket);
  void printBrandProgramSize(int size);
  void printProgram(const BrandProgram* brandPr, bool& needBrandProgramSeparator);
  void printProgramFailGlobalDirection(const BrandProgram& brandPr, const PaxTypeFare& paxTypeFare);
  void printProgramFailDirectionality(const BrandProgram& brandPr, const PaxTypeFare& paxTypeFare,
    Direction programDirection);
  void printBrandSize(int size);
  void printDetailBrand(const BrandInfo* brand);
  void printPT189Size(int size);
  void printPT189(const SvcFeesFareIdInfo* svcFeesFareIdInfo,
                  const BrandInfo* brand,
                  bool& needBrandSeparator,
                  StatusT189 rc,
                  bool& hasPrintedBrandT189Item);
  void printT189NotExist(const FareMarket& fareMarket);
  void printT189Secondary(const SvcFeesFareIdInfo* svcFeesFareIdInfo, bool status);
  void printCurrentBrand(const BrandInfo* brand, int size);
  bool isDDInfo() const;
  void displayT189Status(StatusT189 status, bool displayDetails);
  void printTable189Header() const;
  void displayT189Secondary(const SvcFeesFareIdInfo* t189Secondary);
  void printBrandFilter(const FareMarket& fareMarket, const BrandCode& brandCode, bool match) const;
  bool isForSeqNumber(SequenceNumber sequenceNumber) const;
  bool isForVendor(const VendorCode& vendorCode) const;
  bool isForProgramName(const ProgramCode& programCode) const;
  bool isForBrandId(const BrandCode& brandCode) const;
  bool isForCarrier(const CarrierCode& carrierCode) const;
  bool isForFareClassCode(const FareClassCode& fareClassCode) const;
  void
  displayT189Validation(StatusT189 rc, SvcFeesFareIdInfo * svcFeesFareIdInfo, const BrandInfo* brand, bool& needBrandSeparator, bool& hasPrintedBrandT189Item,
                        const std::vector<SecSvcFeesFareIdInfo*>& secondarySvcFeesFareIdInfoVec);
  void printMatchBasedOnFareBasisCodeHeader();
  void printMatchBasedOnBookingCodeHeaderPrimary();
  void printMatchBasedOnBookingCodeHeaderSecondary();
  void printMatchBasedOnExcludedFareBasisCodeHeader();
  void printMatchBasedOnFareBasisCode(const FareBasisCode& includedFareBasisCode, bool isValid);
  void printMatchBasedOnBookingCode(const BookingCode& bookingCode, bool isValid);
  void printValidateCbasResult(PaxTypeFare::BrandStatus status);
  void printYyFareIgnored();
};

} // tse

