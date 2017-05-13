//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelector.cpp
//  Created:     2013
//  Authors:
//
//  Description:
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
//-------------------------------------------------------------------

#include "BrandedFares/CbasBrandedFaresSelector.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/S8BrandedFaresSelector.h"
#include "Common/FallbackUtil.h"
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

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/format.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>

namespace tse
{

PaxTypeFare::BrandStatus
CbasBrandedFareValidator::validateFare(const FareBasisCodeStructure& ptfFareBasisCode, const std::vector<BookingCode>& bookingCodeVec,
                                       const BrandInfo& brand, BrandedFareDiagnostics& diagnostics,
                                       bool skipHardPassValidation) const
{
  std::vector<BookingCode> primaryBookingCodes;
  std::vector<BookingCode> secondaryBookingCodes;
  std::vector<FareBasisCode> fareBasisCodes;

  PaxTypeFare::BrandStatus status;

  handleFareBasisCodeWorkaround(brand, primaryBookingCodes, secondaryBookingCodes, fareBasisCodes);
  if(!primaryBookingCodes.empty() || !secondaryBookingCodes.empty())
  {
    status = matchBasedOnBookingCode(bookingCodeVec, primaryBookingCodes, secondaryBookingCodes, diagnostics, skipHardPassValidation);
  }
  else
  {
    status = matchBasedOnFareBasisCode(ptfFareBasisCode, fareBasisCodes, diagnostics);
  }
  if(status != PaxTypeFare::BS_FAIL)
  {
    if (!brand.excludedFareBasisCode().empty())
    {
      diagnostics.printMatchBasedOnExcludedFareBasisCodeHeader();
      PaxTypeFare::BrandStatus excludeStatus = matchBasedOnFareBasisCode(ptfFareBasisCode, brand.excludedFareBasisCode(), diagnostics);
      if(excludeStatus == PaxTypeFare::BS_HARD_PASS)
      {
        status = PaxTypeFare::BS_FAIL;
      }
    }
  }
  diagnostics.printValidateCbasResult(status);
  return status;
}

PaxTypeFare::BrandStatus
CbasBrandedFareValidator::validateFare(const PaxTypeFare* paxTypeFare, const BrandProgram* brandPr, const BrandInfo* brand,
                                       bool& needBrandSeparator, BrandedFareDiagnostics& diagnostics,
                                       bool skipHardPassValidation) const
{
  FareBasisCodeStructure ptfFareBasisCode = paxTypeFare->createFareBasis(_trx);
  std::vector<BookingCode> primeBookingCodeVec;
  paxTypeFare->getPrimeBookingCode(primeBookingCodeVec);
  return validateFare(ptfFareBasisCode, primeBookingCodeVec, *brand, diagnostics, skipHardPassValidation);
}

PaxTypeFare::BrandStatus
CbasBrandedFareValidator::matchBasedOnBookingCode(const std::vector<BookingCode>& fareBookingCodes,
                                                  const std::vector<BookingCode>& primaryBookingCodes,
                                                  const std::vector<BookingCode>& secondaryBookingCodes,
                                                  BrandedFareDiagnostics& diagnostics,
                                                  bool skipHardPassValidation) const
{
  if (!skipHardPassValidation)
  {
    if (primaryBookingCodes.empty() && secondaryBookingCodes.empty())
      return PaxTypeFare::BS_HARD_PASS;

    if (!primaryBookingCodes.empty())
    {
      diagnostics.printMatchBasedOnBookingCodeHeaderPrimary();

      if(isBookingCodeMatched(fareBookingCodes, primaryBookingCodes, diagnostics))
        return PaxTypeFare::BS_HARD_PASS;
    }

    if (_trx.isSoftPassDisabled())
    {
      return PaxTypeFare::BS_FAIL;
    }
  }

  if (!secondaryBookingCodes.empty())
  {
    diagnostics.printMatchBasedOnBookingCodeHeaderSecondary();

    if(isBookingCodeMatched(fareBookingCodes, secondaryBookingCodes, diagnostics))
      return PaxTypeFare::BS_SOFT_PASS;
  }

  return PaxTypeFare::BS_FAIL;
}

bool
CbasBrandedFareValidator::isBookingCodeMatched(const std::vector<BookingCode>& primeBookingCodeVec,
                                               const std::vector<BookingCode>& bookingCodes,
                                               BrandedFareDiagnostics& diagnostics) const
{
  static const BookingCode WILDCARD = "*";
  for (const auto& bookingCode : bookingCodes)
  {
    if (bookingCode.empty() || bookingCode == BLANK_CODE || bookingCode == WILDCARD ||
        std::find_if(primeBookingCodeVec.begin(),
                     primeBookingCodeVec.end(),
                     BookingCodeComparator(bookingCode)) != primeBookingCodeVec.end())
    {
      diagnostics.printMatchBasedOnBookingCode(bookingCode, true);
      return true;
    }
    diagnostics.printMatchBasedOnBookingCode(bookingCode, false);
  }
  return false;
}

PaxTypeFare::BrandStatus
CbasBrandedFareValidator::matchBasedOnFareBasisCode(const FareBasisCodeStructure& ptfFareBasisCode,
                                                    const std::vector<FareBasisCode>& fareBasisCodes,
                                                    BrandedFareDiagnostics& diagnostics) const
{
  if (fareBasisCodes.empty())
    return PaxTypeFare::BS_HARD_PASS;

  diagnostics.printMatchBasedOnFareBasisCodeHeader();
  if(boost::algorithm::any_of(fareBasisCodes.begin(), fareBasisCodes.end(),
                              boost::bind(&CbasBrandedFareValidator::isMatchedFareBasisCode, this, ptfFareBasisCode,  _1, diagnostics)))
    return PaxTypeFare::BS_HARD_PASS;
  return PaxTypeFare::BS_FAIL;
}

bool
CbasBrandedFareValidator::isMatchedFareBasisCode(const FareBasisCodeStructure& ptfFareBasisCode,
                                                 const FareBasisCode& matchingFareBasisCode,
                                                 BrandedFareDiagnostics& diagnostics) const
{
  if(ptfFareBasisCode.getFareBasisCode() == matchingFareBasisCode)
  {
    diagnostics.printMatchBasedOnFareBasisCode(matchingFareBasisCode, true);
    return true;
  }

  FareBasisCodeStructure matchingFareBasis(matchingFareBasisCode);

  if(!matchingFareBasis.getTicketDesignator().empty())
  {
    if(matchingFareBasis.getTicketDesignator() != ptfFareBasisCode.getTicketDesignator())
    {
      diagnostics.printMatchBasedOnFareBasisCode(matchingFareBasisCode, false);
      return false;
    }
    else
    {
      if(matchingFareBasis.getFareBasis().empty())
      {
        diagnostics.printMatchBasedOnFareBasisCode(matchingFareBasisCode, true);
        return true;
      }
    }
  }
  else
    if(ptfFareBasisCode.getFareBasis() == matchingFareBasis.getFareBasis())
    {
      diagnostics.printMatchBasedOnFareBasisCode(matchingFareBasisCode, true);
      return true;
    }
  if(isFamilyFareBasisCode(matchingFareBasis.getFareBasis()))
  {
    bool isMatching = isMatchingNonExactFareBasis(ptfFareBasisCode.getFareBasis(), matchingFareBasis.getFareBasis(), diagnostics);
    diagnostics.printMatchBasedOnFareBasisCode(matchingFareBasisCode, isMatching);
    return isMatching;
  }
  diagnostics.printMatchBasedOnFareBasisCode(matchingFareBasisCode, false);
  return false;
}

bool
CbasBrandedFareValidator::isFamilyFareBasisCode(const std::string& matchingFareBasis) const
{
  return matchingFareBasis.find('-') != std::string::npos;
}

bool
CbasBrandedFareValidator::isMatchingNonExactFareBasis(const std::string& ptfFareBasis, const std::string& matchingFareBasis,
                                                      BrandedFareDiagnostics& diagnostics) const
{
  return RuleUtil::matchFareClass(matchingFareBasis.c_str(), ptfFareBasis.c_str());
}

void
CbasBrandedFareValidator::handleFareBasisCodeWorkaround(const BrandInfo& brand, std::vector<BookingCode>& primaryBookingCodes,
                                                        std::vector<BookingCode>& secondaryBookingCodes,
                                                        std::vector<FareBasisCode>& fareBasisCodes) const
{
  std::copy(brand.primaryBookingCode().begin(), brand.primaryBookingCode().end(),
            std::back_inserter(primaryBookingCodes));
  std::copy(brand.secondaryBookingCode().begin(), brand.secondaryBookingCode().end(),
            std::back_inserter(secondaryBookingCodes));
  std::copy(brand.includedFareBasisCode().begin(), brand.includedFareBasisCode().end(),
            std::back_inserter(fareBasisCodes));

  bool hasprimary = !primaryBookingCodes.empty(),
       hassecondary = !secondaryBookingCodes.empty(),
       hasfarebasis = !fareBasisCodes.empty();

  if (!hasprimary && hassecondary && hasfarebasis)
  {
    // Special behavior for "JetBlue A321" project: build primary booking codes
    // out of first letters of fare basis codes. This is a workaround because MM
    // doesn't allow to reuse primary booking codes between different brands. To
    // be deleted after MM is fixed and clients adjust.
    // Check INTELISELL logic in IC/Adapters/Brands/BrandsSAXHandler.C for reference
    primaryBookingCodes.reserve(fareBasisCodes.size());
    for (const FareBasisCode& fbc : brand.includedFareBasisCode())
    {
      std::string const &code = fbc;
      if (code.size()==0 || code[0]<'A' || code[0]>'Z')
      {
        primaryBookingCodes.clear();
        secondaryBookingCodes.clear();
        break;
      }
      primaryBookingCodes.push_back(code.substr(0, 1));
    }
    if (!primaryBookingCodes.empty())
      fareBasisCodes.clear();
  }
}

} // namespace
