//-------------------------------------------------------------------
//
//  File:        BrandedFareDiagnostics.cpp
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

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/S8BrandedFaresSelector.h"
#include "Common/BrandingUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag889Collector.h"
#include "Rules/RuleUtil.h"


namespace tse
{

bool
BrandedFareDiagnostics::createDiag()
{
  if (!_trx.diagnostic().isActive())
    return false;

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if ((!_diag889) && (diagType == Diagnostic889))
  {
    _diag889 = dynamic_cast<Diag889Collector*>(DCFactory::instance()->create(_trx));

    if (_diag889 == nullptr)
    {
      return false;
    }
    _diag889->enable(Diagnostic889);
    _diag889->activate();
    return true;
  }
  return false;
}

void
BrandedFareDiagnostics::printT189Banner()
{
  if (_diag889)
    _diag889->printT189Banner();
}

void
BrandedFareDiagnostics::printSeparator()
{
  if (_diag889)
    _diag889->printSeparator();
}

void
BrandedFareDiagnostics::printTravelInfo(const FareMarket* fm)
{
  if (_diag889)
    _diag889->printTravelInfo(fm);
}

void
BrandedFareDiagnostics::printPaxTypeFareCommonHeader()
{
  if (_diag889)
    _diag889->printPaxTypeFareCommonHeader();
}

void
BrandedFareDiagnostics::printPaxTypeFare(const PaxTypeFare* paxTypeFare)
{
  if (_diag889)
    (*_diag889) << *paxTypeFare;
}

void
BrandedFareDiagnostics::printSeqNoAndStatus(const SvcFeesFareIdInfo* svcFeesFareIdInfo,
                                            StatusT189 status)
{
  if (_diag889)
    _diag889->printSeqNoAndStatus(svcFeesFareIdInfo, status);
}

void
BrandedFareDiagnostics::displayT189Secondary(const SvcFeesFareIdInfo* t189Secondary)
{
  if (_diag889)
    _diag889->displayT189Secondary(t189Secondary);
}

void
BrandedFareDiagnostics::printHeaderInfoSeparator(const FareMarket* fm,
                                                 bool& hasPrintedPaxTypeFareCommonHeader)
{
  if (hasPrintedPaxTypeFareCommonHeader)
  {
    printSeparator();
  }
  else
  {
    printTravelInfo(fm);
    printPaxTypeFareCommonHeader();
    hasPrintedPaxTypeFareCommonHeader = true;
  }
}

void
BrandedFareDiagnostics::deleteDiag()
{
  if (_diag889)
  {
    _diag889->flushMsg();
  }
}

void
BrandedFareDiagnostics::printDetailInfo(const SvcFeesFareIdInfo* svcFeesFareIdInfo,
                                        StatusT189 status)
{
  if (_diag889)
    _diag889->printDetailInfo(svcFeesFareIdInfo, status);
}

void
BrandedFareDiagnostics::printDetailBrandProgram(const BrandProgram* brandPr)
{
  if (_diag889)
    _diag889->printDetailBrandProgram(brandPr);
}

void
BrandedFareDiagnostics::printBrandProgram(const BrandProgram* brandPr)
{
  if (_diag889)
    _diag889->printBrandProgram(brandPr);
}

void
BrandedFareDiagnostics::printProgramFailGlobalDirection(const BrandProgram& brandPr,
                                                        const PaxTypeFare& paxTypeFare)
{
  if (_diag889)
    _diag889->printFailGlobalDirection(brandPr, paxTypeFare);
}

void
BrandedFareDiagnostics::printProgramFailDirectionality(const BrandProgram& brandPr,
                                                       const PaxTypeFare& paxTypeFare,
                                                       Direction programDirection)
{
  if (_diag889)
  {
    const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(_trx);
    _diag889->printFailDirectionality(brandPr, paxTypeFare, useDirectionality,
        programDirection);
  }
}

void
BrandedFareDiagnostics::printT189NotFound(const BrandInfo* brand)
{
  if (_diag889)
    _diag889->printT189NotFound(brand);
}

void
BrandedFareDiagnostics::printBrandT189Item(const SvcFeesFareIdInfo* svcFeesFareIdInfo,
                                           const BrandInfo* brand)
{
  if (_diag889)
    _diag889->printBrandT189Item(svcFeesFareIdInfo, brand);
}

void
BrandedFareDiagnostics::printBrand(const BrandInfo* brand)
{
  if (_diag889)
    _diag889->printBrand(brand);
}

void
BrandedFareDiagnostics::printBlankLine()
{
  if (_diag889)
    _diag889->printBlankLine();
}

void
BrandedFareDiagnostics::printSize(const char* txt, int size)
{
  if (isDDInfo())
  {
    if (_diag889)
      _diag889->printSize(txt, size);
  }
}

void
BrandedFareDiagnostics::printDataNotFound()
{
  if (_diag889)
    _diag889->printDataNotFound();
}

void
BrandedFareDiagnostics::printDataNotFound(const FareMarket& fareMarket)
{
  if (_diag889)
    _diag889->printDataNotFound(fareMarket);
}

void
BrandedFareDiagnostics::printCompleteBrandProgram(const BrandProgram* brandPr)
{
  if (_diag889)
    _diag889->printCompleteBrandProgram(brandPr);
}

void
BrandedFareDiagnostics::printCompleteBrand(const BrandInfo* brand)
{
  if (_diag889)
    _diag889->printCompleteBrand(brand);
}

void
BrandedFareDiagnostics::printBrandProgramNotFound()
{
  if (_diag889)
    _diag889->printBrandProgramNotFound();
}

void
BrandedFareDiagnostics::printBrandProgramNotFound(const FareMarket& fareMarket)
{
  if (_diag889)
    _diag889->printBrandProgramNotFound(fareMarket);
}

void
BrandedFareDiagnostics::printBrandNotFound()
{
  if (_diag889)
    _diag889->printBrandNotFound();
}

void
BrandedFareDiagnostics::printFareMarketNotMatched(const FareMarket& fareMarket)
{
  if (_diag889)
    _diag889->printFareMarketNotMatched(fareMarket);
}

void
BrandedFareDiagnostics::printNoFaresFound(const FareMarket& fareMarket)
{
  if (_diag889)
    _diag889->printNoFaresFound(fareMarket);
}

void
BrandedFareDiagnostics::printBrandProgramSize(int size)
{
  printSeparator();
  printSize("NUMBER OF PROGRAMS: ", size);
}

void
BrandedFareDiagnostics::printProgram(const BrandProgram* brandPr, bool& needBrandProgramSeparator)
{
  if (!_diag889)
    return;

  if (needBrandProgramSeparator)
  {
    printBlankLine();
  }
  else
  {
    needBrandProgramSeparator = true;
  }

  if (isDDInfo())
  {
    if (!(_trx.diagnostic().diagParamMapItem(Diagnostic::PROGRAM_NAME).empty()))
    {
      printCompleteBrandProgram(brandPr);
    }
    else
    {
      printDetailBrandProgram(brandPr);
    }
  }
  else
  {
    printBrandProgram(brandPr);
  }
}

void
BrandedFareDiagnostics::printBrandSize(int size)
{
  printSize("NUMBER OF BRANDS: ", size);
}

void
BrandedFareDiagnostics::printDetailBrand(const BrandInfo* brand)
{
  if (_diag889 && isDDInfo())
  {
    if (!(_trx.diagnostic().diagParamMapItem(Diagnostic::BRAND_ID).empty()))
    {
      printCompleteBrand(brand);
    }
    else
    {
      printBrand(brand);
    }
  }
}

void
BrandedFareDiagnostics::printPT189Size(int size)
{
  printSize("NUMBER OF P T189: ", size);
}

void
BrandedFareDiagnostics::printPT189(const SvcFeesFareIdInfo* svcFeesFareIdInfo,
                                   const BrandInfo* brand,
                                   bool& needBrandSeparator,
                                   StatusT189 rc,
                                   bool& hasPrintedBrandT189Item)
{
  if (isDDInfo())
  {
    printDetailInfo(svcFeesFareIdInfo, rc);
  }
  else
  {
    if (!hasPrintedBrandT189Item)
    {
      if (needBrandSeparator)
      {
        printBlankLine();
      }
      else
      {
        needBrandSeparator = true;
      }
      printBrandT189Item(svcFeesFareIdInfo, brand);
      printTable189Header();
      hasPrintedBrandT189Item = true;
    }
    printSeqNoAndStatus(svcFeesFareIdInfo, rc);
  }
}

void
BrandedFareDiagnostics::printT189NotExist(const FareMarket& fareMarket)
{
  if (_diag889)
    _diag889->printT189NotExist(fareMarket);
}

void
BrandedFareDiagnostics::printT189Secondary(const SvcFeesFareIdInfo* svcFeesFareIdInfo, bool status)
{
  if (_diag889)
  {
    if (isDDInfo())
    {
      _diag889->printT189SecondaryDetailInfo(svcFeesFareIdInfo, status);
    }
    else
    {
      _diag889->printT189SecondaryItemNoAndSeqNo(svcFeesFareIdInfo, status);
    }
  }
}

void
BrandedFareDiagnostics::printFareNotValid()
{
  if (_diag889)
    _diag889->printFareNotValid();
}

void
BrandedFareDiagnostics::printCarrierNotMatched(const FareMarket& fareMarket)
{
  if (_diag889)
    _diag889->printCarrierNotMatched(fareMarket);
}

void
BrandedFareDiagnostics::printCurrentBrand(const BrandInfo* brand, int size)
{
  if (_diag889)
  {
    if(isDDInfo())
      _diag889->printBrandSizeAndCurrentBrand(brand, size);
    else
      _diag889->printCurrentBrand(brand);
  }
}

bool
BrandedFareDiagnostics::isDDInfo() const
{
  return _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
}

void
BrandedFareDiagnostics::displayT189Status(StatusT189 status, bool displayDetails)
{
  if (_diag889)
    _diag889->displayT189Status(status, displayDetails);
}

void
BrandedFareDiagnostics::printIsValidForBranding(const PaxTypeFare& paxTypeFare)
{
  if (_diag889)
    _diag889->printIsValidForBranding(paxTypeFare);
}

void
BrandedFareDiagnostics::printTable189Header() const
{
  if (_diag889)
    _diag889->printTable189Header();
}

void
BrandedFareDiagnostics::printNoProgramFound(const FareMarket& fareMarket)
{
  if (_diag889)
    _diag889->printNoProgramFound(fareMarket);
}

void
BrandedFareDiagnostics::printBrandFilter(const FareMarket& fareMarket, const BrandCode& brandCode, bool match) const
{
  if (_diag889)
    _diag889->printBrandFilter(fareMarket, brandCode, match);
}

bool
BrandedFareDiagnostics::isForSeqNumber(SequenceNumber sequenceNumber) const
{
  return _trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER).empty() ||
         sequenceNumber == boost::lexical_cast<int>(_trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER));
}

bool
BrandedFareDiagnostics::isForVendor(const VendorCode& vendorCode) const
{
  if (!_diag889)
    return true;
  return _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_VENDOR).empty() ||
    vendorCode == _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_VENDOR);
}

bool
BrandedFareDiagnostics::isForProgramName(const ProgramCode& programCode) const
{
  if (!_diag889)
    return true;
  return _trx.diagnostic().diagParamMapItem(Diagnostic::PROGRAM_NAME).empty() ||
    programCode.find(_trx.diagnostic().diagParamMapItem(Diagnostic::PROGRAM_NAME)) == std::string::npos;
}

bool
BrandedFareDiagnostics::isForBrandId(const BrandCode& brandCode) const
{
  if (!_diag889)
    return true;
  return _trx.diagnostic().diagParamMapItem(Diagnostic::BRAND_ID).empty() ||
         brandCode == _trx.diagnostic().diagParamMapItem(Diagnostic::BRAND_ID);
}

bool
BrandedFareDiagnostics::isForCarrier(const CarrierCode& carrierCode) const
{
  return _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER).empty() ||
         carrierCode == _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
}

bool
BrandedFareDiagnostics::isForFareClassCode(const FareClassCode& fareClassCode) const
{
  return _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE).empty() ||
        RuleUtil::matchFareClass(fareClassCode.c_str(),
	                         _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE).c_str());
}

void
BrandedFareDiagnostics::displayT189Validation(StatusT189 rc, SvcFeesFareIdInfo * svcFeesFareIdInfo,
                                              const BrandInfo* brand, bool& needBrandSeparator, bool& hasPrintedBrandT189Item,
                                              const std::vector<SecSvcFeesFareIdInfo*>& secondarySvcFeesFareIdInfoVec)
{
  printPT189(svcFeesFareIdInfo, brand, needBrandSeparator, rc, hasPrintedBrandT189Item);
  bool displayDetails = isDDInfo();
  if (rc == SOFTPASS_RBD || rc == FAIL_SEC_T189)
    displayT189Status(FAIL_PRIME_RBD, displayDetails);

  for (const SecSvcFeesFareIdInfo* secSvcFeesFareIdInfo : secondarySvcFeesFareIdInfoVec)
    printT189Secondary(secSvcFeesFareIdInfo->_svcFeesFareIdInfo, secSvcFeesFareIdInfo->_status);

  if (!(rc == SOFTPASS_RBD || rc == FAIL_SEC_T189))
    displayT189Status(rc, displayDetails);
}

void BrandedFareDiagnostics::printMatchBasedOnFareBasisCodeHeader()
{
  if (_diag889 && isDDInfo())
    _diag889->printMatchBasedOnFareBasisCodeHeader();
}

void BrandedFareDiagnostics::printMatchBasedOnBookingCodeHeaderPrimary()
{
  if (_diag889 && isDDInfo())
    _diag889->printMatchBasedOnBookingCodeHeaderPrimary();
}

void BrandedFareDiagnostics::printMatchBasedOnBookingCodeHeaderSecondary()
{
  if (_diag889 && isDDInfo())
    _diag889->printMatchBasedOnBookingCodeHeaderSecondary();
}

void BrandedFareDiagnostics::printMatchBasedOnExcludedFareBasisCodeHeader()
{
  if (_diag889 && isDDInfo())
    _diag889->printMatchBasedOnExcludedFareBasisCodeHeader();
}

void
BrandedFareDiagnostics::printMatchBasedOnFareBasisCode(const FareBasisCode& includedFareBasisCode,
                                                       bool isValid)
{
  if (_diag889 && isDDInfo())
      _diag889->printMatchBasedOnFareBasisCode(includedFareBasisCode, isValid);
}

void
BrandedFareDiagnostics::printMatchBasedOnBookingCode(const BookingCode& bookingCode, bool isValid)
{
  if (_diag889 && isDDInfo())
      _diag889->printMatchBasedOnBookingCode(bookingCode, isValid);
}

void
BrandedFareDiagnostics::printValidateCbasResult(PaxTypeFare::BrandStatus status)
{
  if (_diag889)
      _diag889->printValidateCbasResult(status);
}

void
BrandedFareDiagnostics::printYyFareIgnored()
{
  if (_diag889)
    _diag889->printYyFareIgnored();
}

} // namespace
